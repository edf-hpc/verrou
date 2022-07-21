#!/usr/bin/python3

import os
import re
import sys
import subprocess
import math

detRounding=["random_det","average_det", "random_comdet","average_comdet"]
roundingListNum=["random", "average", "nearest", "upward", "downward"]
buildConfList=[ "dietzfelbinger","multiply_shift","double_tabulation","mersenne_twister"]


pathNumBin="../unitTest/checkStatRounding"
runNum="run.sh"
extractNum="extract.py"
numEnvConfigTab=[{"ALGO":algo, "ALGO_TYPE":realtype} for realtype in ["double", "float"] for algo in ["Seq", "Rec"]]


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
    for name in buildConfList:
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
            cmd="verrou_plot_stat --rep=%s --rounding-list=%s --no-plot %s %s "%( repNum, roundingStr, pathNumBin+"/"+runNum, pathNumBin+"/"+extractNum)
            print(envStr, cmd)
            lineTab=runCmdToLines(". ./buildRep-%s/install/env.sh ; %s %s "%(name,envStr,cmd))
            resName[concatStr]=parsePlotStat(lineTab)
        res[name]=resName
    return res


def checkCoherence(stat):
    for code in ["Seqdouble", "Seqfloat", "Recdouble", "Recfloat"]:
        for rounding in ["random","average","all"]:
            resTab =[stat[conf][code][rounding]["bit"] for conf in buildConfList]
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

class tabular:
    def __init__(self):
        self.currentStr=""
    def begin(self):
        pass
    def end(self):
        pass
    def lineSep(self):
        print("")
    def endLine(self):
        print(self.currentStr)
        self.currentStr=""
    def line(self,tab):
        self.currentStr+=("\t".join(tab))

    def lineMultiple(self, tab):
        lineTab=[]
        for (nb, value) in tab:
            for i in range(nb):
                lineTab+=[value]
        self.currentStr+= ("\t".join(lineTab))

class tabularLatex:
    def __init__(self,keyStr="c", output=None):
        self.currentStr=""
        self.keyStr=keyStr
        self.output=output
    def begin(self):
        self.currentStr+="\\begin{tabular}{%s}\\toprule\n"%self.keyStr

    def end(self):
        self.currentStr+="\\bottomrule\n"
        self.currentStr+="\end{tabular}\n"
        if self.output==None:
            print(self.currentStr)
        else:
            handler=open(self.output,"w")
            handler.write(self.currentStr)

    def lineSep(self):
        self.currentStr+="\\midrule\n"
    def endLine(self):
        self.currentStr+="\\\\\n"
    def line(self,tab):
        lineStr=("\t&\t".join(tab))
        lineStr=lineStr.replace("_","\_")
        self.currentStr+=lineStr

    def lineMultiple(self, tab):
        lineStr=""
        lineTab=[]
        for (nb, value) in tab:
            if nb>1:
                lineTab+=["\multicolumn{%s}{c}{%s}"%(str(nb), value.replace("_","\_")) ]
            if nb==1:
                lineTab+=[value.replace("_","\_")]
        lineStr+= ("\t&\t".join(lineTab))
        self.currentStr+=lineStr

def feedTab(stat, detTab=["_det","_comdet"], ref=None):
    codeTab=["Seqfloat","Seqdouble", "Recfloat","Recdouble"]
    codeTabName=[x.replace("float","<float>").replace("double","<double>")for x in codeTab]
    tab.begin()
    tab.lineMultiple([(1,""), (2,"Seq"),(2,"Rec")])
    tab.endLine()
    tab.line(["","float","double", "float","double"])
    tab.endLine()
    tab.lineSep()

    tab.line(["error(nearest)"]+ [ "%.2f"%( -math.log2(abs(float(stat["dietzfelbinger"][code]["nearest"])-float(ref)) / float(ref)))  for code in codeTab ])
    tab.endLine()
    roundingTab=[("all", "all", "dietzfelbinger"),"SEPARATOR"]
    for rd in ["random","average"]:
        roundingTab+=[(rd, rd,"dietzfelbinger")]
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

if __name__=="__main__":
    statRes=extractStat()

    if checkCoherence(statRes):
        print("checkCoherence OK")
    else:
        print("checkCoherence FAILURE")

#    tab=tabular()
    tab=tabularLatex("lcccc", output="tabDet.tex")
    feedTab(statRes,detTab=["_det"], ref=2**20*0.1)


    tab=tabularLatex("lcccc", output="tabComDet.tex")
    feedTab(statRes,detTab=["_comdet"], ref=2**20*0.1)
