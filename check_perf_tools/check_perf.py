#!/usr/bin/python3

import os
import re
import sys

verrouConfigList={
    "master":     { "valgrind":"valgrind-3.17.0", "branch_verrou":"master"     ,"flags":"--enable-verrou-fma"},
    "random_det": { "valgrind":"valgrind-3.17.0", "branch_verrou":"random_det" ,"flags":"--enable-verrou-fma"}
    }


valgrindConfigList={
    "valgrind-3.17.0": {"file": "/home/E52654/srcVerrou/valgrind-3.17.0.tar.bz2"}
}

nbRunTuple=(5,5)

roundingList=["random", "average", "nearest", "upward"]
verrouOptionsList=[("","")]


pathBin="../unitTest/testPerf"
binNameList=["stencil-"+i for i in ["O0-DOUBLE", "O3-DOUBLE", "O0-FLOAT", "O3-FLOAT"] ]


cmdParam= "--scale=1 "+str(nbRunTuple[0])


def buildConfig(name):
    verrouConfigParam=verrouConfigList[name]
    buildRep="buildRep-"+name
    os.system("./buildConfig.sh %s %s %s \"%s\""%(
        buildRep,
        valgrindConfigList[verrouConfigParam["valgrind"]]["file"],
        verrouConfigParam["branch_verrou"],
        verrouConfigParam["flags"])
    )


def runConfig(name):
    repMeasure="buildRep-%s/measure"%(name)
    if not os.path.exists(repMeasure):
        os.mkdir(repMeasure)
    for binName in binNameList:
        for (optName, opt) in verrouOptionsList:
            for rounding in roundingList:
                cmd="valgrind --tool=verrou --rounding-mode=%s %s %s %s "%(rounding, optName, pathBin+"/"+binName,cmdParam)
                toPrint=True
                for i in range(nbRunTuple[1]):
                    outputName="buildRep-%s/measure/%s_%s_%s.%i"%(name, binName, optName, rounding, i)                    
                    if not os.path.exists(outputName):
                        if toPrint:
                            print(cmd)
                            toPrint=False
                        os.system(". ./buildRep-%s/install/env.sh ; %s > %s 2> %s"%(name,cmd,outputName, outputName+".err"))
def runRef():
    repMeasure="measureRef"
    if not os.path.exists(repMeasure):
        os.mkdir(repMeasure)
    for binName in binNameList:
        cmd="%s %s "%(pathBin+"/"+binName,cmdParam)
        toPrint=True
        for i in range(nbRunTuple[1]):
            outputName="measureRef/%s.%i"%(binName, i)
            if not os.path.exists(outputName):
                if toPrint:
                    print(cmd)
                    toPrint=False
                os.system("%s > %s 2> %s"%(cmd,outputName, outputName+".err"))


timeRegExp = re.compile("@time of serial run:\s*\[(.+)\] secondes\s*")
minTimeRegExp = re.compile("@mintime of serial run:\s*\[(.+)\] secondes\s*")

def extractMeasure(fileName):
    resTab=[]
    resMin=None
    for line in open(fileName).readlines():
        m=timeRegExp.match(line)
        if m!=None:
            t=m.group(1)
            resTab+=[float(t)]
            continue
        m=minTimeRegExp.match(line)
        if m!=None:
            t=m.group(1)
            resMin=float(t)
            continue
    return {"min": resMin ,"tab":resTab}

def joinMeasure(m1,m2):
    return {"min": min(m1["min"],m2["min"]), "tab": m1["tab"] +  m2["tab"] }


def extract(name):
    res={}
    for binName in binNameList:
        res[binName]={}
        for (optName, opt) in verrouOptionsList:
            res[binName][optName]={}
            for rounding in roundingList:
                resPerf=None
                for i in range(nbRunTuple[1]):
                    outputName="buildRep-%s/measure/%s_%s_%s.%i"%(name, binName, optName, rounding, i)
                    if resPerf==None:
                        resPerf=extractMeasure(outputName)
                    else:
                        resPerf=joinMeasure(resPerf,extractMeasure(outputName))
                res[binName][optName][rounding]=resPerf
    return res

def extractRef():
    res={}
    for binName in binNameList:
        res[binName]={}

        resPerf=None
        for i in range(nbRunTuple[1]):
            outputName="measureRef/%s.%i"%( binName, i)
            if resPerf==None:
                resPerf=extractMeasure(outputName)
            else:
                resPerf=joinMeasure(resPerf,extractMeasure(outputName))
            res[binName]=resPerf
    return res



def nonPerfRegressionAnalyze(data, refName, refOption=""):
    newVersionTab=[x for x in data.keys() if not x==refName]
    dataRef=data[refName]
    print("refernce verrou version : %s"%(refName))
    for newVersion in newVersionTab:
        print("verrou version : %s"%(newVersion))
        for (optionStr, optionVal) in verrouOptionsList:
            print("\truntime verrou option : ", optionStr)
            dataNew=data[newVersion]

            for rounding in roundingList:
                print("\t\trounding : %s "%(rounding))
                for binName in  binNameList:
                    minTimeRef=dataRef[binName][refOption][rounding]["min"]
                    minTimeNew=dataNew[binName][optionStr][rounding]["min"]
                    print("\t\t\t%s  ratio: %.4f "%(binName, minTimeNew/minTimeRef))

def slowDownAnalyze(data):
    versionTab=[x for x in data.keys()]
    refData=extractRef()
    for version in versionTab:
        print("verrou version : %s"%(version))
        for (optionStr, optionVal) in verrouOptionsList:
            print("\t runtime verrou option : ", optionStr)
            dataNew=data[version]

            for rounding in roundingList:
                print("\t\trounding : %s "%(rounding))
                for binName in  binNameList:
                    minTimeNew=dataNew[binName][optionStr][rounding]["min"]
                    refTime=refData[binName]["min"]
                    print("\t\t\t%s  slowDown: x%.1f "%(binName, minTimeNew/refTime))


if __name__=="__main__":
    slowDown=False

    for name in verrouConfigList:
        runConfig(name)
    if slowDown:
        runRef()

    resAll={}
    for name in verrouConfigList:
        resAll[name]=extract(name)

    nonPerfRegressionAnalyze(resAll,"master")
    print("")
    if slowDown:
        slowDownAnalyze(resAll)
