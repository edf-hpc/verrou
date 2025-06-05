#!/usr/bin/python3
import sys
from pathlib import Path


def assertFile(rep, emptyCmp):
    pathMinTab= list(Path(rep).glob("ddmin*/dd.back.include") )
    pathFullTab=list(Path(rep).glob("FullPerturbation/dd.back.include"))
    pathFull=Path(rep) / "FullPerturbation" / "dd.back.include"
    pathNo= Path(rep) / "NoPerturbation"/"dd.back.include"
    pathCmp= Path(rep) / "rddmin-cmp"/"dd.back.include"

    nbDDMin=len(pathMinTab)
    nbDDMin_Line=sum([len(open(x).readlines())  for x in pathMinTab ])
    nbFull_Line=len(open(pathFull).readlines())
    nbNo_Line=len(open(pathNo).readlines())
    nbCmp_Line=len(open(pathCmp).readlines())

    expectedFull=6
    if emptyCmp:
        expectedFull=1

    print("nbDDMin" , nbDDMin)
    print("nbDDMin_Line", nbDDMin_Line)
    print("nbFull_Line", nbFull_Line)
    print("nbNo_Line", nbNo_Line)
    print("nbCmp_Line", nbCmp_Line)

    if nbDDMin !=1:
        print("Wrong number of ddmin: ", nbDDMin)
        return False

    if nbDDMin_Line!=1:
        print("Wrong number of nbDDMin_Line", nbDDMin_Line)
        return False


    if nbFull_Line!=expectedFull :
        print("Wrong number of nbFull_Line", nbFull_Line)
        return False

    if nbCmp_Line!=expectedFull-nbDDMin_Line:
        print(" Wrong number of nbCmp_Line", nbCmp_Line)
        return False

    return True


if __name__=="__main__":
    rep=sys.argv[1]

    withLine=""
    if len(sys.argv)>2:
        withLine=sys.argv[2]

    check=assertFile(rep, withLine)
    if not check:
        print("Fail")
        sys.exit(42)
    print("OK")
    sys.exit(0)
