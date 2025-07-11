#!/usr/bin/python3

import os
import sys
from pathlib import Path

def checkDDmin0(rep, withEmptyLine=False):
    ddmin0=rep / "ddmin0" / "dd.IOMatch.include"
    if ddmin0.is_file():
        lines=open(ddmin0).readlines()
        if len(lines)!=1:
            return False
        line=lines[0]
        if "begin iter" in line or "__verrou__stdout__init__" in line:
            return True
        else:
            return False
    else:
        return False

def checkDDmin1(rep, withEmptyLine=False):
    ddmin1=rep / "ddmin1" / "dd.IOMatch.include"
    if ddmin1.is_file():
        lines=open(ddmin1).readlines()
        if len(lines)!=1:
            print("ddmin1 : nbLineError : %s"%str(lines))
            return False
        line=lines[0]
        if not withEmptyLine:
            if (line.startswith("it: 0")):
                return True
            else:
                print("ddmin1: line error :|%s|"%(line))
                return False
        else:
            if(line =="\n"):
                return True
            else:
                print("ddmin1: line error :|%s|"%(line))
                return False
    else:
        return False


def checkDDcmp(rep, withEmptyLine=False):
    ddminCmp=rep / "rddmin-cmp" / "dd.IOMatch.include"
    if ddminCmp.is_file():
        lines=open(ddminCmp).readlines()
        if withEmptyLine:
            if len(lines)==0:
                return True
            else:
                return False
        else:
            for i in range(1,11):
                res_i=False
                for line in lines:
                    if "it: "+str(i) in line:
                        res_i=True
                if res_i==False:
                    print("checkDDCmp:", lines)
                    return False

            return True
    else:
        return False


if __name__=="__main__":
    rep=Path(sys.argv[1])
    emptyLine=False

    if len(sys.argv)>2 and sys.argv[2]=="emptyLine":
        emptyLine=True

    ddmin0=checkDDmin0(rep, emptyLine)
    if not ddmin0:
        print("error with ddmin0 : ", rep)

    ddmin1=checkDDmin1(rep, emptyLine)
    if not ddmin1:
        print("error with ddmin1 : ", rep)

    ddCmp=checkDDcmp(rep, emptyLine)
    if not ddCmp:
        print("error with ddCmp : ", rep)


    if ddmin0 and ddmin1 and ddCmp:
        print("OK:", rep)
        sys.exit(0)
    else:
        print("KO:", rep)
        sys.exit(42)
