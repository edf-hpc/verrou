#!/usr/bin/python3

import os
import re
import sys
import subprocess

verrouConfigList={
    "local":      {"special_rounding_tab":["random_det", "average_det"]},
    "master":     { "valgrind":"valgrind-3.17.0", "branch_verrou":"master"      ,"flags":"--enable-verrou-fma", "special_rounding_tab":[]},
#    "random_det": { "valgrind":"valgrind-3.17.0", "branch_verrou":"random_det"  ,"flags":"--enable-verrou-fma", "special_rounding_tab":["random_det"]},
#    "average_det":{ "valgrind":"valgrind-3.17.0", "branch_verrou":"average_det" ,"flags":"--enable-verrou-fma", "special_rounding_tab":["random_det","average_det"]},
    "merge": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --enable-verrou-fast-gen=yes ", "special_rounding_tab":["random_det","average_det"]},
    "merge_ref":  { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --enable-verrou-det-ref-hash=yes --enable-verrou-fast-gen=no ", "special_rounding_tab":["random_det","average_det"]},
    "merge_fasthash":  { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --enable-verrou-det-fast-hash=yes", "special_rounding_tab":["random_det","average_det"]},
}


valgrindConfigList={
    "valgrind-3.17.0": {"file": "/home/E52654/srcVerrou/valgrind-3.17.0.tar.bz2"}
}

nbRunTuple=(2,5)

roundingList=["random", "average", "nearest", "upward"]
roundingList=["random", "average"]#, "nearest", "upward"]
verrouOptionsList=[("","")]


pathPerfBin="../unitTest/testPerf"
perfBinNameList=["stencil-"+i for i in ["O0-DOUBLE", "O3-DOUBLE", "O0-FLOAT", "O3-FLOAT"] ]
perfCmdParam= "--scale=1 "+str(nbRunTuple[0])

pathNumBin="../unitTest/checkStatRounding"
runNum="run.sh"
extractNum="extract.py"
numEnvConfigTab=[{"ALGO":algo, "ALGO_TYPE":realtype} for realtype in ["double", "float"] for algo in ["Seq", "Rec"]]




def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def buildConfig(name):
    buildRep="buildRep-"+name
    if name=="local":
        if not os.path.exists(buildRep):
            os.mkdir(buildRep)
        return
    verrouConfigParam=verrouConfigList[name]
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
            roundingTab=roundingList + verrouConfigList[name]["special_rounding_tab"]
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
        roundingTab=roundingList + verrouConfigList[name]["special_rounding_tab"]
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

    for (optName, opt) in verrouOptionsList:
        if opt!="":
            print("verrou opt not take into account")
            sys.exit(42)

        roundingTab=roundingList + verrouConfigList[name]["special_rounding_tab"]
        roundingStr=",".join(roundingTab)
        for envConfig in numEnvConfigTab:
            envStr=""
            for key in envConfig:
                envStr+= " "+key+"="+envConfig[key]

            cmd="verrou_plot_stat --rep=%s --rounding-list=%s %s %s"%( repNum, roundingStr,  pathNumBin+"/"+runNum, pathNumBin+"/"+extractNum)
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
            for rounding in roundingList+verrouConfigList[name]["special_rounding_tab"]:
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
            roundingTab=roundingList +list(set(verrouConfigList[refName]["special_rounding_tab"]).intersection(set(verrouConfigList[newVersion]["special_rounding_tab"])))
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
            roundingTab=roundingList + verrouConfigList[version]["special_rounding_tab"]
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
