#!/usr/bin/python3

import os
import re
import sys
import subprocess
import math
from tabular import *

detRounding=["random_det","average_det", "random_comdet","average_comdet"]
roundingListNum=["random", "average", "nearest", "upward", "downward"]
buildConfList=[ "master","dietzfelbinger","multiply_shift","multiply_shift_fix","double_tabulation","mersenne_twister"]
#buildConfList=[ "master","multiply_shift","multiply_shift_fix"]
#buildConfList=["double_tabulation"]#,"mersenne_twister"]
buildConfListXoshiro=[]#"xoshiro","xoshiro-2","xoshiro-8"]

pathNumBin="../unitTest/checkStatRounding"
runNum="run.sh"
extractNum="extract.py"
numEnvConfigTab=[{"ALGO":algo, "ALGO_TYPE":realtype} for realtype in ["double", "float"] for algo in ["Seq", "Rec"]]


def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def runCmdToLines(cmd):
    process=subprocess.run(cmd, shell=True,capture_output=True, text=True)
    return process.stdout.split("\n")

def parsePlotStat(lineTab):
    toParse=False
    res={}
    for line in lineTab:
        if toParse:
            spline=line.strip().split("\t")
            rounding=spline[0][0:-1]
            absEst=spline[1]
            relEst=spline[2]
            bit=spline[3].replace("bit","")
            res[rounding]={"absEst":absEst, "relEst": relEst , "bit":bit }
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
        repNum="buildRep-%s/num"%(name)
        roundingTab=roundingListNum + detRounding
        roundingStr=",".join(roundingTab)
        for envConfig in numEnvConfigTab:
            envStr=""
            concatStr=""
            for key in envConfig:
                envStr+= " "+key+"="+envConfig[key]
                concatStr+=envConfig[key]
            cmd="verrou_plot_stat --rep=%s --seed=42 --rounding-list=%s --no-plot %s %s "%( repNum, roundingStr, pathNumBin+"/"+runNum, pathNumBin+"/"+extractNum)
            print(envStr, cmd)
            lineTab=runCmdToLines(". ./buildRep-%s/install/env.sh ; %s %s "%(name,envStr,cmd))
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
            print("resTab Neaerest", resTab)
            return False
    return True

def feedTab(stat, detTab=["_det","_comdet"], ref=None):
    refName="master"
    codeTab=["Seqfloat","Seqdouble", "Recfloat","Recdouble"]
    codeTabName=[x.replace("float","<float>").replace("double","<double>")for x in codeTab]
    tab.begin()
    tab.lineMultiple([(1,""), (2,"Seq"),(2,"Rec")])
    tab.endLine()
    tab.line(["","float","double", "float","double"])
    tab.endLine()
    tab.lineSep()

    tab.line(["error(nearest)"]+ [ "%.2f"%( -math.log2(abs(float(stat[refName][code]["nearest"])-float(ref)) / float(ref)))  for code in codeTab ])
    tab.endLine()
    roundingTab=[("all", "all", "master"),"SEPARATOR"]
    for rd in ["random","average"]:
        roundingTab+=[(rd, rd,refName)]
        if rd=="average":
            for gen in buildConfListXoshiro:
                roundingTab+=[(rd+ "("+gen+")" ,rd ,gen )]
        if rd=="random":
            for gen in buildConfListXoshiro:
                roundingTab+=[(rd+ "("+gen+")" ,rd ,gen )]


        for gen in buildConfList:
            for detType in detTab:
                roundingTab+=[(rd+detType+"("+gen+")",rd+detType,gen)]
        roundingTab+=["SEPARATOR"]
    roundingTab=roundingTab[0:-1]

    for confLine in roundingTab:
        if confLine=="SEPARATOR":
            tab.lineSep()
            continue

        head= [confLine[0]]
        content=[stat[confLine[2]][code][confLine[1]]['bit']  for code in codeTab ]
        tab.line(head+content)
        tab.endLine()

    tab.end()



def plotNumConfig():
    histRep="histPng"
    if not os.path.exists(histRep):
        os.mkdir(histRep)

    for name in buildConfList:
        repNum="buildRep-%s/num"%(name)
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
                runCmd(". ./buildRep-%s/install/env.sh ; %s %s "%(name,envStr,cmd))
            else:
                runCmd("%s %s "%(envStr,cmd))


if __name__=="__main__":

    plotNumConfig()

    statRes=extractStat()
    if checkCoherence(statRes):
        print("checkCoherence OK")
    else:
        print("checkCoherence FAILURE")

    tab=tabularLatex("lcccc", output="tabDet.tex")
    feedTab(statRes,detTab=["_det"], ref=2**20*0.1)


    tab=tabularLatex("lcccc", output="tabComDet.tex")
    feedTab(statRes,detTab=["_comdet"], ref=2**20*0.1)


    cmd="ALGO=Rec ALGO_TYPE=float verrou_plot_stat --rep=buildRep-mersenne_twister/num --seed=42 --relative=104857.6 --rounding-list=random,average,nearest,upward,downward,random_det,average_det --png=Recfloatmersenne_twisterDet.png ../unitTest/checkStatRounding/run.sh ../unitTest/checkStatRounding/extract.py"
    print(cmd)
    runCmd(cmd)


    cmd="ALGO=Seq ALGO_TYPE=float verrou_plot_stat --nb-bin=200 --rep=buildRep-mersenne_twister/num  --seed=42 --relative=104857.6 --rounding-list=average,random,random_det,average_det  --png=SeqFloatmersenne_twisterDetZoom.png ../unitTest/checkStatRounding/run.sh ../unitTest/checkStatRounding/extract.py"
    print(cmd)
    runCmd(cmd)
    cmd="ALGO=Seq ALGO_TYPE=float verrou_plot_stat --rep=buildRep-mersenne_twister/num  --seed=42 --relative=104857.6 --rounding-list=average,random,random_det,average_det,nearest,downward,upward  --png=SeqFloatmersenne_twisterDet.png ../unitTest/checkStatRounding/run.sh ../unitTest/checkStatRounding/extract.py"
    print(cmd)
    runCmd(cmd)
