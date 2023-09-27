#!/usr/bin/python3 -u
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


def getLineCharByChar():
    line=""
    while True:
        c=sys.stdin.read(1)
        if line== "\x00": #EOF detected
            return None
        if c=='\n':
            return line
        line+=c

if __name__=="__main__":
    while True:
        line=getLineCharByChar()
        if line==None:
            sys.exit(0)
        print(applyFilter(line, [deleteDebug]))
