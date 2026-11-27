# This file is part of Verrou, a FPU instrumentation tool.

# Copyright (C) 2014-2026 EDF
#   B. Lathuilière <bruno.lathuiliere@edf.fr>

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.

# The GNU Lesser General Public License is contained in the file COPYING.

"""Transports carrying the verrou control protocol (verrouControlProtocol) to
a running verrou process.

A transport finds the running verrou processes, sends them requests (lists of
protocol commands) and returns the replies, and terminates them. The protocol
does not depend on the transport: another instrumentation engine only needs
another Transport.

VgdbTransport : Valgrind engine, through vgdb (gdbserver monitor commands).
"""

import re
import subprocess
from pathlib import Path


class Transport:
    """Interface of a transport"""

    def list_processes(self):
        """Running verrou processes : {pid: {"cmdline": program and its
        arguments, "raw": complete command line}}"""
        raise NotImplementedError

    def send(self, pid, commands):
        """Sends a request (list of protocol commands, executed in order) to
        the process pid ; returns the reply as a list of lines"""
        raise NotImplementedError

    def terminate(self, pid):
        """Terminates the process pid"""
        raise NotImplementedError


class VgdbTransport(Transport):
    """Valgrind engine : monitor commands sent with vgdb"""

    def __init__(self, vgdb_prefix=""):
        # one argument, so that a prefix containing spaces is kept intact
        self.vgdb_prefix_option = ["--vgdb-prefix=" + vgdb_prefix] if vgdb_prefix else []
        self.vgdb = self._find_vgdb()

    @staticmethod
    def _find_vgdb():
        here = Path(__file__).resolve().parent
        for p in (here / "vgdb", here / "../coregrind/vgdb"):
            p = p.resolve()
            if p.exists():
                return str(p)
        return "vgdb"

    def _run(self, args):
        p = subprocess.run([self.vgdb] + self.vgdb_prefix_option + args,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                           universal_newlines=True)
        return p.stdout.splitlines()

    def list_processes(self):
        pids = {}
        for line in self._run(["-l"]):
            m = re.match(r"^use --pid=(\d+) for \S*?valgrind\s+(.*?)\s*$", line)
            if not m:
                continue
            pid = int(m.group(1))
            cmdline = m.group(2)
            if "--tool=verrou" not in cmdline:
                continue
            cleaned = cmdline
            while re.match(r"^-+\S+\s+", cleaned):
                cleaned = re.sub(r"^-+\S+\s+", "", cleaned, count=1)
            pids[pid] = {"cmdline": cleaned, "raw": cmdline}
        return pids

    def send(self, pid, commands):
        # vgdb concatenates its remaining arguments into one monitor command
        # (-c separates the commands), so splitting the commands on spaces is
        # equivalent to the former shell call
        args = ["--pid=" + str(pid)]
        for k, cmd in enumerate(commands):
            if k != 0:
                args.append("-c")
            args += cmd.split()
        return self._run(args)

    def terminate(self, pid):
        # gdbserver command of Valgrind, not a command of the verrou protocol
        return self._run(["--pid=" + str(pid), "v.kill"])


def default_transport(vgdb_prefix=""):
    """Transport of the instrumentation engine in use"""
    return VgdbTransport(vgdb_prefix)
