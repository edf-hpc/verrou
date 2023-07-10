#!/usr/bin/python3
import sys

def applyFilter(strLine,filterList):
    res=strLine
    for filterFnc in filterList:
        res=filterFnc(res)
    return res

def deleteDebug(strLine):
    if "debug" in strLine:
        return ""
    else:
        return strLine

if __name__=="__main__":
    line=sys.stdin.readline()
    print(applyFilter(line, [deleteDebug]), end="")
