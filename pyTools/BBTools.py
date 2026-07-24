
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
            regularExp=re.compile(r"([0-9]+)\|([0-9]+) : (.*) : (\S*) : ([0-9]+) : ([0,1]) : ([0,1])")
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
            #addr,index, sym, sourceFile, lineNum, containFloat, containFloatCmp, index=(None,None,None,None,None,None,None,None)
            if self.trace_kind=="bb":
                addr,index, sym, sourceFile, lineNum, containFloat, containFloatCmp= m.groups()
            else:
                addr,sym, sourceFile, lineNum, containFloat, containFloatCmp= m.groups()
                index=None
            containFloat=(containFloat in ["1","F"])
            containFloatCmp=(containFloatCmp in ["1","?"])

            if addr in self.data:
                if not (sym,sourceFile,lineNum,containFloat, containFloatCmp, index) in self.data[addr]:
                    self.data[addr]+=[(sym,sourceFile,lineNum,containFloat,containFloatCmp, index)]
            else:
                self.data[addr]=[(sym,sourceFile,lineNum,containFloat,containFloatCmp, index)]
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
        return list(set([sym for sym,fileName,num, containFloatMod, containFloatCmp, index in self.data[addr]]))

class mergebbInfoReader:

    def __init__(self, bbInfoReaderRoot, verbose=False):
        self.bbInfoReaderRoot=bbInfoReaderRoot
        self.data=self.bbInfoReaderRoot.data
        self.verbose=verbose

    def addBBInfoReader(self, newBBInfo):
        data=self.data
        newdata=newBBInfo.data
        for addr in newdata:
            if addr in data:
                if data[addr]!=newdata[addr]:
                    print("incompatible .bbInfoReader")
                    print("addr ["+str(addr)+"]" ,str( data[addr]), "=>", str(newdata[addr]))
                    return False
            else:
                if self.verbose:
                    print("newData[",addr,"]", newdata[addr])
                data[addr]=newdata[addr]
        return True

    def compressMarksWithoutSym(self, addr):
        return self.bbInfoReaderRoot.compressMarksWithoutSym(addr)


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
                (key,sep, num)=(line).strip().partition(":")
                (addr,sep,index)=key.partition("|")
                dictRes[addr]=int(num)
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
            print("unknown addr", addr)
            return "?("+addr+")"

    def isBelowMain(self, addr):
        return ("below main" in self.data[addr][0])

class mergeAddrBackReader:
    def __init__(self, backReader, verbose=False):
        self.backReader=backReader
        self.verbose=verbose

    def addBackReader(self, newBack):
        newData=newBack.data
        data=self.backReader.data
        for addr in newData:
            if addr in data:
                if data[addr]!=newData[addr]:
                    print("incompatible addr")
                    print(addr, "=>", data[addr], "!=", newData[addr])
                    return False
            else:
                if self.verbose:
                    print("New back addr", addr,  newData[addr])
                data[addr]=newData[addr]
        return True
    def getBackStr(self, addr):
        return self.backReader.getBackStr(addr)

    def isBelowMain(self, addr):
        return self.backReader.isBelowMain(addr)


class coverageReader:
    def __init__(self, pid, rep, status, trace_kind, mergeRoot=False):
        self.pid=pid
        self.rep=rep
        assert(trace_kind in ["bb"])
        self.tName=traceName(trace_kind)

        self.bbInfo=bbInfoReader(self.rep / self.tName.bbName(pid), trace_kind)
        self.status=status

        if mergeRoot:
            self.mergeIndex=0
            self.mergeNum=1
            self.statusTab=[status]
            self.bbInfo=mergebbInfoReader(self.bbInfo,verbose=True)
        else:
            self.mergeIndex=None
            self.mergeNum=None
            self.statusTab=None
        covFile=openGz(self.rep / self.tName.covName(pid))
        self.dataCov=self.readCoverage(covFile)

    def addMerge(self, covCurrent): #attention ne marche qu'avec le mode addr
        #add check bbInfo Coherence
        if not self.bbInfo.addBBInfoReader(covCurrent.bbInfo):
            print("incoherent merge bbInfo ")

        covCurrent.mergeIndex=self.mergeNum
        self.mergeNum+=1
        self.statusTab+=[covCurrent.status]

        for indexCov in range(len(self.dataCov)):
            for key in covCurrent.dataCov[indexCov]:
                data=covCurrent.dataCov[indexCov][key]
                if key in self.dataCov[indexCov]:
                    size=len(self.dataCov[indexCov][key])
                    if size < self.mergeNum-1:
                        self.dataCov[indexCov][key]+=[(0,None) for i in range(self.mergeNum-1- size)]
                    self.dataCov[indexCov][key].append(data)
                else:
                    buildOld=[(0,None) for i in range(self.mergeNum-1)]
                    buildOld.append(data)
                    self.dataCov[indexCov][key]=buildOld

    def endMerge(self):
        for indexCov in range(len(self.dataCov)):
            for key in self.dataCov[indexCov]:
                size=len(self.dataCov[indexCov][key])
                if size < self.mergeNum:
                    self.dataCov[indexCov][key]+= [(0,None) for i in range(self.mergeNum- size)]

    def readCoverage(self, cov):
        #attention duplication from covReader
        res=[] # tab indexed by cov index. Each element is a dict {addr/index: num}.  
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
                (addrindex,sep, num)=(line).strip().partition(":")
                (addr,sep,index)=addrindex.partition("|")
                if self.mergeIndex!=0:
                    dictRes[addr]=(int(num), int(index))
                else:
                    dictRes[addr]=[(int(num), int(index))]
            else:
                print("error only bb trace_kind is implemented")
                sys.exit()
        return res

    def structureData(self):
        nbCov=len(self.dataCov)
        self.dataSortCov=[ self.sortCov(self.dataCov[g] )   for  g in range(nbCov)]
        #add sort

    def minCallIndex(self, numIndexTab):
        return min([x for x in numIndexTab if x!=None])

    def sortCov(self, addrToNumIndexTab):
        res=[(addrBB, [x[0] for x in addrToNumIndexTab[addrBB]], self.minCallIndex([x[1] for x in  addrToNumIndexTab[addrBB]]) ) for addrBB in addrToNumIndexTab ]
        res.sort(key=lambda x: x[2])
        return res

    def writeData(self,handler, sortCov, outputTypeTab=["data"]):
        for bbAddr,rawDataTab,minCallIndex in sortCov:
            name=self.bbInfo.compressMarksWithoutSym(bbAddr)
            dataBBStr=None
            if self.mergeIndex !=0:
                assert(outputTypeTab==["data"])
                dataBBStr=str(rawDataTab[0])
            else:
                dataTab=[]
                for outputType in outputTypeTab:
                    if outputType=="data":
                        dataTab+=rawDataTab
                    elif outputType in ["biased","standard", "biased-stol", "fdr-stol"]:
                        dataTab+=computeEstimator( self.statusTab  , rawDataTab, [outputType])
                    else:
                        print("unknown outputType", outputType)
                        sys.exit(42)
                dataBBStr="\t".join([str(x) for x in  dataTab])
            handler.write(name +"\t"+ dataBBStr+"\n")

    def writePartialCover(self,outputDir=None,filenamePrefix="", pidMap=None, outputTypeTab=["data"]):
        self.structureData()
        if pidMap!=None:
            handler=openGz(self.rep / "pidMap" ,"w")
            for pid in pidMap:
                handler.write(str(pid)+ " => "+pidMap[pid]+ "\n")

        for numCov in range(len(self.dataSortCov)):
            pidStr=str(self.pid)
            if pidMap!=None:
                pidStr=pidMap[self.pid]

            outDir=self.rep
            if outputDir!=None:
                outDir=outputDir
            handler=openGz(Path(outDir) / ("%scover%05d-%s"%(filenamePrefix ,numCov, pidStr)),"w")
            self.writeData(handler,self.dataSortCov[numCov], outputTypeTab=outputTypeTab)

    def writeCSV(self, pathStr, header="", outputTypeTab=["data"]):
        self.structureData()

        for numCov in range(len(self.dataSortCov)):
            handler=openGz(Path(pathStr.replace("__NUM_COV__", "%i"%(numCov))),"w")
            handler.write(header)
            self.writeData(handler,self.dataSortCov[numCov], outputTypeTab=outputTypeTab)


class backCovReader:
    def __init__(self,pid, rep, status,trace_kind, mergeRoot=False):
        self.pid=pid
        self.rep=rep
        assert(trace_kind in ["back"])
        self.tName=traceName(trace_kind)

        self.bbInfo=bbInfoReader(self.rep / self.tName.bbName(pid), trace_kind)
        self.addrBackInfo=addrBackReader(self.rep / self.tName.backAddrName(pid), trace_kind)

        self.status=status
        if mergeRoot:
            self.mergeIndex=0
            self.mergeNum=1
            self.statusTab=[status]
            self.addrBackInfo=mergeAddrBackReader(self.addrBackInfo, verbose=False)
            self.bbInfo=mergebbInfoReader(self.bbInfo,verbose=False)
        else:
            self.mergeIndex=None
            self.mergeNum=None
            self.statusTab=None
        covFile=openGz(self.rep / self.tName.covName(pid))
        self.backCov=self.readBackCov(covFile)

    def addMerge(self, backCurrent):

        if not self.addrBackInfo.addBackReader(backCurrent.addrBackInfo):
            print("incoherent merge addrBackInfo ")
            sys.exit(42)
        if not self.bbInfo.addBBInfoReader(backCurrent.bbInfo):
            print("incoherent merge bbInfo ")
            sys.exit(42)

        backCurrent.mergeIndex=self.mergeNum
        self.mergeNum+=1
        self.statusTab+=[backCurrent.status]

        for indexCov in range(len(self.backCov)):
            for keyAddr, backAddr in backCurrent.backCov[indexCov]:
                key=keyAddr, backAddr
                data=backCurrent.backCov[indexCov][key]
                if key in self.backCov[indexCov]:
                    size=len(self.backCov[indexCov][key])
                    if size < self.mergeNum-1:
                        self.backCov[indexCov][key]+=[(0,None) for i in range(self.mergeNum-1- size)]
                    self.backCov[indexCov][key].append(data)
                else:
                    #print("debug ignored key", key, "mergeIndex", backCurrent.mergeIndex)
                    buildOld=[(0,None) for i in range(self.mergeNum-1)]
                    buildOld.append(data)
                    self.backCov[indexCov][key]=buildOld
                #backCurrent.backCov[indexCov][key]


    def endMerge(self):
        for indexCov in range(len(self.backCov)):
            for key in self.backCov[indexCov]:
                size=len(self.backCov[indexCov][key])
                if size < self.mergeNum:
                    self.backCov[indexCov][key]+= [(0,None) for i in range(self.mergeNum- size)]



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
                backTabStr=self.removeBelowMain(backTabStr)
                if self.mergeIndex!=0:
                    dictRes[(addr, backTabStr)]=(int(num), int(index))
                else:
                    dictRes[(addr, backTabStr)]=[(int(num), int(index))]
        return res

    def removeBelowMain(self, backTabStr):
        addrTab=backTabStr.split(',')
        for i in range(len(addrTab)):
            addr=addrTab[i]
            if self.addrBackInfo.isBelowMain(addr):
                return ",".join(addrTab[0:i])
        return backTabStr

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
        if tree==[]:
            return None
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
            minCallI=None
            if self.mergeIndex==0:
                #print("dataBB", dataBB)
                minCallI=min([x[1] for x in dataBB if x[1]!=None])
            else:
                minCallI=dataBB[1]
            tree+=[{"addrKind":"addrBB", "addrBB":addrBB, "flatIndex":None, "minCallIndex":minCallI ,"data":dataBB}]
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

    def writeTree(self, handler, tree, deep=0, outputTypeTab=["data"],csvFormat=False):
        for subTree in tree:
            if subTree["addrKind"]=="addrBack":
                addrBack=subTree["addrBack"]
                deepStr="\t"*deep
                if csvFormat:
                    deepStr= "|"+str(deep)+"\t"
                handler.write(deepStr+self.addrBackInfo.getBackStr(addrBack)+"\n")
                self.writeTree(handler, subTree["child"], deep+1, outputTypeTab=outputTypeTab, csvFormat=csvFormat)
            if subTree["addrKind"]=="addrBB":
                addrBB=subTree["addrBB"]
                dataBBStr=None
                if self.mergeIndex !=0:
                    assert(outputTypeTab==["data"])
                    dataBBStr=str(subTree["data"][0])
                else:
                    dataTab=[]
                    for outputType in outputTypeTab:
                        coverTab=[ x[0] for x in subTree["data"]]
                        if outputType=="data":
                            dataTab+=coverTab
                        elif outputType in ["biased","standard", "biased-stol", "fdr-stol"]:
                            dataTab+=computeEstimator( self.statusTab  , coverTab, [outputType])
                        else:
                            print("unknown outputType", outputType)
                            sys.exit(42)
                    dataBBStr="\t".join([str(x) for x in  dataTab])
                deepStr="\t"*deep
                if csvFormat:
                    deepStr= str(deep)+"\t"

                handler.write(deepStr+ self.bbInfo.compressMarksWithoutSym(addrBB) +"\t"+dataBBStr +"\n" )


    def writePartialBackCover(self,outputDir=None,filenamePrefix="", pidMap=None, outputTypeTab=["data"]):
        self.structureData()
        if pidMap!=None:
            handler=openGz(self.rep / "pidMap" ,"w")
            for pid in pidMap:
                handler.write(str(pid)+ " => "+pidMap[pid]+ "\n")

        for numCov in range(len(self.backTreeCov)):
            pidStr=str(self.pid)
            if pidMap!=None:
                pidStr=pidMap[self.pid]

            outDir=self.rep
            if outputDir!=None:
                outDir=outputDir
            handler=openGz(Path(outDir) / ("%scoverBack%05d-%s"%(filenamePrefix ,numCov, pidStr)),"w")
            self.writeTree(handler,self.backTreeCov[numCov], outputTypeTab=outputTypeTab,csvFormat=False)

    def writeCSV(self, pathStr, header="", outputTypeTab=["data"]):
        self.structureData()

        for numCov in range(len(self.backTreeCov)):
            handler=openGz(Path(pathStr.replace("__NUM_COV__", "%i"%(numCov))),"w")
            handler.write(header)
            self.writeTree(handler,self.backTreeCov[numCov], outputTypeTab=outputTypeTab,csvFormat=True)

def isIntegerEqualWithTol(value, ref, tol ):
    if value >= ref+tol[0] and value <= ref+tol[1]:
        return True
    return False

def countForEstimator(statusTab, counterTab, refIndex, tol=[0,0]):
    assert(statusTab[refIndex]==True)
    assert(len(statusTab) == len(counterTab))
    nbSuccess=0
    nbFail=0
    nbFailDiff, nbFailEqual, nbSuccessDiff, nbSuccessEqual=(0,0,0,0)

    for i in range(len(statusTab)):
        assert(statusTab[i] in [True,False])
        if statusTab[i]==True:
            nbSuccess+=1
            if  isIntegerEqualWithTol(counterTab[i], counterTab[refIndex], tol):
                nbSuccessEqual+=1
            else:
                nbSuccessDiff+=1
        else:
            nbFail+=1
            if  isIntegerEqualWithTol(counterTab[i], counterTab[refIndex], tol):
                nbFailEqual+=1
            else:
                nbFailDiff+=1
    dicRes= {"nbSuccess": nbSuccess, "nbFail":nbFail,
            "nbFailDiff":nbFailDiff,
            "nbFailEqual":nbFailEqual,
            "nbSuccessDiff":nbSuccessDiff,
            "nbSuccessEqual":nbSuccessEqual}
    return dicRes


def computeEstimator(statusTab, counterTab, estimatorTab, refIndex=0):
    assert(statusTab[refIndex]==True)
    assert(len(statusTab) == len(counterTab))

    countData=countForEstimator(statusTab, counterTab, refIndex)
    nbSuccess=countData["nbSuccess"]
    nbFail=countData["nbFail"]
    nbFailDiff=countData["nbFailDiff"]
    nbFailEqual=countData["nbFailEqual"]
    nbSuccessDiff=countData["nbSuccessDiff"]
    nbSuccessEqual=countData["nbSuccessEqual"]

    res=[]
    for estimator in estimatorTab:
        if estimator=="standard":
            res+=[float(nbFailDiff + nbSuccessEqual)/ float(nbSuccess+nbFail)]
        elif estimator=="biased":
            res+=[0.5* (float(nbFailDiff)/ float(nbFail) + float(nbSuccessEqual)/float(nbSuccess))]
        elif "-stol" in estimator:
            tolTab=[counterTab[i] -counterTab[refIndex] for i in range(len(statusTab)) if statusTab[i]]
            tolMin, tolMax=min(tolTab), max(tolTab)

            countDataTol=countForEstimator(statusTab, counterTab, refIndex, tol=[tolMin, tolMax])
            nbFailDiffTol=countDataTol["nbFailDiff"]
            #nbFailEqualTol=countDataTol["nbFailEqual"]
            nbSuccessDiffTol=countDataTol["nbSuccessDiff"]
            if nbSuccessDiffTol!=0:
                print("Bug nbSuccessDiff")
                print ("counterTab", counterTab)
                print ("countDataTol", countDataTol)
                sys.exit(42)

            nbSuccessEqualTol=countDataTol["nbSuccessEqual"]
            if estimator=="biased-stol":
                res+=[0.5* (float(nbFailDiffTol)/ float(nbFail) + float(nbSuccessEqualTol)/float(nbSuccess))]
            elif estimator=="fdr-stol": #fail diff ratio
                res+=[(float(nbFailDiffTol)/ float(nbFail))]
            else:
                print("unknown estimator", estimator)
                sys.exit(42)

        else:
            print("unknown estimator", estimator)
    return res


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



    def writePartialCover(self,rep=Path("."),filenamePrefix="", typeIndicator="standard"):

        for num in range(len(self.covMerged)): #loop over snapshots
            maxIndicator=0
            handler=openGz(rep / ("%scoverMerged%05d"%(filenamePrefix ,num)),"w")
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
            cov=covReader(pid,Path(rep), self.trace_kind)
            cov.writePartialCover(filenamePrefix, pidMap=pidMap)


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


    def writeMergedCov(self,estimator):
        """Write merged Cov with correlation indice  between coverage difference and sucess/failure status"""

        #check the presence of success and failure
        (nbSuccess, nbFail)=self.countStatus()
        print("NbSuccess: %d \t nbFail %d"%(nbSuccess,nbFail))
        if nbFail==0 or nbSuccess==0:
            print("mergeCov need Success/Fail partition")
            sys.exit()

        pidRef,repRef=self.tabPidRep[self.refIndex]
        covMerged= covMerge(covReader(pidRef, Path(repRef), self.trace_kind))
        #Loop with addMerge to reduce memory peak

        printIndex=[int(float(p) * len(self.tabPidRep) /100.)  for p in (list(range(0,100,10))+[1,5])]
        printIndex +=[1,  len(self.tabPidRep)-1]

        for i in range(len(self.tabPidRep)):
            if i==self.refIndex:
                continue
            pid,rep=self.tabPidRep[i]
            covMerged.addMerge(covReader(pid,Path(rep),self.trace_kind), self.getStatus(pid,rep))
            if i in printIndex:
                pourcent=float(i+1)/ float(len(self.tabPidRep)-1)
                if i >=self.refIndex:
                    pourcent=float(i)/ float(len(self.tabPidRep)-1)
                print( "%.1f"%(pourcent*100)    +"% of coverage data merged")
        #covMerged.writePartialCover(typeIndicator="standard")
        covMerged.writePartialCover(typeIndicator=estimator)


    def writeMergedBack(self,estimatorTab , csvFormat=False):
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
        backMerged.writePartialBackCover(outputDir=Path("."), outputTypeTab=estimatorTab, csvFormat=csvFormat)
