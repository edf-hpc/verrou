#!/usr/bin/env python3

import sys
import re
from pathlib import Path

dropPattern=["std::","ios_base.h", "cmath", "stl"]
stdBBPattern=["iomanip\(\d+\)", "std_abs.h\(\d+\)", "stl_algobase.h\([0-9,]+\)"]
regExpBBTab=[ re.compile(stdBB) for stdBB in stdBBPattern]
regExpAddr=re.compile("\+\d+\t")

def filterBack(content):
    drop=True
    res=[]
    for line in content:
        if line.startswith("|0\tmain\tunitTest.cxx:58"):
            drop=False
        if drop: continue

        spline=line.split("\t")
        if any([spline[1].startswith(pattern) for pattern in dropPattern]):
            continue

        if line.startswith("|"):
            if "stl_algobase.h" in line:
                continue
            line=regExpAddr.sub("+ADDR\t",line)
        else:
            for regExpBB in regExpBBTab:
                line=regExpBB.sub("STDBB(X)", line)

        res+=[line]
    return res

def parse(filename):
    lines=open(filename,"r").readlines()
    header=lines[0:5]
    content=lines[5:]
    return header+filterBack(content)

def loadRef(filename):
    lines=open(filename,"r").readlines()
    return lines

def export(filenameNew, lines):
    handler=open(filenameNew, "w")
    for line in lines:
        handler.write(line)

def generateRef(repOrigin, repDest):
    dataOrgTab=list(repOrigin.glob("backCoverData_seq*_cover*.csv"))
    for dataOrg in dataOrgTab:
        dataFiltered=parse(dataOrg)
        export(repDest / (dataOrg.name + ".filtered.prop"), dataFiltered)


def cmpRepToRef(rep, repRef):
    dataRefTab=list(repRef.glob("backCoverData_seq*_cover*.csv.filtered"))
    dataTab=list(rep.glob("backCoverData_seq*_cover*.csv"))

    if len(dataRefTab)!=len(dataTab):
        print("invalid size")
        print("dataRef: ", [str(x) for x in dataRefTab])
        print("data   : ", [str(x) for x in dataTab])
        sys.exit(42)

    for data in dataTab:
        dataFiltered=parse(data)
        name=data.name
        dataRef=loadRef(repRef/(name+".filtered"))

        if dataFiltered!=dataRef:
            print("dataRef:")
            for line in dataRef:
                print("ref\t"+line, end="")

            print("data:")
            for line in dataRef:
                print("filtered data\t"+line, end="")

            sizeNew=len(dataFiltered)
            sizeRef=len(dataRef)
            print("size: ", sizeNew , sizeRef)
            for index in range(min(sizeRef, sizeNew)):
                lineRef=dataRef[index]
                lineFiltered=dataFiltered[index]
                if lineRef != lineFiltered:
                    print("First failing line")
                    print("lineRef:      ", [lineRef])
                    print("lineFiltered: ", [lineFiltered])
                    break
            sys.exit(42)
        print(data.name +": OK")
    print(rep , "==", repRef)
    sys.exit(0)

if __name__=="__main__":

    if sys.argv[1]=="generateRef":
        repOrg=sys.argv[2]
        repDist=sys.argv[3]
        generateRef(Path(repOrg), Path(repDist))
    elif sys.argv[1]=="cmpRepToRef":
        rep=sys.argv[2]
        repRef=sys.argv[3]
        cmpRepToRef(Path(rep), Path(repRef))
    else:
        print(sys.argv[1] , " should be generateRef cmpRepToRef")
        sys.exit(42)
