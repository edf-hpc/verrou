#!/usr/bin/python3

import os
import re
import sys
import subprocess
from tabular import *

from gen_build import workDirectory
from pathlib import Path

fastAnalyze=False
fastAnalyze=True

what="cmpBranch"
if len(sys.argv)==2:
    if sys.argv[1]=="cmpBranch":
        what="cmpBranch"
    elif sys.argv[1]=="cmpHash":
        what="cmpHash"
    elif sys.argv[1]=="cmpStable":
        what="cmpStable"
    else:
        print("invalid cmd")
        sys.exit(42)
if not len(sys.argv) in [1,2]:
    print("invalid cmd")
    sys.exit(42)

#if what=="cmpBranch":
roundingListPerf=["tool_none",  "exclude_all-nc", "exclude_all", "nearest-nc", "nearest", "random", "average"]
buildConfigList=["master","master_fast","back", "back_fast"]
ref_name="master"
detRounding=[]
buildSpecialConfigList=[]
if what=="cmpStable":
    buildConfigList=["stable","current", "current_fast"]
    ref_name="stable"

if what=="cmpHash":
    buildConfigList=["current", "current_fast"]
    buildSpecialConfigList=["dietzfelbinger", "multiply_shift","double_tabulation", "xxhash","mersenne_twister"]
    detRounding=["random_det","average_det", "random_comdet","average_comdet","random_scomdet","average_scomdet", "sr_monotonic","sr_smonotonic"]
    ref_name="current_fast"


drop=10
nbRunTuple=(5,10) #inner outer
minTime=False

if fastAnalyze:
    drop=0
    nbRunTuple=(2,2) #inner outer
    minTime=True

slowDown=True


verrouOptionsList=[("","")]
postFixTab=["O0-DOUBLE-FMA", "O3-DOUBLE-FMA", "O0-FLOAT-FMA", "O3-FLOAT-FMA"]
if fastAnalyze:
    postFixTab=["O3-DOUBLE-FMA","O3-FLOAT-FMA"]
    #postFixTab=["O3-DOUBLE-FMA"]

pathPerfBin=Path("../unitTest/testPerf")
perfBinNameList=["stencil-"+i for i in  postFixTab]
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
    repMeasure=workDirectory / ("buildRep-%s"%(name)) / "measure"
    print("working in %s"%(repMeasure))
    repMeasure.mkdir(exist_ok=True)

    for binName in perfBinNameList:
        for (optName, opt) in verrouOptionsList:
            roundingTab=get_rounding_tab(name)
            for rounding in roundingTab:
                cmd="valgrind --tool=verrou --rounding-mode=%s %s %s %s "%(rounding, optName, pathPerfBin / binName,perfCmdParam)
                if rounding=="exclude_all":
                    cmd="valgrind --tool=verrou --rounding-mode=nearest --exclude=exclude.all.ex %s %s %s "%(optName, pathPerfBin / binName,perfCmdParam)
                if rounding=="exclude_all-nc":
                    cmd="valgrind --tool=verrou --rounding-mode=nearest --exclude=exclude.all.ex --count-op=no %s %s %s "%(optName, pathPerfBin / binName,perfCmdParam)
                if rounding=="nearest-nc":
                    cmd="valgrind --tool=verrou --rounding-mode=nearest --count-op=no %s %s %s "%(optName, pathPerfBin / binName,perfCmdParam)
                if rounding=="tool_none":
                    cmd="valgrind --tool=none %s %s %s "%(optName, pathPerfBin / binName,perfCmdParam)
                if rounding=="fma_only":
                    cmd="valgrind --tool=verrou --vr-instr=mAdd,mSub --rounding-mode=nearest %s %s %s "%(optName, pathPerfBin / binName,perfCmdParam)

                toPrint=True
                for i in range(nbRunTuple[1]):
                    outputName=workDirectory / ("buildRep-%s"%(name)) / "measure" / ("%s_%s_%s.%i"%(binName, optName, rounding, i))
                    if not outputName.is_file():
                        if toPrint:
                            print(cmd)
                            toPrint=False
                        if name!="local":
                            envFile= workDirectory / ("buildRep-"+name) / "install" / "env.sh"
                            runCmd(". %s ; %s > %s 2> %s"%(envFile,cmd,outputName, str(outputName)+".err"))
                        else:
                            runCmd("%s > %s 2> %s"%(cmd,outputName, str(outputName)+".err"))

def runPerfRef():
    repMeasure=workDirectory / "measureRef"
    repMeasure.mkdir(exist_ok=True)
    for binName in perfBinNameList:
        cmd="%s %s "%(pathPerfBin / binName,perfCmdParam)
        toPrint=True
        for i in range(nbRunTuple[1]):
            outputName=repMeasure  / ("%s.%i"%(binName, i))
            if not outputName.is_file():
                if toPrint:
                    print(cmd)
                    toPrint=False
                runCmd("%s > %s 2> %s"%(cmd,outputName, str(outputName)+".err"))


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
                    outputName=workDirectory / ("buildRep-%s"%(name)) / "measure" / ("%s_%s_%s.%i"%(binName, optName, rounding, i))
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
            outputName=workDirectory / "measureRef" / ("%s.%i"%( binName, i))
            if resPerf==None:
                resPerf=extractPerfMeasure(outputName)
            else:
                resPerf=joinMeasure(resPerf,extractPerfMeasure(outputName))
            res[binName]=resPerf
    return res

def meanTab(tab):
    return sum(tab)/ len(tab)


def extractTime(data):
    if minTime==True:
        return data["min"]
    else:
        if drop==0:
            return meanTab(data["tab"])
        tab=data["tab"]
        tab.sort()
        filterTab=tab[drop:-drop]
        return meanTab(filterTab)



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
                    minTimeRef=extractTime(dataRef[binName][refOption][rounding])
                    minTimeNew=extractTime(dataNew[binName][optionStr][rounding])
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
                    minTimeNew=extractTime(dataNew[binName][optionStr][rounding])
                    refTime=extractTime(refData[binName])
                    print("\t\t\t%s  slowDown: x%.1f "%(binName, minTimeNew/refTime))

def headerTab(tab):
    tab.begin()
    if len(postFixTab)==4:
        tab.lineMultiple([(1,"type"), (2,"double"),(2,"float") ])
    if len(postFixTab)==2:
        tab.lineMultiple([(1,"type"), (1,"double"), (1,"float")])
    if len(postFixTab)==1:
        tab.lineMultiple([(1,"type"), (1,"double")])
    tab.endLine()
    if len(postFixTab)==4:
        tab.line(["compilation option", "O0", "O3","O0", "O3"])
    if len(postFixTab)==2:
        tab.line(["compilation option", "O3","O3"])
    if len(postFixTab)==1:
        tab.line(["compilation option", "O3"])
    tab.endLine()
    tab.lineSep()

def addRefDet(roundingTab, build, branchName, withNearestNc=False, withExclude=False, withExcludeNc=False, withFmaOnly=False, withToolNone=False):
    versionStr=""
    if branchName:
        versionStr="("+ build +")"

    roundingTab+=[("nearest"+versionStr, "nearest",build)]
    if withNearestNc:
        roundingTab+=[("nearest-nc"+versionStr, "nearest-nc", build)]
    if withExclude:
        roundingTab+=[("exclude_all"+versionStr, "exclude_all", build)]
    if withExcludeNc:
        roundingTab+=[("exclude_all-nc"+versionStr, "exclude_all-nc", build)]
    if withToolNone:
        roundingTab+=[("tool_none"+versionStr, "tool_none", build)]
    if withFmaOnly:
        roundingTab+=[("fma_only"+versionStr, "fma_only", build)]
    return roundingTab

def generateTab( tab, data, roundingTab, optionStr):
    headerTab(tab)
    refData=extractPerfRef()

    for confLine in roundingTab:
        if confLine=="SEPARATOR":
            tab.lineSep()
            continue
        head= [confLine[0]]
        def content(post, rounding, configure):
            binName="stencil-"+post
            minTimeNew=extractTime(data[configure][binName][optionStr][rounding])
            refTime=extractTime(refData[binName])
            slowDownF=minTimeNew/refTime
            slowDown="x%.1f "%(slowDownF)
            if slowDownF>100.:
                slowDown="x%.0f "%(slowDownF)
            return slowDown
        contentTab=[ content(post,confLine[1],confLine[2]) for post in postFixTab ]
        tab.line(head+contentTab)
        tab.endLine()

    tab.end()


def feedPerfTab(tab, data, buildList, detTab=["_det","_comdet"], branchTab=False, extraRounding=[], optionStr="", withNearestNc=False,withExclude=False, withExcludeNc=False, withFmaOnly=False, withToolNone=False):

    roundingTab=[]
    if branchTab:
        assert(len(detTab)==0)
        for branch in buildList:
            for rounding in roundingListPerf:
                roundingTab+=[(rounding+"("+branch+")", rounding, branch)]
            roundingTab+=["SEPARATOR"]
    else:
        addRefDet(roundingTab,  ref_name, False, withNearestNc, withExclude, withExcludeNc, withFmaOnly, withToolNone)
        roundingTab+=["SEPARATOR"]
        for rd in ["random","average"]:
            roundingTab+=[(rd, rd, ref_name)]

            for detType in detTab:
                for gen in buildList:#on supprime master
                    roundingTab+=[(rd+detType+"("+gen+")",rd+detType,gen)]
            roundingTab+=["SEPARATOR"]
        roundingTab=roundingTab[0:-1]
        if extraRounding != []:
            roundingTab+=["SEPARATOR"]
            for rd in extraRounding:
                for gen in buildList:#on supprime master
                    roundingTab+=[(rd+"("+gen+")",rd,gen)]
    generateTab(tab,data,roundingTab, optionStr)



if __name__=="__main__":

    runCmd("make -C ../unitTest/testPerf/")

    for name in buildConfigList+buildSpecialConfigList:
        runPerfConfig(name)
    runPerfRef()

    resAll={}
    for name in buildConfigList+buildSpecialConfigList:
        resAll[name]=extractPerf(name)

    #print(resAll)
    if what=="cmpBranch":
        print("ref_name:",ref_name)
        nonPerfRegressionAnalyze(resAll, ref_name)

        tab=tabularLatex("lcccc", output="slowDownBranch.tex")
        feedPerfTab(tab,resAll,buildConfigList, detTab=[], branchTab=True)
    if what=="cmpStable":
        print("ref_name:",ref_name)
        nonPerfRegressionAnalyze(resAll, ref_name)

    if what=="cmpHash":
        tab=tabularLatex("lcccc", output="slowDownHash.tex")
        feedPerfTab(tab,resAll,buildSpecialConfigList, detTab=[])

        tab=tabularLatex("lcccc", output="slowDownHash_det.tex")
        feedPerfTab(tab,resAll,buildSpecialConfigList, detTab=["_det"])

        tab=tabularLatex("lcccc", output="slowDownHash_comdet.tex")
        feedPerfTab(tab,resAll,buildSpecialConfigList, detTab=["_comdet"])

        tab=tabularLatex("lcccc", output="slowDownHash_scomdet.tex")
        feedPerfTab(tab,resAll,buildSpecialConfigList, detTab=["_scomdet"])

        tab=tabularLatex("lcccc", output="slowDownHash_doubleTab.tex")
        feedPerfTab(tab,resAll,["double_tabulation"], detTab=["_det","_comdet","_scomdet"])

        tab=tabularLatex("lcccc", output="slowDownHash_xxhash.tex")
        feedPerfTab(tab,resAll,["xxhash"], detTab=["_det","_comdet","_scomdet"], extraRounding=["sr_monotonic","sr_smonotonic"], withToolNone=True, withExcludeNc=True, withExclude=True, withNearestNc=True)

        tab=tabularLatex("lcccc", output="slowDownHash_all.tex")
        feedPerfTab(tab, resAll,["dietzfelbinger", "multiply_shift", "double_tabulation", "xxhash","mersenne_twister"], detTab=["_det","_comdet","_scomdet"], extraRounding=["sr_monotonic","sr_smonotonic"])

