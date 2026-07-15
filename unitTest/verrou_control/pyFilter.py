#!/usr/bin/env python3

import sys
import re

def filterPID(tabLines):
    pid=None
    #search first PID
    for line in tabLines:
        if line.startswith("PID "):
            pid=line.replace("PID ","").split(":")[0]
            break
    if pid==None:
        print("Unknown PID")
        sys.exit(42)
    res=[line.replace(pid,"FILTERED_PID") for line in tabLines]    
    return res
        

def findNonNullCounterNumber(tabLines, keyTab=["div","cmp"]):
    res=[]
    for line  in tabLines:
        for key in keyTab:
            begin="==FILTERED_PID==  "+key
            if line.startswith(begin):
                restLine=line.replace(begin," ").strip()
                m=re.match(r"(\d*)\s*(\d*)\s*\(\s*(\d+)%\)", restLine)
                res+=[m.group(1),m.group(2)]
                if m.group(3)!="100":
                    res+=[m.group(3)]
    res=set(res)
    res=[x for x in res if x!="0"]
    res.sort(key=lambda x: len(x), reverse=True)
    return res
                          

                          
def replaceMultipleSpace(line):
    if "  " in line:
        return replaceMultipleSpace(line.replace("  "," "))
    return line

    
def filterCounter(tabLines):
    tabValue=findNonNullCounterNumber(tabLines, keyTab=["div","cmp"])
    res=[]
    for line in tabLines:
        if any([value in line for value in tabValue ]):
            for value in tabValue:
                line=line.replace(value, "NON_NULL_VALUE")
                line=line.replace("llo","scal") #homogene between x86 and arm
                #we chose arm reference to avoid to parse differently div and cmp
            line=replaceMultipleSpace(line)
        res+=[line]    
    return res
    
def filterAddress(tabLines):
    #    0-  0x4001203: main (infiniteLoop.cxx:21)
    filterActive=True
    res=[]
    for line in tabLines:
        if filterActive:
            m=re.match(r"\t\d+-\s*0x([0-9ABCDEF]*): .*\n",line)
            if m!=None:
                res+=[line.replace(m.group(1), "FFFFFF")]
            else:
                res+=[line]
                
            if "verrou_control --verbose=off" in line:
                filterActive=False
        else:
            res+=[line]
    return res

def filterPath(tabLines):
    res=[]
    for line in tabLines:
        m=re.match(r"(.+)verrou_control -.*\n",line)
        if m!=None:
            line=line.replace(m.group(1), "$PATH/")
        res+=[line]
    return res
    


def output(tabLines):
    for line in tabLines:
        print(line, end="")

if __name__=="__main__":
    tabLines=open(sys.argv[1]).readlines()
    tabLines=filterPID(tabLines)
    tabLines=filterPath(tabLines)
    tabLines=filterAddress(tabLines)
    tabLines=filterCounter(tabLines)
    
    output(tabLines)
    
    
    
    
