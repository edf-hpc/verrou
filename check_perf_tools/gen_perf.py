#!/usr/bin/python3

import os
import re
import sys
import subprocess
from tabular import *

detRounding=["random_det","average_det", "random_comdet","average_comdet"]

buildConfigList=["master","dietzfelbinger", "multiply_shift", "double_tabulation", "mersenne_twister"]

nbRunTuple=(2,5)
roundingListPerf=["random", "average", "nearest"]
verrouOptionsList=[("","")]


pathPerfBin="../unitTest/testPerf"
postFixTab=["O0-DOUBLE", "O3-DOUBLE", "O0-FLOAT", "O3-FLOAT"]
perfBinNameList=["stencil-"+i for i in  postFixTab]
#perfBinNameList=["stencil-"+i for i in ["O3-DOUBLE"] ]
perfCmdParam= "--scale=1 "+str(nbRunTuple[0])

def special_rounding(name):
    if name=="master":
        return []
    else:
        return detRounding


def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def runPerfConfig(name):
    repMeasure="buildRep-%s/measure"%(name)
    if not os.path.exists(repMeasure):
        os.mkdir(repMeasure)
    for binName in perfBinNameList:
        for (optName, opt) in verrouOptionsList:
            roundingTab=roundingListPerf + special_rounding(name)
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
            for rounding in roundingListPerf+special_rounding(name):
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
            roundingTab=roundingListPerf +list(set(special_rounding(refName)).intersection(set(special_rounding(newVersion))))
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
            roundingTab=roundingListPerf + special_rounding(version)
            for rounding in roundingTab:
                print("\t\trounding : %s "%(rounding))
                for binName in  perfBinNameList:
                    minTimeNew=dataNew[binName][optionStr][rounding]["min"]
                    refTime=refData[binName]["min"]
                    print("\t\t\t%s  slowDown: x%.1f "%(binName, minTimeNew/refTime))

def feedPerfTab(data, buildList, detTab=["_det","_comdet"], optionStr=""):


#    codeTabName=[x.replace("FLOAT","float").replace("DOUBLE","double")for x in postFixTab]
    tab.begin()

    tab.lineMultiple([(1,"type"), (2,"double"),(2,"float") ])
    tab.endLine()
    tab.line(["compilation option", "O0", "O3","O0", "O3"])
    tab.endLine()
    tab.lineSep()

    roundingTab=[("nearest", "nearest", "dietzfelbinger"),"SEPARATOR"]
    for rd in ["random","average"]:
        roundingTab+=[(rd, rd,"dietzfelbinger")]
        for gen in buildList:#on supprime master
            for detType in detTab:
                roundingTab+=[(rd+detType+"("+gen+")",rd+detType,gen)]
        roundingTab+=["SEPARATOR"]
    roundingTab=roundingTab[0:-1]

    refData=extractPerfRef()

    for confLine in roundingTab:
        if confLine=="SEPARATOR":
            tab.lineSep()
            continue

        head= [confLine[0]]
        def content(post, rounding, configure):
            binName="stencil-"+post
            minTimeNew=data[configure][binName][optionStr][rounding]["min"]
            refTime=refData[binName]["min"]
            slowDown="x%.1f "%(minTimeNew/refTime)
            return slowDown
        contentTab=[ content(post,confLine[1],confLine[2]) for post in postFixTab ]
        tab.line(head+contentTab)
        tab.endLine()

    tab.end()



if __name__=="__main__":
    slowDown=True

    for name in buildConfigList:
        runPerfConfig(name)
    if slowDown:
        runPerfRef()

    resAll={}
    for name in buildConfigList:
        if slowDown:
            resAll[name]=extractPerf(name)

    nonPerfRegressionAnalyze(resAll,"master")
    print("")
    if slowDown:

        tab=tabularLatex("lcccc", output="slowDown.tex")
#        tab=tabular()
        feedPerfTab(resAll,buildConfigList[1:], detTab=["_det","_comdet"])

