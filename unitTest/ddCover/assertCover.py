#!/usr/bin/python3

import sys
import os.path


def listOfFileToCheck(rep, ddminNumber, sampleNumberDD, sampleNumberPost, dumpCoverNumber,  fileTab, rounding="default"):
    res=[]    
    symLinkTab=["NoPerturbation", "FullPerturbation","rddmin-cmp"]+ ["ddmin"+str(i) for i in range(ddminNumber)]


    for symLink in symLinkTab:
        pathWithoutTrace=os.path.join(rep, symLink)
        res+=[os.path.join(pathWithoutTrace, "dd.run"+str(i), fileName)
              for i in range(sampleNumberDD)
              for fileName in fileTab]
        localRounding=rounding

        pathWithTrace=os.path.join(rep, symLink+"-trace", rounding)
        if "NoPerturbation" == symLink:
            pathWithTrace=os.path.join(rep, symLink+"-trace", "default")
        
        fileTabCov=fileTab+["covBBLog"]+ [os.path.join("cover", "cover0000"+str(dumpIndex)+"-seqCount0") for dumpIndex in range(dumpCoverNumber+1) ]
        def sampleNumberPostLambda(path):
            if "NoPerturbation-trace" in path:
                return 1
            else:
                return sampleNumberPost                
        
        res+=[os.path.join(pathWithTrace, "dd.run"+str(i), fileName)
              for fileName in fileTabCov
              for i in range(sampleNumberPostLambda(pathWithTrace))
              ]
                
    return res
        
def checkExit(tabOfFile):
    for filePath in tabOfFile:
        if not os.path.exists(filePath):
            print("file missing:", filePath)
            return False
    return True
    



fileTab=["dd.compare.err", "dd.compare.out", "dd.return.value", "dd.run.err", "dd.run.out", "res.dat"]


if __name__=="__main__":
    rep=sys.argv[1]
    ddminNumber=int(sys.argv[2])
    sampleNumberDD=int(sys.argv[3])
    sampleNumberPost=int(sys.argv[4])
    dumpCoverNumber=int(sys.argv[5])
    rounding=sys.argv[6]
    
    listOfFile=listOfFileToCheck(rep,ddminNumber,sampleNumberDD,sampleNumberPost, dumpCoverNumber, fileTab, rounding)
    check=checkExit(listOfFile)
    if not check:
        sys.exit(42)
    sys.exit(0)
