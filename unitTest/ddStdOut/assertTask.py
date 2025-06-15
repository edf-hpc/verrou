#!/usr/bin/python3
import os
import sys
from pathlib import Path

def checkDDmin(rep, num):
    ddmin0=rep / ("ddmin"+str(num)) / "dd.task.include"
    if ddmin0.is_file():
        lines=open(ddmin0).readlines()
        if len(lines)!=1:
            return False
        line=lines[0]
        if "muller_iter\t"+str(num) in line:
            return True
        else:
            return False
    else:
        return False


def checkDDcmp(rep):
    ddminCmp=rep / "rddmin-cmp" / "dd.task.include"
    if ddminCmp.is_file():
        lines=open(ddminCmp).readlines()
        for i in range(1,11):
            res_i=False
            for line in lines:
                if "muller_iter\t"+str(i) in line:
                    res_i=True
            if res_i==False:
                print("checkDDCmp:", lines)
                return False
        return True
    else:
        return False


if __name__=="__main__":
    rep=Path(sys.argv[1])

    ddmin0=checkDDmin(rep, 0)
    if not ddmin0:
        print("error with ddmin0 : ", rep)

    ddmin1=checkDDmin(rep, 1)
    if not ddmin1:
        print("error with ddmin1 : ", rep)

    ddCmp=checkDDcmp(rep)
    if not ddCmp:
        print("error with ddCmp : ", rep)


    if ddmin0 and ddmin1 and ddCmp:
        print("OK:", rep)
        sys.exit(0)
    else:
        print("KO:", rep)
        sys.exit(42)
