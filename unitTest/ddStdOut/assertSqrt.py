#!/usr/bin/python3

import os
import sys
from pathlib import Path

cmpExpected=["__verrou__stdout__init__",
             "xn[1]=*  delta=*",
             "xn[2]=*  delta=*",
             "xn[3]=*  delta=*",
             "xn[4]=*  delta=*"
             "xn[5]=*  delta=*"
]


cmpUnExpected=[
    "xn[6]=*  delta=*",
    "xn[7]=*  delta=*"
]

def filterInit(line):
    if line.startswith("expect open filename:"):
        return "__verrou__stdout__init__"
    else:
        return line

def checkDDcmp(rep):
    ddminCmp=rep / "rddmin-cmp" / "dd.IOMatch.include"
    if ddminCmp.is_file():
        lines=[filterInit(line) for line in open(ddminCmp).readlines()]
        for expectedLine in cmpExpected:
            if expectedLine not in cmpExpected:
                return False

        for line in lines:
            if line in cmpUnExpected:
                return False
        return True
    return False


if __name__=="__main__":
    rep=Path(sys.argv[1])

    ddCmp=checkDDcmp(rep)
    if ddCmp:
        print("OK:", rep)
        sys.exit(0)
    else:
        print("error with ddCmp : ", rep)
        sys.exit(42)
