#!/usr/bin/python3

import os
import re
import sys
import subprocess
from tabular import *


roundingListPerf=["random", "average","nearest"]
detRounding=["random_det","average_det", "random_comdet","average_comdet","random_scomdet","average_scomdet", "sr_monotonic","sr_smonotonic"]

buildConfigList=["stable","current", "current_fast"]
buildSpecialConfigList=["dietzfelbinger", "multiply_shift","double_tabulation", "xxhash","mersenne_twister"]

nbRunTuple=(5,5) #inner outer
ref_name="current_fast"
slowDown=True


# buildConfigList=["current", "current-upgrade"]
# ref_name="current"
# buildSpecialConfigList=[]
# detRounding=[]
# nbRunTuple=(5,20) #inner outer
# slowDown=False

buildConfigList=["current", "last_stable"]
ref_name="current"
buildSpecialConfigList=[]
detRounding=[]
nbRunTuple=(5,2) #inner outer
slowDown=True


verrouOptionsList=[("","")]

postFixTab=["O0-DOUBLE-FMA", "O3-DOUBLE-FMA", "O0-FLOAT-FMA", "O3-FLOAT-FMA"]
postFixTab=["O3-DOUBLE-FMA"]


pathPerfBin="../unitTest/testPerf"
perfBinNameList=["stencil-"+i for i in  postFixTab]
#perfBinNameList=["stencil-"+i for i in ["O3-DOUBLE"] ]
perfCmdParam= "--scale=1 "+str(nbRunTuple[0])

def get_rounding_tab(name):
    if name in ["current","current_fast", "current-upgrade"]:
        return roundingListPerf+detRounding
    if name in buildConfigList:
        return roundingListPerf
    if name in buildSpecialConfigList:
        return detRounding


def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def runPerfConfig(name):
    repMeasure="buildRep-%s/measure"%(name)
    print("working in %s"%(repMeasure))
    if not os.path.exists(repMeasure):
        os.mkdir(repMeasure)
    for binName in perfBinNameList:
        for (optName, opt) in verrouOptionsList:
            roundingTab=get_rounding_tab(name)
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
            for rounding in get_rounding_tab(name):
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
    print("reference verrou version : %s"%(refName))
    for newVersion in newVersionTab:
        print("verrou version : %s"%(newVersion))
        for (optionStr, optionVal) in verrouOptionsList:
            print("\truntime verrou option : ", optionStr)
            dataNew=data[newVersion]
            roundingTab=get_rounding_tab(newVersion)#  roundingListPerf +list(set(special_rounding(refName)).intersection(set(special_rounding(newVersion))))
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
            roundingTab= get_rounding_tab(name) #roundingListPerf + special_rounding(version)
            for rounding in roundingTab:
                print("\t\trounding : %s "%(rounding))
                for binName in  perfBinNameList:
                    minTimeNew=dataNew[binName][optionStr][rounding]["min"]
                    refTime=refData[binName]["min"]
                    print("\t\t\t%s  slowDown: x%.1f "%(binName, minTimeNew/refTime))

def feedPerfTab(data, buildList, detTab=["_det","_comdet"], extraRounding=[], optionStr=""):


#    codeTabName=[x.replace("FLOAT","float").replace("DOUBLE","double")for x in postFixTab]
    tab.begin()
    if len(postFixTab)==4:
        tab.lineMultiple([(1,"type"), (2,"double"),(2,"float") ])
    if len(postFixTab)==1:
        tab.lineMultiple([(1,"type"), (1,"double")])
    tab.endLine()
    if len(postFixTab)==4:
        tab.line(["compilation option", "O0", "O3","O0", "O3"])
    if len(postFixTab)==1:
        tab.line(["compilation option", "O3"])
    tab.endLine()
    tab.lineSep()

    roundingTab=[("nearest", "nearest", "current"),"SEPARATOR"]
    for rd in ["random","average"]:
        roundingTab+=[(rd, rd,"current")]

        for gen in buildList:#on supprime master
            for detType in detTab:
                roundingTab+=[(rd+detType+"("+gen+")",rd+detType,gen)]
        roundingTab+=["SEPARATOR"]
    roundingTab=roundingTab[0:-1]
    if extraRounding != []:
        roundingTab+=["SEPARATOR"]
        for rd in extraRounding:
            for gen in buildList:#on supprime master
                roundingTab+=[(rd+"("+gen+")",rd,gen)]

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


    runCmd("make -C ../unitTest/testPerf/")

    for name in buildConfigList+buildSpecialConfigList:
        runPerfConfig(name)
    if slowDown:
        runPerfRef()

    resAll={}
    for name in buildConfigList+buildSpecialConfigList:
        resAll[name]=extractPerf(name)

    print(resAll)
    print("ref_name:",ref_name)
    nonPerfRegressionAnalyze(resAll, ref_name)
    print("")
    if slowDown:

        tab=tabularLatex("lcccc", output="slowDown_det.tex")
        feedPerfTab(resAll,buildSpecialConfigList, detTab=["_det"])

        tab=tabularLatex("lcccc", output="slowDown_comdet.tex")
        feedPerfTab(resAll,buildSpecialConfigList, detTab=["_comdet"])

        tab=tabularLatex("lcccc", output="slowDown_scomdet.tex")
        feedPerfTab(resAll,buildSpecialConfigList, detTab=["_scomdet"])

#        tab=tabularLatex("lcccc", output="slowDown_doubleTab.tex")
#        feedPerfTab(resAll,["double_tabulation"], detTab=["_det","_comdet","_scomdet"])

        tab=tabularLatex("lcccc", output="slowDown_xxhash.tex")
        feedPerfTab(resAll,["xxhash"], detTab=["_det","_comdet","_scomdet"], extraRounding=["sr_monotonic","sr_smonotonic"])

        sys.exit()
        tab=tabular()
        feedPerfTab(resAll,buildSpecialConfigList, detTab=["_det","_comdet"])

