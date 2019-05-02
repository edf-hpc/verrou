#!/usr/bin/env python3
import re
import sys
from operator import itemgetter, attrgetter
import gzip
import os


class openGz:
    """ Class to read/write  gzip file or ascii file """
    def __init__(self,name, mode="r", compress=None):
        self.name=name
        if os.path.exists(name+".gz") and compress==None:
            self.name=name+".gz"

        if (self.name[-3:]==".gz" and compress==None) or compress==True:
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
    def __init__(self,fileName):
        self.read(fileName)

    def read(self,fileName):

        self.data={}
        self.dataMax={}
        self.dataCorrupted={}
        regularExp=re.compile("([0-9]+) : (.*) : (\S*) : ([0-9]+)")
        fileHandler=openGz(fileName)

        line=fileHandler.readline()
        counter=0
        while not line in [None, ''] :
            m=(regularExp.match(line.strip()))
            if m==None :
                print("error read fileName line:",[line])
                sys.exit()
            addr, sym, sourceFile, lineNum= m.groups()
            if addr in self.data:
                if not (sym,sourceFile,lineNum) in self.data[addr]:
                    self.data[addr]+=[(sym,sourceFile,lineNum)]
                if self.dataMax[addr]+1!= counter:
                    self.dataCorrupted[addr]=True
                self.dataMax[addr]=counter
            else:
                self.data[addr]=[(sym,sourceFile,lineNum)]
                self.dataMax[addr]=counter
                self.dataCorrupted[addr]=False
            counter+=1
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

    def compressMarksWithoutSym(self, addr):
        select=[(x[1],x[2]) for x in self.data[addr]  ]
        return self.compressFileNames(select)

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
        intTab=[int(x) for x in lineTab]
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

    def isCorrupted(self,addr):
        return self.dataCorrupted[addr]

    def print(self):
        for addr in self.data:
            print(self.compressMarks(self.data[addr]))

    def addrToIgnore(self, addr, ignoreList):
        listOfFile=[fileName for sym,fileName,num in self.data[addr]]
        for fileName in listOfFile:
            if fileName in ignoreList:
                return True
        return False

    def getListOfSym(self,addr):
        return list(set([sym for sym,fileName,num in self.data[addr]]))

class traceReader:

    def __init__(self,pid):
        self.trace=openGz("trace_bb_trace.log-"+str(pid))
        self.traceOut=openGz("trace_bb_trace.log-"+str(pid)+"-post","w")
        self.bbInfo=bbInfoReader("trace_bb_info.log-"+str(pid))

    def readLine(self,comment=True):
        addr=self.trace.readline().strip()
        if addr==None or addr=="":
            return None

        if not (self.bbInfo.addrToIgnore(addr, self.ignoreList ) or self.bbInfo.isCorrupted(bbAddr)):
            if comment:
                return addr + " " +self.bbInfo.getStrToPrint(addr)
            else:
                return addr
        return ""

    def writeFilteredAndComment(self,ignoreList=['vfprintf.c','printf_fp.c','rounding-mode.h']):
        self.ignoreList=ignoreList
        line=self.readLine()
        while not line in [None]:
            if line !="":
                self.traceOut.write(line+"\n")
            line=self.readLine()



class cmpTools:
    def __init__(self, pid1, pid2):
        self.bbInfo1=bbInfoReader("trace_bb_info.log-"+str(pid1))
        self.bbInfo2=bbInfoReader("trace_bb_info.log-"+str(pid2))

        self.bbInfo1.print()
        self.bbInfo2.print()
        self.trace1=open("trace_bb_trace.log-"+str(pid1))
        self.trace2=open("trace_bb_trace.log-"+str(pid2))
        self.context=2
        self.bufferContext=[None]*self.context


    def writeLines(self, addr1,num1,addr2,num2):
        toPrint1=self.bbInfo1.getStrToPrint(addr1)
        toPrint2=self.bbInfo2.getStrToPrint(addr2)
        if addr1==addr2:
            resLine= "num: "+ str(num1)+"/"+str(num2) + " == " + addr1+ "\t"
            if(toPrint1==toPrint2):
                resLine+= toPrint1
            else:
                if self.bbInfo1.isCorrupted(addr1) and self.bbInfo2.isCorrupted(addr2):
                    print("corrupted \n")
                    print('toPrint1:',toPrint1)
                    print('toPrint2:',toPrint2)
                else:
                    print("Serious problem")
                    sys.exit()

        if addr1!=addr2:
            resLine= "num: "+ str(num1)+"/"+str(num2)+" " + addr1+" != " + addr2+ "\n" +toPrint1 +"\n"+toPrint2
        print(resLine)

    def printContext(self):
        for i in range(self.context):
            buffer=self.bufferContext[self.context-i-1]
            if buffer !=None:
                (addr1,lineNum1, addr2, lineNum2)=buffer
                self.writeLines(addr1,lineNum1, addr2, lineNum2)

    def readUntilDiffer(self, ignoreList=[]):
        self.ignoreList=ignoreList
        addr1, addr2=("","")
        lineNum1,lineNum2=(0,0)
#        lineNumInc1, lineNumInc2=(0,0)
        while addr1==addr2:
            self.bufferContext=[(addr1,lineNum1, addr2, lineNum2)]+self.bufferContext[0:-1]
            addr1,lineNumInc1=self.read(self.trace1,self.bbInfo1)
            addr2,lineNumInc2=self.read(self.trace2,self.bbInfo2)
            lineNum1+=lineNumInc1
            lineNum2+=lineNumInc2

            if lineNum1 % 1000 ==0:
                print( "lineNum1: ", lineNum1)

        self.printContext()
        self.writeLines( addr1, lineNum1, addr2,lineNum2)
        #        print(self.compressMarks(self.data["587F00C0"]))

    def read(self, traceFile, bbInfo):
        addr=traceFile.readline().strip()
        counter=1
        while not addr in ["", None]:
            if not (bbInfo.addrToIgnore(addr, self.ignoreList ) or bbInfo.isCorrupted(addr)):
                return (addr,counter)
            addr=traceFile.readline().strip()
            counter+=1
        return (None,counter)


class covReader:
    def __init__(self,pid, rep):
        self.pid=pid
        self.rep=rep
        self.bbInfo=bbInfoReader(self.rep+"/trace_bb_info.log-"+str(pid))
        covFile=openGz(self.rep+"/trace_bb_cov.log-"+str(pid))

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
            (index,sep, num)=(line).strip().partition(":")
            dictRes[index]=int(num)

        return res

    def writePartialCover(self,filenamePrefix=""):

        for num in range(len(self.cov)):
            resTab=[(index,num,self.bbInfo.getListOfSym(index),self.bbInfo.compressMarksWithoutSym(index)) for index,num in self.cov[num].items() ]
            resTab.sort( key= itemgetter(2,3,0)) # 2 sym  3 compress string 0 index

            handler=openGz(self.rep+"/"+filenamePrefix+"cover"+str(num)+"-"+str(self.pid),"w")
            for (index,count,sym, strBB) in resTab:
                handler.write("%d\t: %s\n"%(count,strBB))


class covMerge:
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
        self.covMerged=[]
        self.initCounterGlobal()
        for num in range(len(self.covRef.cov)):
            mergeDict={}
            for index,num in self.covRef.cov[num].items():
                sym=self.covRef.bbInfo.getListOfSym(index)[0]
                strLine=self.covRef.bbInfo.compressMarksWithoutSym(index)
                if not (sym,strLine) in mergeDict:
                    mergeDict[(sym,strLine)]=(num,self.initCounterLine())
                else:
                    mergeDict[(sym,strLine)]=(mergeDict[(sym,strLine)][0]+num,self.initCounterLine())

            self.covMerged+=[mergeDict]


    def addMerge(self, cov, status):

        if len(cov.cov) != len(self.covRef.cov):
            print("addMerge : problem with the number of sync point")
            sys.exit()
        for num in range(len(cov.cov)):
            resDic={}
            for index,numLine in cov.cov[num].items():
                sym=cov.bbInfo.getListOfSym(index)[0]
                strLine=cov.bbInfo.compressMarksWithoutSym(index)
                if not (sym,strLine) in resDic:
                    resDic[(sym,strLine)]=numLine
                else:
                    resDic[(sym,strLine)]+=numLine

            for ((sym, strLine), numLine) in resDic.items():
                if (sym,strLine)in self.covMerged[num]:
                    merged=self.covMerged[num][(sym,strLine)]
                    self.covMerged[num][(sym,strLine)]=(merged[0], self.incCounterLine(merged[1], merged[0]==numLine,status))
                else:
                    merged=(0, self.incCounterLine(self.initCounterLine(), False,status))
                    self.covMerged[num][(sym,strLine)]=merged
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

        for num in range(len(self.covMerged)):
            maxIndicator=0
            handler=openGz(rep+"/"+filenamePrefix+"coverMerged"+str(num),"w")
            partialCovMerged=self.covMerged[num]

            resTab=[(sym, strLine,
                     partialCovMerged[(sym,strLine)][0],
                     self.indicatorFromCounter(partialCovMerged[(sym,strLine)][1], typeIndicator ) ) for ((sym,strLine),counter) in partialCovMerged.items() ]

            resTab.sort( key= itemgetter(0,1)) # 2 sym  3 compress string 0 index
            for i in range(len(resTab)):
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

    def __init__(self,pid, rep):
        self.pid=pid
        self.rep=rep
        self.isSuccess=None
        self.read()

    def read(self):
        pathName=os.path.join(self.rep, "returnVal")
        if os.path.exists(pathName):
            value=int(open(pathName).readline().strip())
            if value==0:
                self.isSuccess=True
            else:
                self.isSuccess=False
        else:
            if self.rep.endswith("ref"):
                print("Consider ref as a success")
                self.isSuccess=True

    def getStatus(self):
        return self.isSuccess


class cmpToolsCov:

    def __init__(self, tabPidRep):
        self.tabPidRep=tabPidRep
#        self.covTab=[covReader(pid,rep)  for (pid,rep) in self.tabPidRep]
#        self.statusTab=[statusReader(pid,rep) for (pid,rep) in self.tabPidRep]

    def writePartialCover(self, filenamePrefix=""):
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            cov=covReader(pid,rep)
            cov.writePartialCover(filenamePrefix)

    def writeStatus(self):
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            status=statusReader(pid,rep)
            success=status.getStatus()
            print( rep+":" + str(success))

    def countStatus(self):
        nbSuccess=0
        nbFail=0
        for i in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[i]
            status=statusReader(pid,rep)
            success=status.getStatus()
            if success:
                nbSuccess+=1
            else:
                nbFail+=1
        return (nbSuccess, nbFail)


    def findRef(self, pattern="ref", optionalPattern=None):
        if optionalPattern!=None:
            for index in range(len(self.tabPidRep)):
                (pid,rep)=self.tabPidRep[index]
                status=statusReader(pid,rep)
                success=status.getStatus()
                if rep.endswith(pattern) and optionalPattern in rep and success:
                    return index
        print('Optional failed')
        for index in range(len(self.tabPidRep)):
            (pid,rep)=self.tabPidRep[index]
            status=statusReader(pid,rep)
            success=status.getStatus()
            if rep.endswith(pattern) and success:
                return index
        print("Warning : pattern not found" )
        print("Switch to first Success reference selection")
        for index in range(len(self.tabPidRep)):
            pid,rep=self.tabPidRep[index]
            status=statusReader(pid,rep)
            success=status.getStatus()
            if success:
                return index
        print("Error fail only : cmpToolsCov is ineffective" )
        sys.exit(42)

    def mergeCov(self):

        (nbSuccess, nbFail)=self.countStatus()
        print("NbSuccess: %d \t nbFail %d"%(nbSuccess,nbFail))
        if nbFail==0 or nbSuccess==0:
            print("mergeCov need Success/Fail partition")
            sys.exit()

        refIndex=self.findRef(pattern="ref", optionalPattern="dd.sym/ref")
        covMerged= covMerge(covReader(*self.tabPidRep[refIndex]))
        for i in range(len(self.tabPidRep)):
            if i==refIndex:
                continue
            pid,rep=self.tabPidRep[i]
            covMerged.addMerge(covReader(pid,rep), statusReader(pid,rep).getStatus())
#        covMerged.writePartialCover(typeIndicator="standard")
        covMerged.writePartialCover(typeIndicator="biased")



def extractPidRep(fileName):
    rep=os.path.dirname(fileName)
    if rep=="":
        rep="."
    baseFile=os.path.basename(fileName)
    begin="trace_bb_cov.log-"
    if baseFile.startswith(begin):
        pid=int((baseFile.replace(begin,'')).replace(".gz",""))
        return (pid,rep)
    return None

def selectPidFromFile(fileNameTab):
    return [extractPidRep(fileName)  for fileName in fileNameTab]



def usage():
    print ("Usage: genCov.py [--help] [--genCov] [--genCovCorrelation]  file list of type trace_bb_cov.log* " )

if __name__=="__main__":
    options=[arg for arg in sys.argv[1:]]

    genCov=False
    if "--genCov" in options:
        options.remove("--genCov")
        genCov=True

    genCovCorrelation=False
    if "--genCovCorrelation" in options:
        options.remove("--genCovCorrelation")
        genCovCorrelation=True

    if not (genCovCorrelation or genCovCorrelation):
        genCov=True

    if "--help" in options:
        usage()
        sys.exit()

    if len(options)<1 and genCov:
        usage()
        print("--genCov required at least 1 argument")
        sys.exit()
    if len(options)<2 and genCovCorrelation:
        usage()
        print("--genCov required at least 2 arguments")
        sys.exit()

    cmp=cmpToolsCov(selectPidFromFile(options))
    if genCov:
        cmp.writePartialCover()
    if genCovCorrelation:
        cmp.mergeCov()
