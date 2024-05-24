#!/usr/bin/python3

import os
import sys

def checkDDmin(rep, num):
    ddmin0=os.path.join(rep, "ddmin"+str(num), "dd.task.include")
    if os.path.exists(ddmin0):
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
    ddminCmp=os.path.join(rep, "rddmin-cmp", "dd.task.include")
    if os.path.exists(ddminCmp):
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
    rep=sys.argv[1]

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
