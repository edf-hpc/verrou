import sys
from pathlib import Path
import math
import re


from BBTools import backCovReader, coverageReader, traceName

def findCoverRep(baseRep):
    return baseRep.glob("*-trace/**/cover/")

def extractPidPathTime(fileName, trace_kind="bb_cover"):
    """extract the pid d'un fichier de la form trace_bb_cov.log-PID[.gz]"""
    rep=fileName.parent
    if rep=="":
        rep="."
    baseFile=fileName.name
    begin=traceName(trace_kind).covPrefixName()

    mtime=(fileName.stat()).st_mtime
    if baseFile.startswith(begin):
        pid=int((baseFile.replace(begin,'')).replace(".gz",""))
        return (pid,fileName, mtime)
    return None

def detect_trace_kind(baseRep):
    listBBNum=len(list(baseRep.glob("*-trace/**/cover/trace_bb_cov.log-*")))
    listBackNum=len(list(baseRep.glob("*-trace/**/cover/backCoverInfo-*")))
    if listBackNum!=0 and listBBNum!=0:
        print("incompatible trace format")
        sys.exit(42)
    if listBackNum==0 and listBBNum==0:
        print("no trace found")
        sys.exit(42)
    if listBBNum!=0:
        return "bb_cover"
    if listBackNum!=0:
        return "back_cover"
    return None


def selectSortedPidFromGlob(fileNameTab,trace_kind="bb_cover"):
    """Return a list of pid from a list of file by using extractPidRep"""
    pidPathTimeList=[extractPidPathTime(fileName,trace_kind) for fileName in fileNameTab]
    sortedPidList=sorted(pidPathTimeList, key=lambda x: x[2])
    return [x[0:2] for x in sortedPidList ]


def searchSeq( conf, trace_kind):
    fileCoverBackTab=list(conf.glob(traceName(trace_kind).covPrefixName()+"*"))
    pidPathTab=selectSortedPidFromGlob(fileCoverBackTab, trace_kind=trace_kind)
    return pidPathTab


def analysePath(baseRep, pathConf, trace_kind):
    relativePathStr=str(pathConf.relative_to(baseRep))
    print("relativePathStr", relativePathStr)
    dataStr=relativePathStr.split("/")
    conf_name=dataStr[0].replace("-trace","")
    rounding=dataStr[1]
    indexNext=2
    if dataStr[1]=="det":
        rounding=dataStr[2]
        indexNext=3
    ddRunIndex=int( dataStr[indexNext].replace("dd.run",""))

    returnPath=pathConf / ".." / "dd.return.value"
    returnValue=int(open(returnPath).readline())
    returnStr="OK"
    if returnValue!=0:
        returnStr="KO"

    pidPathTab=searchSeq(pathConf, trace_kind)
    confData={"pathConf":pathConf,
              "runIndex":ddRunIndex,
              "rounding": rounding,
              "conf_name": conf_name,
              "return_value": returnStr,
              "pidPathOrderList": pidPathTab
              };
    return confData

def keyConf(conf):
    if conf=="NoPerturbation":
        return 0
    if conf=="FullPerturbation":
        return 1
    if conf=="rddmin-cmp":
        return 2
    if conf.startswith("ddmin"):
        return 10+ int( conf.replace("ddmin","")) # 10 we keep margin

def keyRounding(rounding):
    tab=["default", "nearest", "downward", "upward", "random", "nearness", "sr_monotonic", "sr_smonotonic"]
    if rounding in tab:
        return tab.index(rounding)
    return len(tab)



def sortDataParsed(confDataTab):
    return sorted(confDataTab,key=lambda x: (keyConf(x["conf_name"]), keyRounding(x["rounding"]), x["runIndex"]))


def getRefIndex(confDataTab):
    for index in range(len(confDataTab)):
        if dataParsedTab[index]["conf_name"]=="NoPerturbation":
            return index
    return 0




def csvHeader(dataParsedTab, estimatorTab, trace_kind):
    levelPrefix=None
    if trace_kind=="bb_cover":
        levelPrefix=""
    elif trace_kind=="back_cover":
        levelPrefix="\t" #column for deep

    estTab=[x for x in estimatorTab if x!="data"]
    res=levelPrefix+"" +"\t" + "\t".join(estTab)+"\n"
    res+=levelPrefix+"conf_name" +"\t" + "\t".join([""]*len(estTab))+ "\t"+"\t".join([str(x["conf_name"]) for x in dataParsedTab]) +"\n" 
    res+=levelPrefix+"indexRun" +"\t" + "\t".join([""]*len(estTab))+ "\t"+ "\t".join([str(x["runIndex"]) for x in dataParsedTab])  +"\n"
    res+=levelPrefix+"rounding" +"\t" + "\t".join([""]*len(estTab))+ "\t"+"\t".join([str(x["rounding"]) for x in dataParsedTab])  +"\n"
    res+=levelPrefix+"return_value" +"\t" +"\t".join([""]*len(estTab))+ "\t"+ "\t".join([str(x["return_value"]) for x in dataParsedTab])  +"\n"

    return res



class genMerge:

    def __init__(self, pidRef, pathRef,statusRef, trace_kind):

        if trace_kind=="back_cover":
            self.root=backCovReader(pidRef, pathRef,statusRef, trace_kind, mergeRoot=True)
        elif trace_kind=="bb_cover":
            self.root=coverageReader(pidRef, pathRef,statusRef, trace_kind, mergeRoot=True)
        else:
            pass

    def current(self,  pid, path, status, trace_kind):
        if trace_kind=="back_cover":
            return backCovReader( pid, path, status, trace_kind, mergeRoot=False)
        elif trace_kind=="bb_cover":
            return coverageReader(pid, path,status, trace_kind, mergeRoot=False)

    def addMerge(self, current):
        self.root.addMerge(current)

    def endMerge(self):
        self.root.endMerge()

    def writeCSV(self, pathStr, header, outputTypeTab):
        self.root.writeCSV(pathStr, header, outputTypeTab=outputTypeTab)


def cleanIntermediateFile(dataParsedTab, trace_kind):
    fileToDelTab=[]
    patternTab=[]
    if trace_kind=="bb_cover":
        patternTab+=["trace_bb_cov.log-","trace_bb_info.log-"]
    if trace_kind=="back_cover":
        patternTab+=["backAddrInfo-","backCoverInfo-","bbAddrInfo-"]

    for config in dataParsedTab:
        coverRep=config["pathConf"]
        for pattern in patternTab:
            fileToDelTab+=list(coverRep.glob(pattern+"*"))

    for fileToDel in fileToDelTab:
        fileToDel.unlink()

def generateTraceAnalysis(baseRep,trace_kind, estimatorTab, clean):
    pathTab=list(findCoverRep(baseRep))
    if len(pathTab)==0:
        print("invalid baseRep:" , baseRep)
        sys.exit(42)
    else:
        print("number of path configuration : ", len(pathTab))

    dataParsed=[analysePath(baseRep,path, trace_kind) for path in pathTab]
    dataParsed=sortDataParsed(dataParsed)

    maxSeq=max([len(config["pidPathOrderList"]) for config in dataParsed])
    minSeq=max([len(config["pidPathOrderList"]) for config in dataParsed])
    if maxSeq != minSeq:
        print("incoherent number of process")
        print("dataParsed", dataParsed)
        sys.exit(42)

    header=csvHeader(dataParsed, estimatorTab, trace_kind)

    statusTab=[data ["return_value"]=="OK" for data in dataParsed]
    statusRef=statusTab[0]
    mergeSize=len(statusTab)

    for seqIndex in range(maxSeq):
        pidTab=[data["pidPathOrderList"][seqIndex][0] for data in dataParsed]
        pathRepTab=[ (data["pidPathOrderList"][seqIndex][1]).parent for data in dataParsed] 

        pidRef=pidTab[0]
        repRef=pathRepTab[0]

        mergeTool=genMerge(pidRef, Path(repRef),statusRef, trace_kind)

        printIndex=[int(float(p) * mergeSize /100.)  for p in (list(range(0,100,10))+[1,5])]
        printIndex +=[1,  mergeSize-1]

        for i in range(1,mergeSize):
            current=mergeTool.current(pidTab[i], pathRepTab[i], statusTab[i], trace_kind)
            mergeTool.addMerge(current)
            if i in printIndex:
                pourcent=float(i)/ float(mergeSize-1)
                print( "%.1f"%(pourcent*100)    +"% of coverage data merged")

        mergeTool.endMerge()

        output=baseRep / "trace_analysis"
        output.mkdir(exist_ok=True)
        pathStr= str(output)+ "/"+str(trace_kind).replace("_cover","Cover")+"Data_seq%i_cover__NUM_COV__.csv"%(seqIndex)

        mergeTool.writeCSV(pathStr, header, outputTypeTab=estimatorTab)
    if clean:
        cleanIntermediateFile(dataParsed, trace_kind)
