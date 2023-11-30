#!/usr/bin/python3

import os
import sys

listOfFunction=["cos","sinf","erf", "atan2", "jn"]

def checkDD(rep, symOrLine, listOfInstableFunction):
    listOfStableFunction=[x for x in listOfFunction if not (x in listOfInstableFunction)]

    if len(listOfStableFunction)+ len(listOfInstableFunction)!= len(listOfFunction):
        print("incoherent check function")
        return False
    
    
    ddminCmpPath=os.path.join(rep, "rddmin-cmp", "dd."+symOrLine+".include")
    if os.path.exists(ddminCmpPath):
        lines=open(ddminCmpPath).readlines()
        if len(lines)!=len(listOfStableFunction):
            print("KO incoherent ddmin")
            return False
        
        resDDMinCmp=True
        for stableFunction in listOfStableFunction:
            res=any([stableFunction in line for line in lines ])
            print('OK: ', stableFunction)  
            resDDMinCmp= res and resDDMinCmp
        if resDDMinCmp==False:
            return False
    else:
        print(ddminCmpPath,"missing")
        return False

    for i in range(len(listOfInstableFunction)):
        ddminCmpPath=os.path.join(rep, "ddmin"+str(i), "dd."+symOrLine+".include")
        if os.path.exists(ddminCmpPath):
            lines=open(ddminCmpPath).readlines()
            if len(lines)!=1:
                print(ddminCmpPath, "bad size")
                return False
            res=any([(unstable in lines[0]) for unstable in listOfInstableFunction  ])
            if not res:
                print("problem with ",ddminCmpPath, "and", lines[0])
                return False
        else:
            print(ddminCmpPath,"missing")
            return False

            
        
    
    return True

              
    

if __name__=="__main__":
    rep=sys.argv[1]
    unstableFunction=sys.argv[2:]

    symOrLine=None
    if rep.startswith("dd.line"):
        symOrLine="line"
    if rep.startswith("dd.sym"):
        symOrLine="sym"

    if symOrLine==None:
        print("invalidCall")
        sys.exit(42)
        
    isOK=checkDD(rep, symOrLine, unstableFunction)

    if isOK:
        print("OK:", rep)
        sys.exit(0)
    else:
        print("KO:", rep)
        sys.exit(42)
