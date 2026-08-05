

import sys
from pathlib import Path
from coverTools import backCovReader, coverageReader, traceName


class statusReader:
    """Class to provide the status of a run"""
    # should maybe be a function instead of a class
    def __init__(self,pid, rep, runEval=None,runCmp=None, repRef=None):
        self.pid=pid
        self.rep=rep

        self.remoteRep=self.searchRemoteRep(self.rep)
        if self.remoteRep==None:
            self.remoteRep=rep
        self.isSuccess=None
        if runCmp!=None:
            self.runCmpScript(runCmp, repRef)
            return
        if runEval!=None:
            self.runEvalScript(runEval)
            return
        self.read()

    def searchRemoteRep(self,rep, coverName="cover"):
        res=rep
        if (res / "dd.return.value").is_file():
            return res
        if rep.name==coverName:
            res=rep.parent
            if (res / "dd.return.value").is_file():
                return res
        return None


    def runCmpScript(self,runCmp,repref):
        subProcessRun=runCmdAsync([runCmp, repref,self.rep], self.rep / ("cmpCmd%i"%(self.pid)))
        res=getResult(subProcessRun)
        if res==0:
            self.isSuccess=True
        else:
            self.isSuccess=False

    def runEvalScript(self,runEval):
        subProcessRun=runCmdAsync([runEval,self.rep], self.rep / ("evalCmd%i"%(self.pid)))
        res=getResult(subProcessRun)
        if res==0:
            self.isSuccess=True
        else:
            self.isSuccess=False

    def read(self, level=0):
        pathName=self.remoteRep /"dd.return.value"
        if pathName.is_file():
            try:
                value=int(open(pathName).readline().strip())
                if value==0:
                    self.isSuccess=True
                else:
                    self.isSuccess=False
            except:
                print("Error while  reading "+pathName )
                self.isSuccess=None
        else:
            if self.rep.name=="ref":
                print("Consider ref as a success")
                self.isSuccess=True
            else:
                self.isSuccess=None

    def getStatus(self):
        return self.isSuccess


class genMerge:

    def __init__(self, pidRef, pathRef,statusRef, trace_kind):

        if trace_kind=="back_cover":
            self.root=backCovReader(pidRef, pathRef,statusRef, trace_kind, mergeRoot=True)
        elif trace_kind=="bb_cover":
            self.root=coverageReader(pidRef, pathRef,statusRef, trace_kind, mergeRoot=True)
        else:
            pass

    def current(self,  pid, path, status, trace_kind):
        if trace_kind=="back_cover":
            return backCovReader( pid, path, status, trace_kind, mergeRoot=False)
        elif trace_kind=="bb_cover":
            return coverageReader(pid, path,status, trace_kind, mergeRoot=False)

    def addMerge(self, current):
        self.root.addMerge(current)

    def endMerge(self):
        self.root.endMerge()

    def writeCSV(self, pathStr, header, outputTypeTab):
        self.root.writeCSV(pathStr, header, outputTypeTab=outputTypeTab)

    

class cmpToolsCov:
    """Class to write partial cover of several executions :
    with writePartialCover the object write a partial cover for each execution
    with mergedCov the object write one merged partial cover with correlation information"""

    def __init__(self, tabPidRep, runCmp=None, runEval=None, trace_kind="bb_cover"):
        self.tabPidRep=tabPidRep
        self.runCmp=runCmp
        self.runEval=runEval
        self.trace_kind=trace_kind

    def findRefForMerge(self):
        if self.runCmp!=None:
            self.refIndex=self.findRef(patternList=["ref","Ref","nearest","Nearest"])
            return
        if self.runEval!=None:
            self.refIndex=self.findRefDD(pattern="ref", optionalPattern="Nearest")
            return
        self.refIndex=self.findRefDD(pattern="ref", optionalPattern="dd.line/ref")

    def findRefForMergePost(self):
        self.refIndex=self.findRefDD(pattern="NoPerturbation-trace")

    def writePartialCover(self,filenamePrefix="", pidMap=None):
        """Write partial cover for each execution (defined by a tab of pid)"""
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            cov=coverageReader(pid,Path(rep), None, self.trace_kind)
            cov.writePartialCover(filenamePrefix=filenamePrefix, pidMap=pidMap)


    def writePartialBack(self,filenamePrefix="", pidMap=None):
        """Write partial cover for each execution (defined by a tab of pid)"""
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            covBack=backCovReader(pid,Path(rep), None, self.trace_kind)
            covBack.writePartialBackCover(filenamePrefix=filenamePrefix, pidMap=pidMap)

    # def writeStatus(self):
    #     for i in range(len(self.tabPidRep)):
    #         pid,rep=self.tabPidRep[i]
    #         status=statusReader(pid,rep)
    #         success=status.getStatus()
    #         print( rep+":" + str(success))
    def getStatus(self,pid,rep):
        if self.runCmp!=None:
            status=statusReader(pid,rep, self.runCmp, self.tabPidRep[self.refIndex][1])
            return status.getStatus()
        if self.runEval!=None:
            status=statusReader(pid,rep, runEval=self.runEval)
            return status.getStatus()
        status=statusReader(pid,rep)
        return status.getStatus()

    def countStatus(self):
        """ Count the number of Success/Fail"""
        nbSuccess=0
        nbFail=0
        listPidRepoIgnore=[]
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            success=self.getStatus(pid,rep)
            if success==None:
                listPidRepoIgnore+=[(pid,rep)]
            else:
                if success:
                    nbSuccess+=1
                else:
                    nbFail+=1
        for (pid,rep) in listPidRepoIgnore:
            print("directory ignored : "+rep)
            self.tabPidRep.remove((pid,rep))

        return (nbSuccess, nbFail)


    def findRefDD(self, pattern="ref", optionalPattern=None):
        "return the index of the reference (required for correlation)"
        if optionalPattern!=None:
            for index in range(len(self.tabPidRep)):
                (pid,rep)=self.tabPidRep[index]
                success=self.getStatus(pid,rep)
                if str(rep).endswith(pattern) and optionalPattern in str(rep) and success:
                    return index
            print('Optional failed')
        for index in range(len(self.tabPidRep)):
            (pid,rep)=self.tabPidRep[index]
            success=self.getStatus(pid,rep)
            if str(rep).endswith(pattern) and success:
                return index
        print("Warning : pattern not found" )
        print("Switch to first Success reference selection")
        for index in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[index]
            success=self.getStatus(pid,rep)
            if success:
                return index
        print("Error fail only : cmpToolsCov is ineffective" )
        sys.exit(42)

    def findRef(self, patternList):
        "return the index of the reference (required for correlation)"

        for index in range(len(self.tabPidRep)):
            rep=self.tabPidRep[index][1]
            for pattern in patternList:
                if pattern in rep:
                    return index
        return 0


    def writeMergedBB(self,estimatorTab):
        """Write merged BB with correlation indice  between coverage difference and success/failure status"""
        (nbSuccess, nbFail)=self.countStatus()
        print("NbSuccess: %d \t nbFail %d"%(nbSuccess,nbFail))
        if nbFail==0 or nbSuccess==0:
            print("mergeCov need Success/Fail partition")
            sys.exit()

        pidRef,repRef=self.tabPidRep[self.refIndex]
        statusRef=self.getStatus(pidRef,repRef)


        bbMerged=coverageReader(pidRef, Path(repRef),statusRef,self.trace_kind, mergeRoot=True)
        #Loop with addMerge to reduce memory peak

        printIndex=[int(float(p) * len(self.tabPidRep) /100.)  for p in (list(range(0,100,10))+[1,5])]
        printIndex +=[1,  len(self.tabPidRep)-1]

        for i in range(len(self.tabPidRep)):
            if i==self.refIndex:
                continue
            pid,rep=self.tabPidRep[i]
            currentBBCov=coverageReader(pid,Path(rep),self.getStatus(pid,rep),self.trace_kind, mergeRoot=False)
            bbMerged.addMerge(currentBBCov)
            if i in printIndex:
                pourcent=float(i+1)/ float(len(self.tabPidRep)-1)
                if i >=self.refIndex:
                    pourcent=float(i)/ float(len(self.tabPidRep)-1)
                print( "%.1f"%(pourcent*100)    +"% of coverage data merged")

        bbMerged.endMerge()
        bbMerged.writePartialCover(outputDir=Path("."), outputTypeTab=estimatorTab)


    def writeMergedBack(self,estimatorTab):
        """Write merged Back with correlation indice  between coverage difference and success/failure status"""

        pidRef,repRef=self.tabPidRep[self.refIndex]
        statusRef=self.getStatus(pidRef,repRef)

        backMerged=backCovReader(pidRef, Path(repRef),statusRef,self.trace_kind, mergeRoot=True)
        #Loop with addMerge to reduce memory peak

        printIndex=[int(float(p) * len(self.tabPidRep) /100.)  for p in (list(range(0,100,10))+[1,5])]
        printIndex +=[1,  len(self.tabPidRep)-1]

        for i in range(len(self.tabPidRep)):
            if i==self.refIndex:
                continue
            pid,rep=self.tabPidRep[i]
            currentBackCov=backCovReader(pid,Path(rep),self.getStatus(pid,rep),self.trace_kind, mergeRoot=False)
            backMerged.addMerge(currentBackCov)
            if i in printIndex:
                pourcent=float(i+1)/ float(len(self.tabPidRep)-1)
                if i >=self.refIndex:
                    pourcent=float(i)/ float(len(self.tabPidRep)-1)
                print( "%.1f"%(pourcent*100)    +"% of coverage data merged")

        backMerged.endMerge()
        backMerged.writePartialBackCover(outputDir=Path("."), outputTypeTab=estimatorTab)


