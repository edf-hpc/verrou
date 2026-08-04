#!/usr/bin/env python3

import sys
import re
from pathlib import Path
import platform

dropPattern=["std::","ios_base.h", "cmath", "stl"]
stdBBPattern=[r"iomanip\(\d+\)", r"std_abs.h\(\d+\)", r"stl_algobase.h\([0-9,]+\)"]
regExpBBTab=[ re.compile(stdBB) for stdBB in stdBBPattern]
regExpAddr=re.compile(r"\+\d+\t")


dropInitLine="|0\tmain\tunitTest.cxx:58"
dropInitLineMatch="2\tunitTest.cxx(26) F"

renameDic={
    "integrate.hxx(16-17,26) F?":"integrate.hxx(15-17,26) F?" ,
}

compressDic={
    ("integrate.hxx(11,13,15) F", None,"integrate.hxx(16) F?") : "integrate.hxx(11,13,15-16) F?",
    ("unitTest.cxx(36) F", 1, "unitTest.cxx(36) F"): "unitTest.cxx(36) F",
    ("unitTest.cxx(64-65) F", 1, "unitTest.cxx(65) F"): "unitTest.cxx(64-65) F",
    ("integrate.hxx(11,13,20) F", 1, "integrate.hxx(20) F"): "integrate.hxx(11,13,20) F",
}


keepFFlagTab=[
    "integrate.hxx(11,13,20) F",
    "integrate.hxx(11,13,15-16) F?",
    "unitTest.cxx(28)integrate.hxx(9,11) F",
    "integrate.hxx(15-17,26) F?",
    "unitTest.cxx(28)integrate.hxx(9,11,13,15) F",
]

keepMiddlePost=None
#("float integrate<float (*)(float), float>(float (* const&)(float), float, float, unsigned int)+ADDR\tintegrate.hxx:26",
#                "integrate.hxx(11,13,15-16) F?")


def keepBetween(content,conf):
    size=len(content)
    res=[]
    keep=False
    for line in content:
        if (not keep) and conf[0] in line:
            res+=["|*\tcut 8<\tbegin\n"]
            keep=True
        if keep:
            res+=[line]
            if conf[1] in line:
                return res+ ["|*\tcut 8<\tend\n"]

    return res

def filterBack(content, dropInit=None, keep=None):
    drop=False
    if dropInit:
        drop=True

    res=[]
    for line in content:
        if drop:
            if line.startswith(dropInit):
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
    res= removeDuplicationBack(compressBB(res))
    if keep==None:
        return res
    else:
        return keepBetween(res, keep)


def removeDuplicationBack(content):
    size=len(content)
    res=[]
    i=0
    while i<size-1:
        line1=content[i]
        line2=content[i+1]
        if line1==line2 and line1[0]=="|":
            i+=1
        res+=[line1]
        i+=1
    if i<size:
        res+=[content[i]]
    return res



def reduceKey(key):
    return key.replace(" F"," ")

def equalBBKey(key1, key2):
    return reduceKey(key1)==reduceKey(key2)

def compressBB(content):
    compressDicReorder={ reduceKey(begin): (nbInter, reduceKey(end), compressDic[(begin,nbInter, end)]) for (begin,nbInter, end) in compressDic }
    reducedRenameDic={reduceKey(x): reduceKey(renameDic[x])  for x in renameDic}

    size=len(content)
    res=[]
    i=0
    while i<size:
        line=content[i]
        spline=line.split("\t")
        reducedKey=reduceKey(spline[1])

        if reducedKey in compressDicReorder:
            (nbInter, reducedEnd, dest)= compressDicReorder[reducedKey]
            reducedDest=reduceKey(dest)
            if nbInter==None:
                inter=1
                while i+inter+1 < size:
                    line2=content[i+inter+1]
                    spline2=line2.split("\t")
                    if reducedEnd==reduceKey(spline2[1]):
                        if spline[2:]==spline2[2:]: #data equality
                            res+=["C-"+line.replace(spline[1], dest)]
                            interContent=content[i+1:i+inter+1-1] #-1 we remove the back line before the end line
                            remain=content[i+inter+2:]
                            return res+compressBB(interContent+remain)
                        else:
                            break
                    else:
                        inter+=1
            elif i+nbInter+1 < size:
                line2=content[i+nbInter+1]
                spline2=line2.split("\t")
                if spline[2:]==spline2[2:] and equalBBKey(reducedEnd,spline2[1]) :
                    res+=["C-"+line.replace(spline[1],dest)]
                    i+=nbInter+2
                    continue

        if reducedKey in [reduceKey(compressDic[x]) for x  in compressDic]:
            res+=["C-"+line]
            i+=1
            continue

        if reducedKey in reducedRenameDic:
            ignoreCmpKey=reducedKey.replace(" ?"," ")
            renameIgnoreCmpKey=(reducedRenameDic[reducedKey]).replace(" ?"," ")
            res+=["R-"+line.replace(ignoreCmpKey, renameIgnoreCmpKey)]
            i+=1
            continue
        if reducedKey in [reducedRenameDic[x] for x  in reducedRenameDic]:
            res+=["R-"+line]
            i+=1
            continue

        res+=[line]
        i+=1
    return res

def parse(filename, needToPermutate):
    lines=open(filename,"r").readlines()
    header=lines[0:5]
    content=lines[5:]
    dropInit=None
    if "cover0.csv" in filename.name:
        dropInit=dropInitLine
#    else:
#        dropInit=dropInitLineMatch
    keepData=None
#    if "cover1.csv" in filename.name:
#        keepData=keepMiddlePost
    if needToPermutate:
        newHeader, newContent=permutateDDMIN(header, filterBack(content,dropInit=dropInit, keep=keepData))
        res= newHeader+ newContent
    else:
        res= header+filterBack(content,dropInit=dropInit, keep=keepData)
    return removeFFlag(res)

def removeFFlag(content):
    res=[]
    for line in content:
        spline=line.split("\t")
        name=spline[1]

        if any([name in keepF for keepF in keepFFlagTab]):
            res+=[line]
        else:
            newLine=line.replace(" F\t", " \t")
            newLine=newLine.replace(" F?\t", " ?\t")
            res+=[newLine]
    return res


def permutateLine(line, indexDDmin0Tab, indexDDmin1Tab):
    spline=line[0:-1].split("\t")

    for i in range(len(indexDDmin0Tab)):
        index0=indexDDmin0Tab[i]
        index1=indexDDmin1Tab[i]
        spline[index0], spline[index1]= spline[ index1 ], spline[ index0 ]
    res="\t".join(spline)+"\n"
    return res

def permutateDDMIN(header, content):
    headerConf=(header[1][0:-1]).split("\t")
    indexDDmin0Tab= [i for i, x in enumerate(headerConf) if x == "ddmin0"]
    indexDDmin1Tab= [i for i, x in enumerate(headerConf) if x == "ddmin1"]

    if len(indexDDmin0Tab)!=len(indexDDmin1Tab):
        print("invalid ddmin permutation")
        print("indexDDmin0Tab", indexDDmin0Tab)
        print("indexDDmin1Tab", indexDDmin1Tab)
        print("headerConf", headerConf)
        sys.exit(42)
    newHeader=[header[0],
               header[1],
               permutateLine(header[2], indexDDmin0Tab, indexDDmin1Tab), #index
               permutateLine(header[3], indexDDmin0Tab, indexDDmin1Tab), #rounding
               permutateLine(header[4], indexDDmin0Tab, indexDDmin1Tab) #status
               ]
    assert(newHeader[2]==header[2])
    assert(newHeader[3]==header[3])
    newContent=[]
    for line in content:
        if line.startswith("|"):
            newContent+=[line]
        else:
            newContent+=[permutateLine(line, indexDDmin0Tab, indexDDmin1Tab)]
    return (newHeader, newContent)


def needToPermutate(repOrigin):
    ddmin0Line= open(repOrigin/".."/"ddmin0"/"dd.line.include").readline()
    try:
        ddmin1Line= open(repOrigin/".."/"ddmin1"/"dd.line.include").readline()
    except:
        return False
    res=(ddmin0Line > ddmin1Line)
    if res:
        print("need to permutate")
    return res

def loadRef(filename):
    lines=open(filename,"r").readlines()
    return lines

def export(filenameNew, lines):
    handler=open(filenameNew, "w")
    for line in lines:
        handler.write(line)

def generateRef(repOrigin, repDest):
    perm=needToPermutate(repOrigin)
    dataOrgTab=list(repOrigin.glob("backCoverData_seq*_cover*.csv"))
    for dataOrg in dataOrgTab:
        dataFiltered=parse(dataOrg, perm)
        export(repDest / (dataOrg.name + ".filtered.prop"), dataFiltered)


def cmpRepToRef(rep, repRef):
    dataRefTab=list(repRef.glob("backCoverData_seq*_cover*.csv.filtered"))
    dataTab=list(rep.glob("backCoverData_seq*_cover*.csv"))
    perm=needToPermutate(rep)

    if len(dataRefTab)!=len(dataTab):
        print("invalid size")
        print("dataRef: ", [str(x) for x in dataRefTab])
        print("data   : ", [str(x) for x in dataTab])
        sys.exit(42)

    for data in dataTab:
        dataFiltered=parse(data,perm)
        name=data.name
        dataRef=loadRef(repRef/(name+".filtered"))
#        if "cover1.csv" in name:
#            continue

        if dataFiltered!=dataRef:
            print("BEGIN DATAREF: ", (repRef/(name+".filtered")).name)
            for line in dataRef:
                print(line, end="")
            print("END DATAREF")
            print("BEGIN DATA FILTERED: ", data.name)
            for line in dataFiltered:
                print(line, end="")
            print("END DATA FILTERED")

            dataBrut=loadRef(data)
            print("BEGIN DATA BRUT: ", data.name)
            for line in dataBrut:
                print(line, end="")
            print("END DATA BRUT")

            sizeNew=len(dataFiltered)
            sizeRef=len(dataRef)
            print("size (new/ref): ", sizeNew , sizeRef)
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
    arch=platform.uname().machine
    print("arch:", arch)

    if sys.argv[1]=="generateRef":
        repOrg=sys.argv[2]
        repDist=sys.argv[3]
        if arch!="x86_64":
            repDist+=("-"+arch)
        generateRef(Path(repOrg), Path(repDist))
    elif sys.argv[1]=="cmpRepToRef":
        rep=sys.argv[2]
        repRef=sys.argv[3]
        if arch!="x86_64":
            repRef+=("-"+arch)
        cmpRepToRef(Path(rep), Path(repRef))
    else:
        print(sys.argv[1] , " should be generateRef cmpRepToRef")
        sys.exit(42)
