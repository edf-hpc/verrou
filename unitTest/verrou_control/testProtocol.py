#!/usr/bin/env python3
"""Test of the verrou control protocol (verrouControlProtocol), without a
running process: commands built and replies parsed."""
import sys
import verrouControlProtocol as protocol

fail = 0


def check(name, value, expected):
    global fail
    if value == expected:
        print("OK    %s" % name)
    else:
        print("KO    %s : %r instead of %r" % (name, value, expected))
        fail = 1


check("instrumentation start hard", protocol.instrumentation(True), "instrumentation start hard")
check("instrumentation stop soft", protocol.instrumentation(False, protocol.SOFT), "instrumentation stop soft")
check("status instrumentation", protocol.status_instrumentation(), "status instrumentation")
check("status back", protocol.status_back(), "status back")
check("display_counter", protocol.display_counter(), "display_counter")
check("reset_counter", protocol.reset_counter(), "reset_counter")
check("rounding (display name)", protocol.rounding("random_det"), "rounding RANDOM_DET")
check("verbose", protocol.verbose(False), "verbose off")

reply = ["Thread tid: 1", "\t0x4001154: infLoop(double) (infiniteLoop.cxx:7)",
         "\t0x40011D9: main (infiniteLoop.cxx:21)", "", "Thread tid: 2", "\t0x40012: f (a.c:3)", ""]
check("parse_backtrace", protocol.parse_backtrace(reply),
      [(1, ["0x4001154: infLoop(double) (infiniteLoop.cxx:7)", "0x40011D9: main (infiniteLoop.cxx:21)"]),
       (2, ["0x40012: f (a.c:3)"])])

reply = ["--1234-- Monitor: instrumentation switched OFF", "instrumentation status:", "\tsoft: ON", "\thard: OFF"]
check("parse_instrumentation_status", protocol.parse_instrumentation_status(reply), {"soft": True, "hard": False})

try:
    protocol.parse_instrumentation_status(["garbage"])
    check("invalid status reply rejected", False, True)
except ValueError:
    check("invalid status reply rejected", True, True)

sys.exit(fail)
