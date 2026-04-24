
import re
import sys
from operator import itemgetter, attrgetter
import gzip
import os
import copy

import subprocess

from pathlib import Path

def runCmdAsync(cmd, fname, envvars=None):
    """Run CMD, adding ENVVARS to the current environment, and redirecting standard
    and error outputs to FNAME.out and FNAME.err respectively.

    Returns CMD's exit code."""
    if envvars is None:
        envvars = {}

    with open("%s.out"%fname, "w") as fout:
        with open("%s.err"%fname, "w") as ferr:
            env = copy.deepcopy(os.environ)
            for var in envvars:
                env[var] = envvars[var]
            return subprocess.Popen(cmd, env=env, stdout=fout, stderr=ferr)

def getResult(subProcess):
    subProcess.wait()
    return subProcess.returncode



class openGz:
    """ Class to read/write  gzip file or ascii file """
    def __init__(self,name, mode="r", compress=None):
        self.name=name
        if (name.with_suffix(".gz")).is_file() and compress==None:
            self.name=name.with_suffix(".gz")

        if (self.name.suffix==".gz" and compress==None) or compress==True:
            self.compress=True
            self.handler=gzip.open(self.name, mode)
        else:
            self.compress=False
            self.handler=open(self.name, mode)

    def readline(self):
        if self.compress:
            return self.handler.readline().decode("ascii")
        else:
            return self.handler.readline()
    def readlines(self):
        if self.compress:
            return [line.decode("ascii") for line in self.handler.readlines()]
        else:
            return self.handler.readlines()


    def write(self, line):
        self.handler.write(line)


class bbInfoReader:
    """ Class to read trace_bb_info_log-PID(.gz) file and to provide a string describing
    a basic bloc defined by an index (called addr) from the MARK (debug symbol)

    After init the usual call are :
      .compressMarksWithoutSym(addr)
      .getStrToPrint(addr)

    """
    def __init__(self,fileName, trace_kind):
        self.trace_kind=trace_kind
        self.read(fileName)

    def read(self,fileName):
        self.data={}
        regularExp=None
        if self.trace_kind=="bb":
            regularExp=re.compile(r"([0-9]+) : (.*) : (\S*) : ([0-9]+) : ([0,1]) : ([0,1])")
        else:
            #[67133584] unamed_filename_verrou	0	F	!
            regularExp=re.compile(r"\[([0-9]+)\]() (\S*)\t([0-9]+)\t([F,I])\t([?,!])")
            #() is there to avoid sym shift in m.groups()
        fileHandler=openGz(fileName)

        line=fileHandler.readline()
        while not line in [None, ''] :
            m=(regularExp.match(line.strip()))
            if m==None :
                print("error read fileName line:",[line])
                sys.exit()
            addr, sym, sourceFile, lineNum, containFloat, containFloatCmp= m.groups()
            containFloat=(containFloat in ["1","F"])
            containFloatCmp=(containFloatCmp in ["1","?"])
            if addr in self.data:
                if not (sym,sourceFile,lineNum,containFloat, containFloatCmp) in self.data[addr]:
                    self.data[addr]+=[(sym,sourceFile,lineNum,containFloat,containFloatCmp)]
            else:
                self.data[addr]=[(sym,sourceFile,lineNum,containFloat,containFloatCmp)]
            line=fileHandler.readline()


    def compressMarks(self, lineMarkInfoTab):
        lineToTreat=lineMarkInfoTab
        res=""
        while len(lineToTreat)!=0:
            symName=lineToTreat[0][0]
            select=[(x[1],x[2]) for x in lineToTreat if x[0]==symName ]
            lineToTreat=[x for x in lineToTreat if x[0]!=symName ]
            res+=" "+symName +"["+self.compressFileNames(select)+"] |"
        return res[0:-1]

    def containFloatMod(self,addr):
        select=[ x[3] for x in self.data[addr]]
        return (True in select)

    def containFloatCmp(self,addr):
        select=[ x[4] for x in self.data[addr]]
        return (True in select)

    def compressMarksWithoutSym(self, addr):
        select=[(x[1],x[2]) for x in self.data[addr]  ]
        res=self.compressFileNames(select)
        res+=" "
        if self.containFloatMod(addr):
            res+="F"
        if self.containFloatCmp(addr):
            res+="?"
        return res

    def compressFileNames(self, tabFile):
        tabToTreat=tabFile
        res=""
        while len(tabToTreat)!=0:
            fileName=tabToTreat[0][0]
            select=[(x[1]) for x in tabToTreat if x[0]==fileName ]
            tabToTreat=[x for x in tabToTreat if x[0]!=fileName ]
            res+=fileName +"("+self.compressLine(select)+")"
        return res

    def compressLine(self, lineTab):
        res=""
        intTab=list(set([int(x) for x in lineTab]))
        intTab.sort()
        while len(intTab)!=0:
            begin=intTab[0]
            nbSuccessor=0
            for i in range(len(intTab))[1:]:
                if intTab[i]==begin+i:
                    nbSuccessor+=1
                else:
                    break
            if nbSuccessor==0:
                res+=str(begin)+","
            else:
                res+=str(begin)+"-"+str(begin+nbSuccessor)+","
            intTab=intTab[nbSuccessor+1:]
        return res[0:-1]

    def getStrToPrint(self, addr):
        return self.compressMarks(self.data[addr])

    def print(self):
        for addr in self.data:
            print(self.compressMarks(self.data[addr]))

    def addrToIgnore(self, addr, ignoreList):
        listOfFile=[fileName for sym,fileName,num, containFloatMod, containFloatCmp in self.data[addr]]
        for fileName in listOfFile:
            if fileName in ignoreList:
                return True
        return False

    def getListOfSym(self,addr):
        return list(set([sym for sym,fileName,num, containFloatMod, containFloatCmp in self.data[addr]]))

class traceName:
    def __init__(self, trace_kind):
        self.trace_kind=trace_kind
        if not trace_kind in ["bb","back"]:
            print('error trace_kind sould be in ["bb","back"]')
            sys.exit(42)

    def bbPrefixName(self):
        if self.trace_kind=="bb":
            return "trace_bb_info.log-"
        if self.trace_kind=="back":
            return "bbAddrInfo-"

    def covPrefixName(self):
        if self.trace_kind=="bb":
            return "trace_bb_cov.log-"
        if self.trace_kind=="back":
            return "backCoverInfo-"

    def backAddrPrefixName(self):
        if self.trace_kind=="back":
            return "backAddrInfo-"
        print("Invalid Trace Kind")
        sys.exit(42)

    def bbName(self,pid):
        return self.bbPrefixName()+str(pid)

    def covName(self,pid):
        return self.covPrefixName()+str(pid)

    def backAddrName(self,pid):
        return self.backAddrPrefixName()+str(pid)

    def bbNameForCovName(self, covName):
        return covName.replace(self.covPrefixName(), self.bbPrefixName())


class covReader:
    def __init__(self,pid, rep, trace_kind):
        self.pid=pid
        self.rep=rep
        self.tName=traceName(trace_kind)
        self.bbInfo=bbInfoReader(self.rep / self.tName.bbName(pid), trace_kind)
        covFile=openGz(self.rep / self.tName.covName(pid))

        self.cov=self.readCov(covFile)

    def readCov(self, cov):
        res=[]
        currentNumber=-1
        dictRes={}
        while True:
            line=cov.readline()
            if line in [None,""]:
                if currentNumber!=-1:
                    res+=[dictRes]
                break
            if line=="cover-"+str(currentNumber+1)+"\n":
                if currentNumber!=-1:
                    res+=[dictRes]
                currentNumber+=1
                dictRes={}
                continue
            if self.tName.trace_kind=="bb":
                (index,sep, num)=(line).strip().partition(":")
                dictRes[index]=int(num)
            if self.tName.trace_kind=="back":
                #"[76699587] 9:76699587,77091480,77146041,67113775,67114088,79412601,67113353,137422174823,27     5       9325"
                spline=line.strip().split(' ')
                addr=spline[0][1:-1]
                num=spline[1].split('\t')[1]
                if addr in dictRes:
                    dictRes[addr]+=int(num)
                else:
                    dictRes[addr]=int(num)

        return res

    def writePartialCover(self,filenamePrefix="", pidMap=None):
        if pidMap!=None:
            handler=openGz(self.rep / "pidMap" ,"w")
            for pid in pidMap:
                handler.write(str(pid)+ " => "+pidMap[pid]+ "\n")

        for num in range(len(self.cov)):
            resTab=[(index,num,self.bbInfo.getListOfSym(index),self.bbInfo.compressMarksWithoutSym(index)) for index,num in self.cov[num].items() ]
            resTab.sort( key= itemgetter(2,3,0)) # 2 sym  3 compress string 0 index
            pidStr=str(self.pid)
            if pidMap!=None:
                pidStr=pidMap[self.pid]

            handler=openGz(self.rep / ("%scover%05d-%s"%(filenamePrefix ,num, pidStr)),"w")
            for (index,count,sym, strBB) in resTab:
                handler.write("%d\t: %s\n"%(count,strBB))


class addrBackReader:

    def __init__(self,fileName, trace_kind):
        self.trace_kind=trace_kind
        self.read(fileName)

    def read(self,fileName):
        self.data={}
        if not self.trace_kind in ["back"]:
            print("invalid trace kind")
            sys.exit(42)

        fileHandler=openGz(fileName)
        line=fileHandler.readline()
        while not line in [None, ''] :
            spline=(line.strip()).split('\t')
            addr, sym, sourceFile, lineNum=spline
            if not addr in self.data:
                self.data[addr]=(sym,sourceFile,lineNum)
            else:
                if self.data[addr]!=(sym,sourceFile,lineNum):
                    print("addrBackReader incompatible addr ",addr)
                    print("fileName:", fileName)
            line=fileHandler.readline()

    def getBackStr(self, addr):
        if addr in self.data.keys():
            sym,sourceFile,lineNum=self.data[addr]
            return sym + "\t" + sourceFile+":"+lineNum
        else:
            print("unknonw addr", addr)
            return "?"+addr


class backCovReader:
    def __init__(self,pid, rep, trace_kind):
        self.pid=pid
        self.rep=rep
        assert(trace_kind in ["back"])
        self.tName=traceName(trace_kind)

        self.bbInfo=bbInfoReader(self.rep / self.tName.bbName(pid), trace_kind)
        self.addrBackInfo=addrBackReader(self.rep / self.tName.backAddrName(pid), trace_kind)
        covFile=openGz(self.rep / self.tName.covName(pid))

        self.backCov=self.readBackCov(covFile)
        self.structureData()


    def readBackCov(self, covFile):
        res=[]
        currentNumber=-1
        dictRes={}
        while True:
            line=covFile.readline()
            if line in [None,""]:
                if currentNumber!=-1:
                    res+=[dictRes]
                break
            if line=="cover-"+str(currentNumber+1)+"\n":
                if currentNumber!=-1:
                    res+=[dictRes]
                currentNumber+=1
                dictRes={}
                continue

            if self.tName.trace_kind=="back":
                #"[76699587] 9:76699587,77091480,77146041,67113775,67114088,79412601,67113353,137422174823,27     5       9325"
                spline=line.strip().split(' ')
                addr=spline[0][1:-1]

                splineRemain=spline[1].split('\t')
                num=splineRemain[1]
                index=splineRemain[2]
                backTabStr=splineRemain[0].split(':')[1]

                dictRes[(addr, backTabStr)]=(int(num), int(index))
        return res

    def structureData(self):
        #[ ("addrBack", [...]) ,("addrBB", minIndex,addrrBB, flatIndex,dataBB), ("addrBack",minIndex [.. ]) ]
        self.backTreeCov=[]
        for cov in self.backCov:
            treeData=[]
            for (addr,backTabStr) in cov.keys():
                backTab=backTabStr.split(',')
                dataBB=cov[(addr, backTabStr)]
                self.addTreeNode(treeData, addr, backTab, dataBB)
            self.minIndexAndSortTab(treeData)
            self.setIndexForFlatData(treeData, 0)
            self.backTreeCov+=[treeData]


    def minIndexAndSortTab(self, tree):
        minIndex=min([self.minIndexAndSortChild(subTree)  for subTree in tree])
        tree.sort(key=itemgetter("minCallIndex"))
        return minIndex

    def minIndexAndSortChild(self, subTree):
        if subTree["addrKind"]=="addrBB":
            return subTree["minCallIndex"]
        if subTree["addrKind"]=="addrBack":
            subTree["minCallIndex"]=self.minIndexAndSortTab(subTree["child"])
            return subTree["minCallIndex"]
        return None

    def setIndexForFlatData(self, tree, indexInit):
        index=indexInit
        for subTree in tree:
            if subTree["addrKind"]=="addrBack":
                index=self.setIndexForFlatData(subTree["child"], indexInit)
            if subTree["addrKind"]=="addrBB":
                subTree["flatIndex"]=index
                dataBB=subTree["data"]
                index+=1
        return index

    def addTreeNode(self, tree, addrBB, tabBack, dataBB):
        if tabBack==[]:
            tree+=[{"addrKind":"addrBB", "addrBB":addrBB, "flatIndex":None, "minCallIndex":dataBB[1] ,"data":dataBB}]
        else:
            lastAddrBack=tabBack[-1]
            remainBack=tabBack[0:-1]
            for subTree in tree:
                if subTree["addrKind"]=="addrBack":
                    if subTree["addrBack"]==lastAddrBack:
                        self.addTreeNode(subTree["child"], addrBB, remainBack, dataBB)
                        return
            tree+=[{"addrKind":"addrBack", "addrBack":lastAddrBack , "child":[]}]
            self.addTreeNode(tree[-1]["child"], addrBB, remainBack,dataBB)

    def writeTree(self, handler, tree, deep=0):
        for subTree in tree:
            if subTree["addrKind"]=="addrBack":
                addrBack=subTree["addrBack"]
                handler.write("\t"*deep+self.addrBackInfo.getBackStr(addrBack)+"\n")
                self.writeTree(handler, subTree["child"], deep+1)
            if subTree["addrKind"]=="addrBB":
                addrBB=subTree["addrBB"]
                numBB=subTree["data"][0]
                handler.write("\t"*deep+ self.bbInfo.compressMarksWithoutSym(addrBB) +"\t"+str(numBB) +"\n" )


    def writePartialBackCover(self,filenamePrefix="", pidMap=None):
        if pidMap!=None:
            handler=openGz(self.rep / "pidMap" ,"w")
            for pid in pidMap:
                handler.write(str(pid)+ " => "+pidMap[pid]+ "\n")

        for numCov in range(len(self.backTreeCov)):
            pidStr=str(self.pid)
            if pidMap!=None:
                pidStr=pidMap[self.pid]

            handler=openGz(self.rep / ("%scoverBack%05d-%s"%(filenamePrefix ,numCov, pidStr)),"w")
            self.writeTree(handler,self.backTreeCov[numCov])


class covMerge:
    """Class CovMerge allows to merge covers with counter of (success/failure) x (equal cover, diff cover).
    The method writePartialCover will compute from this counter a correlation coefficient for each covered bb"""

    def __init__(self, covRef):
        self.covRef=covRef
        print("covMerged with reference : %s"%(self.covRef.rep))
        self.init()

    def initCounterGlobal(self):
        self.success=0
        self.fail=0

    def incCounterGlobal(self,status):
        if status:
            self.success+=1
        else:
            self.fail+=1

    def initCounterLine(self):
        return (0,self.fail,0,self.success) #NbFailDiff, NbFailEqual, NbSuccessDiff, NbSuccessEqual
    def incCounterLine(self, counter, equalCounter, status):
        diff=0
        equal=0
        if equalCounter:
            equal=1
        else:
            diff=1

        if status:
            return (counter[0], counter[1] , counter[2]+diff, counter[3]+equal)
        else:
            return (counter[0]+diff, counter[1]+equal, counter[2], counter[3])

    def init(self):
        #init global counter
        self.initCounterGlobal()
        #load the first cover
        self.covMerged=[]
        for num in range(len(self.covRef.cov)): #loop over snapshots
            mergeDict={}
            for index,num in self.covRef.cov[num].items(): #loop over basic-bloc
                #get basic-bloc information
                sym=self.covRef.bbInfo.getListOfSym(index)[0]
                strLine=self.covRef.bbInfo.compressMarksWithoutSym(index)
                if not (sym,strLine) in mergeDict:
                    mergeDict[(sym,strLine)]=(num,self.initCounterLine())#bb not yet seen
                else:
                    mergeDict[(sym,strLine)]=(mergeDict[(sym,strLine)][0]+num,self.initCounterLine()) #bb already seen

            self.covMerged+=[mergeDict]


    def addMerge(self, cov, status):

        if len(cov.cov) != len(self.covRef.cov):
            print("addMerge : problem with the number of sync point")
            sys.exit()
        for num in range(len(cov.cov)): #loop over snapshots

            #use intermediate resDic to collapse bb with the same couple sym,strLine
            resDic={}
            for index,numLine in cov.cov[num].items():
                sym=cov.bbInfo.getListOfSym(index)[0]
                strLine=cov.bbInfo.compressMarksWithoutSym(index)
                if not (sym,strLine) in resDic:
                    resDic[(sym,strLine)]=numLine
                else:
                    resDic[(sym,strLine)]+=numLine
            #loop over collapse bb
            for ((sym, strLine), numLine) in resDic.items():
                if (sym,strLine)in self.covMerged[num]:
                    merged=self.covMerged[num][(sym,strLine)]
                    self.covMerged[num][(sym,strLine)]=(merged[0], self.incCounterLine(merged[1], merged[0]==numLine,status))
                else:
                    merged=(0, self.incCounterLine(self.initCounterLine(), False,status))
                    self.covMerged[num][(sym,strLine)]=merged
            #loop over bb not seen in this run
            for ((sym,strLine),merged) in self.covMerged[num].items():
                if not (sym,strLine)  in resDic:
                    self.covMerged[num][(sym,strLine)]=(merged[0], self.incCounterLine(merged[1], False,status))
        #update the global counter
        self.incCounterGlobal(status)


    def indicatorFromCounter(self, localCounter, name=""):
        nbFailDiff=   localCounter[0]
        nbFailEqual=  localCounter[1]
        nbSuccessDiff=localCounter[2]
        nbSuccessEqual=localCounter[3]

        nbSuccess=self.success
        nbFail=   self.fail

        if nbFailDiff +nbFailEqual !=nbFail:
            print("Assert Fail error")
            print("nbFail:",nbFail)
            print("nbFailDiff:",nbFailDiff)
            print("nbFailEqual:",nbFailEqual)
            return None


        if nbSuccessDiff +nbSuccessEqual !=nbSuccess:
            print("Assert Success error")
            print("nbSuccess:",nbSuccess)
            print("nbSuccessDiff:",nbSuccessDiff)
            print("nbSuccessEqual:",nbSuccessEqual)
            return None


        if name=="standard":
            return float(nbFailDiff + nbSuccessEqual)/ float(nbSuccess+nbFail)

        if name=="biased":
            return 0.5* (float(nbFailDiff)/ float(nbFail) + float(nbSuccessEqual)/float(nbSuccess))

        return None



    def writePartialCover(self,rep=".",filenamePrefix="", typeIndicator="standard"):

        for num in range(len(self.covMerged)): #loop over snapshots
            maxIndicator=0
            handler=openGz(rep+"/%scoverMerged%05d"%(filenamePrefix ,num),"w")
            partialCovMerged=self.covMerged[num]

            resTab=[(sym, strLine,
                     partialCovMerged[(sym,strLine)][0],
                     self.indicatorFromCounter(partialCovMerged[(sym,strLine)][1], typeIndicator ) ) for ((sym,strLine),counter) in partialCovMerged.items() ]

            resTab.sort( key= itemgetter(0,1)) # 2 sym  3 compress string 0 index
            for i in range(len(resTab)): #loop over sorted cover item
                sym,strLine,numLineRef,indicator=resTab[i]
                if indicator==None:
                    print("resTab[i]:",resTab[i])
                    print("partialCovMerged:",partialCovMerged[(sym,strLine)])
                    handler.write("none\t: %s\n"%(strLine))
                else:
                    maxIndicator=max(maxIndicator,indicator)
                    handler.write("%.2f\t: %s\n"%(indicator,strLine))

            print("Num: ", num , "\tMaxindicator: ", maxIndicator)

class statusReader:
    """Class to provide the status of a run"""
    # should maybe be a function instead of a class
    def __init__(self,pid, rep, runEval=None,runCmp=None, repRef=None):
        self.pid=pid
        self.rep=rep
        self.isSuccess=None
        if runCmp!=None:
            self.runCmpScript(runCmp, repRef)
            return
        if runEval!=None:
            self.runEvalScript(runEval)
            return
        self.read()

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

    def read(self):
        pathName=self.rep / "dd.return.value"
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


class cmpToolsCov:
    """Class to write partial cover of several executions :
    with writePartialCover the object write a partial cover for each execution
    with mergedCov the object write one merged partial cover with correlation information"""

    def __init__(self, tabPidRep, runCmp=None, runEval=None, trace_kind="bb"):
        self.tabPidRep=tabPidRep
        self.runCmp=runCmp
        self.runEval=runEval
        self.trace_kind=trace_kind

    def findRefForMerge(self):
        if runCmp!=None:
            self.refIndex=self.findRef(patternList=["ref","Ref","nearest","Nearest"])
            return
        if runEval!=None:
            self.refIndex=self.findRefDD(pattern="ref", optionalPattern="Nearest")
            return
        self.refIndex=self.findRefDD(pattern="ref", optionalPattern="dd.line/ref")



    def writePartialCover(self,filenamePrefix="", pidMap=None):
        """Write partial cover for each execution (defined by a tab of pid)"""
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            cov=covReader(pid,rep, self.trace_kind)
            cov.writePartialCover(filenamePrefix, pidMap=pidMap)


    def writePartialBack(self,filenamePrefix="", pidMap=None):
        """Write partial cover for each execution (defined by a tab of pid)"""
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            covBack=backCovReader(pid,rep, self.trace_kind)
            covBack.writePartialBackCover(filenamePrefix, pidMap=pidMap)

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
                if rep.endswith(pattern) and optionalPattern in rep and success:
                    return index
            print('Optional failed')
        for index in range(len(self.tabPidRep)):
            (pid,rep)=self.tabPidRep[index]
            success=self.getStatus(pid,rep)
            if rep.endswith(pattern) and success:
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


    def writeMergedCov(self,estimator):
        """Write merged Cov with correlation indice  between coverage difference and sucess/failure status"""

        #check the presence of success and failure
        (nbSuccess, nbFail)=self.countStatus()
        print("NbSuccess: %d \t nbFail %d"%(nbSuccess,nbFail))
        if nbFail==0 or nbSuccess==0:
            print("mergeCov need Success/Fail partition")
            sys.exit()


        covMerged= covMerge(covReader(*self.tabPidRep[self.refIndex], self.trace_kind))
        #Loop with addMerge to reduce memory peak

        printIndex=[int(float(p) * len(self.tabPidRep) /100.)  for p in (list(range(0,100,10))+[1,5])]
        printIndex +=[1,  len(self.tabPidRep)-1]

        for i in range(len(self.tabPidRep)):
            if i==self.refIndex:
                continue
            pid,rep=self.tabPidRep[i]
            covMerged.addMerge(covReader(pid,rep,self.trace_kind), self.getStatus(pid,rep))
            if i in printIndex:
                pourcent=float(i+1)/ float(len(self.tabPidRep)-1)
                if i >=self.refIndex:
                    pourcent=float(i)/ float(len(self.tabPidRep)-1)
                print( "%.1f"%(pourcent*100)    +"% of coverage data merged")
        #covMerged.writePartialCover(typeIndicator="standard")
        covMerged.writePartialCover(typeIndicator=estimator)

