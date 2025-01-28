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

fusionParam=1
def fusionFilter(strLine):
    if strLine.startswith("it:"):
        it=int((strLine.split("\t")[0]).split(":")[1])
        if it%fusionParam==0:
            return strLine
        else:
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
    fusion=False
    if len(sys.argv)>1:
        for arg in sys.argv:
            if arg.startswith("--loop-fusion="):
                fusion=True
                fusionParam=int(arg.partition("=")[2])
    while True:
        line=getLineCharByChar()
        if line==None:
            sys.exit(0)
        if fusion:
            print(applyFilter(line, [deleteDebug, fusionFilter]))
        else:
            print(applyFilter(line, [deleteDebug]))

