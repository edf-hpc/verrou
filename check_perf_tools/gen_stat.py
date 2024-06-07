#!/usr/bin/python3

import os
import re
import sys
import subprocess
import math
from tabular import *

from gen_build import workDirectory


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

#cmpBranch
buildConfList=[ "master", "seed", "llo"]
detRounding=["random_det","average_det", "random_comdet","average_comdet", "random_scomdet","average_scomdet", "sr_monotonic", "sr_smonotonic"]
roundingListNum=["random", "average", "nearest", "upward", "downward"]
refName="master"

#cmpHash
if what=="cmpHash":
    buildConfList=[ "current","dietzfelbinger","multiply_shift", "double_tabulation", "xxhash","mersenne_twister"]
    refName="current"

pathNumBin="../unitTest/checkStatRounding"
runNum="run.sh"
extractNum="extract.py"
numEnvConfigTab=[{"ALGO":algo, "ALGO_TYPE":realtype} for realtype in ["double", "float"] for algo in ["Seq", "Rec"]]

buildConfListXoshiro=[]#"xoshiro","xoshiro-2","xoshiro-8"]

def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def runCmdToLines(cmd):
    process=subprocess.run(cmd, shell=True,capture_output=True, text=True)
    return process.stdout.split("\n")

def parsePlotStat(lineTab):
    toParse=False
    res={}
    for line in lineTab:
        if line.startswith("[mca] estimator"):
            continue
        if line.startswith("[mca] all:"):
            toParse=True

        if toParse:
            if not line.startswith("[mca]"):
                spline=line.strip().split("\t")
                rounding=spline[0][0:-1]
                absEst=spline[1]
                relEst=spline[2]
                bit=spline[3].replace("bit","")
                res[rounding]={"absEst":absEst, "relEst": relEst , "bit":bit }
                if rounding=="all":
                    toParse=False
            else:
                line=line.replace("[mca] ","")
                spline=line.strip().split("\t")
                rounding=spline[0][0:-1]
                relEst=spline[1]
                bit=spline[2].replace("bit","")
                res[rounding]["mca"]=relEst
                res[rounding]["mca_bit"]=bit
                if rounding=="all":
                    toParse=False

        if line.startswith("refValue[nearest]"):
            res["nearest"]=(line.split(":")[1]).strip()

        if line.startswith("estimator"):
            toParse=True
    return res



def extractStat():
    res={}
    for name in buildConfList+buildConfListXoshiro:
        resName={}
        repNum=os.path.join(workDirectory,"buildRep-%s/num"%(name))
        roundingTab=roundingListNum + detRounding
        roundingStr=",".join(roundingTab)
        for envConfig in numEnvConfigTab:
            envStr=""
            concatStr=""
            for key in envConfig:
                envStr+= " "+key+"="+envConfig[key]
                concatStr+=envConfig[key]
            cmd="verrou_plot_stat --rep=%s --seed=42 --rounding-list=%s --no-plot --mca-estimator %s %s "%( repNum, roundingStr, pathNumBin+"/"+runNum, pathNumBin+"/"+extractNum)
            print(envStr, cmd)
            envFile=os.path.join(workDirectory,"buildRep-"+name, "install","env.sh")
            lineTab=runCmdToLines(". %s ; %s %s "%(envFile,envStr,cmd))
            resName[concatStr]=parsePlotStat(lineTab)
        res[name]=resName
    return res


def checkCoherence(stat):
    for code in ["Seqdouble", "Seqfloat", "Recdouble", "Recfloat"]:
        for rounding in ["random","average","all"]:
            try:
                resTab =[stat[conf][code][rounding]["bit"] for conf in buildConfList]
            except:
                print("debug error stat")
                for conf in buildConfList:
                    print("stat["+conf+"]",stat[conf])

            if len(set(resTab))>1:
                bitTab=[float(x) for x in set(resTab)]
                maxError=max([abs(x-bitTab[0])/ bitTab[0] for x in bitTab ])
                if maxError > 0.01:
                    print(code + " "+rounding+ " " +str(resTab))
                    print("maxError:",maxError)
                    return False
        resTab =[stat[conf][code]["nearest"] for conf in buildConfList]
        if len(set(resTab))>1:
            print("resTab Nearest ", resTab)
            return False
    return True

def feedTab(stat, rndList=["random","average","sr_monotonic","sr_smonotonic" ] ,detTab=["_det","_comdet"], extraRounding=[], ref=None, precisionVar="bit", buildConfList=buildConfList):

    codeTab=["Seqfloat","Seqdouble", "Recfloat","Recdouble"]
    codeTabName=[x.replace("float","<float>").replace("double","<double>")for x in codeTab]
    tab.begin()
    tab.lineMultiple([(1,""), (2,"Seq"),(2,"Rec")])
    tab.endLine()
    tab.line(["","float","double", "float","double"])
    tab.endLine()
    tab.lineSep()
    if precisionVar=="bit":
        tab.line(["error(nearest)"]+ [ "%.2f"%( -math.log2(abs(float(stat[refName][code]["nearest"])-float(ref)) / float(ref)))  for code in codeTab ])
        tab.endLine()
        roundingTab=[("all", "all", refName),"SEPARATOR"]
    else:
        roundingTab=[]
    for rd in rndList:
        if rd in ["sr_monotonic", "sr_smonotonic"]:
            break
        if not rd in  ["sr_monotonic", "sr_smonotonic"]:
            roundingTab+=[(rd, rd,refName)]
        if rd=="average":
            for gen in buildConfListXoshiro:
                roundingTab+=[(rd+ "("+gen+")" ,rd ,gen )]
        if rd=="random":
            for gen in buildConfListXoshiro:
                roundingTab+=[(rd+ "("+gen+")" ,rd ,gen )]


        for gen in buildConfList:
            if gen=="current":
                continue
            if rd in ["random","average"]:
                for detType in detTab:
                    roundingTab+=[(rd+detType+"("+gen+")",rd+detType,gen)]
            else:
                roundingTab+=[(rd+"("+gen+")",rd,gen)]

        roundingTab+=["SEPARATOR"]
    needNewSep=False
    for gen in buildConfList:
        if gen=="current":
            continue
        for rd in rndList:
            if not rd in ["sr_monotonic", "sr_smonotonic"]:
                continue
            roundingTab+=[(rd+"("+gen+")",rd,gen)]
            needNewSep=True
    if needNewSep:
        roundingTab+=["SEPARATOR"]
    for confLine in roundingTab[0:-1]:
        if confLine=="SEPARATOR":
            tab.lineSep()
            continue

        head= [confLine[0]]
        content=[stat[confLine[2]][code][confLine[1]][precisionVar]  for code in codeTab ]
        tab.line(head+content)
        tab.endLine()

    tab.end()



def plotNumConfig():
    histRep="histPng"
    if not os.path.exists(histRep):
        os.mkdir(histRep)

    for name in buildConfList:
        repNum=os.join.path(workDirectory,"buildRep-%s"%(name), "num")
        if not os.path.exists(repNum):
            os.mkdir(repNum)

        roundingTab=detRounding+ roundingListNum
        roundingStr=",".join(roundingTab)
        for envConfig in numEnvConfigTab:
            envStr=""
            pngStr=os.path.join(histRep,name+"-")
            for key in envConfig:
                envStr+= " "+key+"="+envConfig[key]
                pngStr+=envConfig[key]
            cmd="verrou_plot_stat --rep=%s --num-threads=5 --seed=42 --relative=104857.6 --rounding-list=%s --png=%s.png %s %s "%( repNum, roundingStr, pngStr, pathNumBin+"/"+runNum, pathNumBin+"/"+extractNum)
            print(envStr, cmd)
            if name!="local":
                envFile=os.path.join(workDirectory,"buildRep-"+name, "install","env.sh")
                runCmd(". %s ; %s %s "%(envFile,envStr,cmd))
            else:
                runCmd("%s %s "%(envStr,cmd))


if __name__=="__main__":

    runCmd("make -C ../unitTest/checkStatRounding/ checkStatRounding")
#    plotNumConfig()
    statRes=extractStat()
    if checkCoherence(statRes):
        print("checkCoherence OK")
    else:
        print("checkCoherence FAILURE")

    tab=tabularLatex("lcccc", output="tabDet.tex")
    feedTab(statRes,rndList=["random","average"],detTab=["_det"], ref=2**20*0.1)

    tab=tabularLatex("lcccc", output="tabComDet.tex")
    feedTab(statRes,rndList=["random","average"],detTab=["_comdet"], ref=2**20*0.1)

    tab=tabularLatex("lcccc", output="tabScomDet.tex")
    feedTab(statRes,rndList=["random","average"],detTab=["_scomdet"], ref=2**20*0.1)

    tab=tabularLatex("lcccc", output="tabMono.tex")
    feedTab(statRes,rndList=["average","sr_monotonic","sr_smonotonic"],detTab=["_scomdet"], ref=2**20*0.1)

    tab=tabularLatex("lcccc", output="tabMaxDiff.tex")
    feedTab(statRes,rndList=["random","average","sr_monotonic", "sr_smonotonic"],detTab=["_det","_comdet","_scomdet"], ref=2**20*0.1)

    tab=tabularLatex("lcccc", output="tabMCA.tex")
    feedTab(statRes,rndList=["random","average","sr_monotonic", "sr_smonotonic"],detTab=["_det","_comdet","_scomdet"], ref=2**20*0.1, precisionVar="mca_bit")

    if what=="cmpHash":
        name="mersenne_twister"
        repNum=os.path.join(workDirectory,"buildRep-%s/num"%(name))
        cmd="ALGO=Rec ALGO_TYPE=float verrou_plot_stat --rep="+repNum+" --seed=42 --relative=104857.6 --rounding-list=random,average,nearest,upward,downward,random_det,average_det --png=Recfloat"+name+"Det.png ../unitTest/checkStatRounding/run.sh ../unitTest/checkStatRounding/extract.py"
        print(cmd)
        runCmd(cmd)

        cmd="ALGO=Seq ALGO_TYPE=float verrou_plot_stat --nb-bin=200 --rep="+ repNum +" --seed=42 --relative=104857.6 --rounding-list=average,random,random_det,average_det  --png=SeqFloat"+name+"DetZoom.png ../unitTest/checkStatRounding/run.sh ../unitTest/checkStatRounding/extract.py"
        print(cmd)
        runCmd(cmd)
        cmd="ALGO=Seq ALGO_TYPE=float verrou_plot_stat --rep="+repNum+" --seed=42 --relative=104857.6 --rounding-list=average,random,random_det,average_det,nearest,downward,upward  --png=SeqFloat"+name+"Det.png ../unitTest/checkStatRounding/run.sh ../unitTest/checkStatRounding/extract.py"
        print(cmd)
        runCmd(cmd)
