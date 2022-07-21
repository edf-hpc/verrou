#!/usr/bin/python3

import os
import re
import sys
import subprocess


detRounding=["random_det","average_det", "random_comdet","average_comdet"]
verrouConfigList={
    "master":     { "valgrind":"valgrind-3.17.0", "branch_verrou":"master"      ,"flags":"--enable-verrou-fma", "special_rounding_tab":[]},
    "dietzfelbinger": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=dietzfelbinger", "special_rounding_tab":detRounding},
    "multiply_shift": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=multiply_shift", "special_rounding_tab":detRounding},
    "double_tabulation": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=double_tabulation", "special_rounding_tab":detRounding},
    "mersenne_twister": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=mersenne_twister", "special_rounding_tab":detRounding},
}

valgrindConfigList={
    "valgrind-3.17.0": {"file": "valgrind-3.17.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.17.0.tar.bz2"},
    "valgrind-3.19.0": {"file": "valgrind-3.19.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.19.0.tar.bz2"}
}

nbRunTuple=(2,5)

roundingListPerf=["random", "average", "nearest"]
roundingListNum=["random", "average", "nearest", "upward", "downward"]#, "nearest", "upward"]
verrouOptionsList=[("","")]


pathPerfBin="../unitTest/testPerf"
perfBinNameList=["stencil-"+i for i in ["O0-DOUBLE", "O3-DOUBLE", "O0-FLOAT", "O3-FLOAT"] ]
#perfBinNameList=["stencil-"+i for i in ["O3-DOUBLE"] ]
perfCmdParam= "--scale=1 "+str(nbRunTuple[0])

pathNumBin="../unitTest/checkStatRounding"
runNum="run.sh"
extractNum="extract.py"
numEnvConfigTab=[{"ALGO":algo, "ALGO_TYPE":realtype} for realtype in ["double", "float"] for algo in ["Seq", "Rec"]]
#numEnvConfigTab=[{"ALGO":algo, "ALGO_TYPE":realtype} for realtype in ["double", "float"] for algo in ["Seq"]]



def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def buildConfig(name):
    buildRep="buildRep-"+name
    if name=="local":
        if not os.path.exists(buildRep):
            os.mkdir(buildRep)
        return
    verrouConfigParam=verrouConfigList[name]

    valgrindArchive=valgrindConfigList[verrouConfigParam["valgrind"]]["file"]
    if not os.path.exists(valgrindArchive):
        valgrindUrl=valgrindConfigList[verrouConfigParam["valgrind"]]["url"]
        runCmd("wget --output-document=%s %s"%(valgrindArchive,valgrindUrl))

    if not os.path.exists(buildRep):
        runCmd("./buildConfig.sh %s %s %s \"%s\""%(
            buildRep,
            valgrindConfigList[verrouConfigParam["valgrind"]]["file"],
            verrouConfigParam["branch_verrou"],
            verrouConfigParam["flags"])
        )


def runPerfConfig(name):
    repMeasure="buildRep-%s/measure"%(name)
    if not os.path.exists(repMeasure):
        os.mkdir(repMeasure)
    for binName in perfBinNameList:
        for (optName, opt) in verrouOptionsList:
            roundingTab=roundingListPerf + verrouConfigList[name]["special_rounding_tab"]
            for rounding in roundingTab:
                cmd="valgrind --tool=verrou --rounding-mode=%s %s %s %s "%(rounding, optName, pathPerfBin+"/"+binName,perfCmdParam)
                toPrint=True
                for i in range(nbRunTuple[1]):
                    outputName="buildRep-%s/measure/%s_%s_%s.%i"%(name, binName, optName, rounding, i)
                    if not os.path.exists(outputName):
                        if toPrint:
                            print(cmd)
                            toPrint=False
                        if name!="local":
                            runCmd(". ./buildRep-%s/install/env.sh ; %s > %s 2> %s"%(name,cmd,outputName, outputName+".err"))
                        else:
                            runCmd("%s > %s 2> %s"%(cmd,outputName, outputName+".err"))

def runNumConfig(name):
    repNum="buildRep-%s/num"%(name)
    if not os.path.exists(repNum):
        os.mkdir(repNum)

    for (optName, opt) in verrouOptionsList:
        if opt!="":
            print("verrou opt not take into account")
            sys.exit(42)
        roundingTab=roundingListNum + verrouConfigList[name]["special_rounding_tab"]
        roundingStr=",".join(roundingTab)
        cmd="verrou_plot_stat --rep=%s --rounding-list=%s --run-only %s"%( repNum, roundingStr,  pathNumBin+"/"+runNum)
        print(cmd)
        if name!="local":
            runCmd(". ./buildRep-%s/install/env.sh ; %s "%(name,cmd))
        else:
            runCmd("%s "%(cmd))

def plotNumConfig(name):
    repNum="buildRep-%s/num"%(name)
    if not os.path.exists(repNum):
        os.mkdir(repNum)

    histRep="histPng"
    if not os.path.exists(histRep):
            os.mkdir(histRep)

    for (optName, opt) in verrouOptionsList:
        if opt!="":
            print("verrou opt not take into account")
            sys.exit(42)

        roundingTab=roundingListNum + verrouConfigList[name]["special_rounding_tab"]
        roundingStr=",".join(roundingTab)
        for envConfig in numEnvConfigTab:
            envStr=""
            pngStr=os.path.join(histRep,name+"-")
            for key in envConfig:
                envStr+= " "+key+"="+envConfig[key]
                pngStr+=envConfig[key]
            cmd="verrou_plot_stat --rep=%s --relative=104857.6 --rounding-list=%s --png=%s.png %s %s "%( repNum, roundingStr, pngStr, pathNumBin+"/"+runNum, pathNumBin+"/"+extractNum)
            print(envStr, cmd)
            if name!="local":
                runCmd(". ./buildRep-%s/install/env.sh ; %s %s "%(name,envStr,cmd))
            else:
                runCmd("%s %s "%(envStr,cmd))

def runPerfRef():
    repMeasure="measureRef"
    if not os.path.exists(repMeasure):
        os.mkdir(repMeasure)
    for binName in perfBinNameList:
        cmd="%s %s "%(pathPerfBin+"/"+binName,perfCmdParam)
        toPrint=True
        for i in range(nbRunTuple[1]):
            outputName="measureRef/%s.%i"%(binName, i)
            if not os.path.exists(outputName):
                if toPrint:
                    print(cmd)
                    toPrint=False
                runCmd("%s > %s 2> %s"%(cmd,outputName, outputName+".err"))


timeRegExp = re.compile("@time of serial run:\s*\[(.+)\] secondes\s*")
minTimeRegExp = re.compile("@mintime of serial run:\s*\[(.+)\] secondes\s*")

def extractPerfMeasure(fileName):
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
    if resMin==None:
        print("No timer in file %s "%(fileName))
    return {"min": resMin ,"tab":resTab}

def joinMeasure(m1,m2):
    return {"min": min(m1["min"],m2["min"]), "tab": m1["tab"] +  m2["tab"] }


def extractPerf(name):
    res={}
    for binName in perfBinNameList:
        res[binName]={}
        for (optName, opt) in verrouOptionsList:
            res[binName][optName]={}
            for rounding in roundingListPerf+verrouConfigList[name]["special_rounding_tab"]:
                resPerf=None
                for i in range(nbRunTuple[1]):
                    outputName="buildRep-%s/measure/%s_%s_%s.%i"%(name, binName, optName, rounding, i)
                    if resPerf==None:
                        resPerf=extractPerfMeasure(outputName)
                    else:
                        resPerf=joinMeasure(resPerf,extractPerfMeasure(outputName))
                res[binName][optName][rounding]=resPerf
    return res

def extractPerfRef():
    res={}
    for binName in perfBinNameList:
        res[binName]={}

        resPerf=None
        for i in range(nbRunTuple[1]):
            outputName="measureRef/%s.%i"%( binName, i)
            if resPerf==None:
                resPerf=extractPerfMeasure(outputName)
            else:
                resPerf=joinMeasure(resPerf,extractPerfMeasure(outputName))
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
            roundingTab=roundingListPerf +list(set(verrouConfigList[refName]["special_rounding_tab"]).intersection(set(verrouConfigList[newVersion]["special_rounding_tab"])))
            for rounding in roundingTab:
                print("\t\trounding : %s "%(rounding))
                for binName in  perfBinNameList:
                    minTimeRef=dataRef[binName][refOption][rounding]["min"]
                    minTimeNew=dataNew[binName][optionStr][rounding]["min"]
                    print("\t\t\t%s  ratio: %.4f "%(binName, minTimeNew/minTimeRef))

def slowDownAnalyze(data):
    versionTab=[x for x in data.keys()]
    refData=extractPerfRef()
    for version in versionTab:
        print("verrou version : %s"%(version))
        for (optionStr, optionVal) in verrouOptionsList:
            print("\t runtime verrou option : ", optionStr)
            dataNew=data[version]
            roundingTab=roundingListPerf + verrouConfigList[version]["special_rounding_tab"]
            for rounding in roundingTab:
                print("\t\trounding : %s "%(rounding))
                for binName in  perfBinNameList:
                    minTimeNew=dataNew[binName][optionStr][rounding]["min"]
                    refTime=refData[binName]["min"]
                    print("\t\t\t%s  slowDown: x%.1f "%(binName, minTimeNew/refTime))





if __name__=="__main__":
    slowDown=True

    for name in verrouConfigList:
        buildConfig(name)

    for name in verrouConfigList:
        runPerfConfig(name)
        runNumConfig(name)
    if slowDown:
        runPerfRef()

    resAll={}
    for name in verrouConfigList:
        if slowDown:
            resAll[name]=extractPerf(name)

    for name in verrouConfigList:
        plotNumConfig(name)

    nonPerfRegressionAnalyze(resAll,"master")
    print("")
    if slowDown:
        slowDownAnalyze(resAll)
