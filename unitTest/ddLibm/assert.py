#!/usr/bin/python3
from pathlib import Path
import sys

listOfFunction=["cos","sinf","erf", "atan2", "jn"]

def checkDD(rep, symOrLine, listOfInstableFunction):
    listOfStableFunction=[x for x in listOfFunction if not (x in listOfInstableFunction)]

    if len(listOfStableFunction)+ len(listOfInstableFunction)!= len(listOfFunction):
        print("incoherent check function")
        return False
    
    
    ddminCmpPath=rep / "rddmin-cmp" / ("dd."+symOrLine+".include")
    if ddminCmpPath.is_file():
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
        ddminCmpPath=rep / ("ddmin"+str(i)) / ("dd."+symOrLine+".include")
        if ddminCmpPath.is_file():
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
    rep=Path(sys.argv[1])
    unstableFunction=sys.argv[2:]

    symOrLine=None
    if (rep.name).startswith("dd.line"):
        symOrLine="line"
    if (rep.name).startswith("dd.sym"):
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
