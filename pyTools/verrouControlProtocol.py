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

"""Protocol used to control a running verrou process (verrou_control).

The protocol is independent of the transport which carries it (see
verrouControlTransport: vgdb monitor commands with the Valgrind engine). A
request is a list of commands, executed in order by verrou; the reply is the
text printed by verrou, as a list of lines.

Commands (handled by vr_handle_monitor_command in vr_clreq.c):

  help
      list of the commands.
  instrumentation [on|start|off|stop] [hard|soft]
      switches the instrumentation (hard by default); without argument, only
      prints the status. Reply: the instrumentation status (see below).
  status instrumentation
      reply:
          instrumentation status:
          <TAB>soft: ON|OFF
          <TAB>hard: ON|OFF
  status back
      backtrace of each thread. Reply, for each thread:
          Thread tid: <N>
          <TAB><frame>          (one line by frame, innermost frame first)
          <empty line>
  display_counter
      prints the operation counters (in the verrou log, not in the reply).
  reset_counter
      resets the operation counters.
  rounding <MODE>
      switches the rounding mode. MODE is the display name of the mode, in
      capitals (NEAREST, UPWARD, RANDOM_DET...), not the --rounding-mode= name.
  verbose on|off
      switches the verbose messages.

A message of verrou can be interleaved with a reply (for instance
"Monitor: instrumentation switched OFF" before the instrumentation status).
"""

HARD = "hard"
SOFT = "soft"


def instrumentation(start, kind=HARD):
    """Command switching the instrumentation on (start=True) or off"""
    if kind not in (HARD, SOFT):
        raise ValueError("instrumentation kind: hard or soft")
    return "instrumentation %s %s" % ("start" if start else "stop", kind)


def status_instrumentation():
    return "status instrumentation"


def status_back():
    return "status back"


def display_counter():
    return "display_counter"


def reset_counter():
    return "reset_counter"


def rounding(mode):
    """Command switching the rounding mode; mode: --rounding-mode= name or display name"""
    return "rounding " + mode.upper()


def verbose(on):
    return "verbose " + ("on" if on else "off")


def parse_backtrace(lines):
    """Reply of "status back" -> list of (tid, [frames]), innermost frame first"""
    res = []
    frames = []
    tid = None
    for line in lines:
        if line.startswith("Thread tid:"):
            tid = int(line.split(":")[1])
            continue
        if line == "":
            res += [(tid, frames)]
            tid = None
            frames = []
            continue
        if line.startswith("\t"):
            frames += [line[1:]]
    return res


def parse_instrumentation_status(lines):
    """Reply of "status instrumentation" (or "instrumentation ...") -> {"soft": bool, "hard": bool}"""
    res = {}
    for line in lines:
        line = line.strip()
        for kind in (SOFT, HARD):
            if line.startswith(kind + ":"):
                res[kind] = line.split(":", 1)[1].strip() == "ON"
    if set(res) != {SOFT, HARD}:
        raise ValueError("invalid instrumentation status reply: %s" % lines)
    return res
