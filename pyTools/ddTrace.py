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

"""Execution trace of the delta-debug algorithms (DD_stoch).

The trace is a JSON-lines file (one event per line) :
  header      : {"ev":"header", "version", "nbProc", "wallStart", "argv"}
  task_begin  : {"ev":"task_begin", "t", "slot", "kind", "conf", "sample"}
  task_end    : {"ev":"task_end",   "t", "slot", "kind", "conf", "sample", "status"}
  phase_begin : {"ev":"phase_begin","t", "name", "info"}
  phase_end   : {"ev":"phase_end",  "t", "name"}
  runtime_begin : {"ev":"runtime_begin","t", "name", "info"}
  runtime_end   : {"ev":"runtime_end",  "t", "name"}
  event       : {"ev":"event",      "t", "name", "info"}

The event oneMinimal (info : kind, conf, subConfs) gives for each ddmin configuration the
configurations (one delta removed) which prove its 1-minimality.

t is in seconds from the tracer creation. slot is an index in [0, nbProc[ : each
task uses the smallest free slot, so slots are stable between successive thread pools.
The phases describe the algorithms, the runtime sections describe how the sub-processes
are scheduled (testTab : order and earlyExit status).
The file is converted into a Paje or OTF2 trace by verrou_dd_trace_convert.
"""

import json
import threading
import time
import functools
import contextlib
import sys


class noTracer:
    """Default tracer : does nothing"""
    enabled=False
    def phaseBegin(self, name, **info):
        pass
    def phaseEnd(self, name):
        pass
    def event(self, name, **info):
        pass
    @contextlib.contextmanager
    def phase(self, name, **info):
        yield
    @contextlib.contextmanager
    def runtime(self, name, **info):
        yield
    @contextlib.contextmanager
    def worker(self):
        yield
    @contextlib.contextmanager
    def task(self, kind, conf, sample=None):
        yield {}
    def close(self):
        pass


class jsonTracer:
    enabled=True
    def __init__(self, fileName, nbProc):
        self.handler=open(str(fileName),"w", buffering=1) #line buffering : the trace survives an interruption
        self.lock=threading.Lock()
        self.t0=time.perf_counter()
        self.freeSlots=set(range(max(1,nbProc)))
        self.nextSlot=max(1,nbProc) #used only if more workers than expected
        self.local=threading.local()
        self._write({"ev":"header", "version":1, "nbProc":max(1,nbProc), "wallStart":time.time(), "argv":sys.argv})

    def _time(self):
        return time.perf_counter()-self.t0

    def _write(self, data):
        line=json.dumps(data)
        with self.lock:
            self.handler.write(line+"\n")

    def phaseBegin(self, name, **info):
        self._write({"ev":"phase_begin", "t":self._time(), "name":name, "info":info})

    def phaseEnd(self, name):
        self._write({"ev":"phase_end", "t":self._time(), "name":name})

    @contextlib.contextmanager
    def phase(self, name, **info):
        self.phaseBegin(name, **info)
        try:
            yield
        finally:
            self.phaseEnd(name)

    @contextlib.contextmanager
    def runtime(self, name, **info):
        self._write({"ev":"runtime_begin", "t":self._time(), "name":name, "info":info})
        try:
            yield
        finally:
            self._write({"ev":"runtime_end", "t":self._time(), "name":name})

    def event(self, name, **info):
        self._write({"ev":"event", "t":self._time(), "name":name, "info":info})

    def _acquireSlot(self):
        with self.lock:
            if len(self.freeSlots)==0:
                self.freeSlots.add(self.nextSlot)
                self.nextSlot+=1
            slot=min(self.freeSlots)
            self.freeSlots.remove(slot)
        return slot

    def _releaseSlot(self, slot):
        with self.lock:
            self.freeSlots.add(slot)

    @contextlib.contextmanager
    def worker(self):
        """Reserve a slot for the current thread (several tasks executed in sequence)"""
        if getattr(self.local, "slot", None)!=None:
            yield
            return
        self.local.slot=self._acquireSlot()
        try:
            yield
        finally:
            self._releaseSlot(self.local.slot)
            self.local.slot=None

    @contextlib.contextmanager
    def task(self, kind, conf, sample=None):
        """Trace a sub-process execution. The caller can set the "status" key of the yielded dict"""
        with self.worker():
            slot=self.local.slot
            self._write({"ev":"task_begin", "t":self._time(), "slot":slot, "kind":kind, "conf":conf, "sample":sample})
            res={"status":None}
            try:
                yield res
            finally:
                self._write({"ev":"task_end", "t":self._time(), "slot":slot, "kind":kind, "conf":conf, "sample":sample, "status":res["status"]})

    def close(self):
        self.handler.close()


tracer=noTracer()

def init(fileName, nbProc):
    global tracer
    tracer.close()
    if fileName==None:
        tracer=noTracer()
    else:
        tracer=jsonTracer(fileName, 1 if nbProc==None else nbProc)
    return tracer


def tracePhase(name):
    """Decorator : trace the method as an algorithm phase (nbDelta is the length of the first argument)"""
    def decorator(func):
        @functools.wraps(func)
        def wrapper(self, *args, **kwargs):
            info={}
            if len(args)>0 and isinstance(args[0], list):
                info["nbDelta"]=len(args[0])
            if len(args)>1 and isinstance(args[1], int):
                info["nbRun"]=args[1]
            with tracer.phase(name, **info):
                return func(self, *args, **kwargs)
        return wrapper
    return decorator
