#!/usr/bin/env python3
import re
import sys
from operator import itemgetter, attrgetter


class bbInfoReader:

    def __init__(self,fileName):
        self.read(fileName)


    def read(self,fileName):

        self.data={}
        self.dataMax={}
        self.dataCorrupted={}
        regularExp=re.compile("([0-9]+) : (.*) : (\S*) : ([0-9]+)")
        fileHandler=open(fileName)

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
        self.trace=open("trace_bb_trace.log-"+str(pid))
        self.traceOut=open("trace_bb_trace.log-"+str(pid)+"-post","w")
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


class cmpToolsCov:

    def __init__(self, pid1, pid2):
        self.pid1=pid1
        self.pid2=pid2
        self.bbInfo1=bbInfoReader("trace_bb_info.log-"+str(pid1))
        self.bbInfo2=bbInfoReader("trace_bb_info.log-"+str(pid2))
        self.cov1=open("trace_bb_cov.log-"+str(pid1))
        self.cov2=open("trace_bb_cov.log-"+str(pid2))


    def readCov(self, cov, bbinfo):
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
            dictRes[index]=(index,num, bbinfo.getListOfSym(index), bbinfo.compressMarksWithoutSym(index) )

        return res

    def writePartialCover(self,filenamePrefix=""):
        res1=self.readCov(self.cov1, self.bbInfo1)
        res2=self.readCov(self.cov2, self.bbInfo2)

        for num in range(min(len(res1),len(res2))):
            resTab1=[value for key,value in res1[num].items() ]
            resTab2=[value for key,value in res2[num].items() ]

            resTab1.sort( key= itemgetter(2,3,0))
            resTab2.sort( key= itemgetter(2,3,0))
            handler1=open(filenamePrefix+"cover"+str(num)+"-"+str(self.pid1),"w")
            handler2=open(filenamePrefix+"cover"+str(num)+"-"+str(self.pid2),"w")
            for (index,count,sym, strBB) in resTab1:
                handler1.write("%s\t: %s\n"%(count,strBB))
            for (index,count,sym, strBB) in resTab2:
                handler2.write("%s\t: %s\n"%(count,strBB))



def extractPid(fileName):
    begin="trace_bb_cov.log-"
    if fileName.startswith(begin):
        return int(fileName.replace(begin,''))
    return None

def selectPidFromFile(fileName1, fileName2):
    return (extractPid(fileName1),extractPid(fileName2))


if __name__=="__main__":

    if len(sys.argv)!=3:
        print("Two args required")
        sys.exit()

    (pid1,pid2)=selectPidFromFile(sys.argv[1], sys.argv[2])
    cmp=cmpToolsCov(pid1,pid2)
    cmp.writePartialCover()
