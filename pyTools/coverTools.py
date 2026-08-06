
import re
import sys
from operator import itemgetter
from pathlib import Path

from sysTools import runCmdAsync,getResult,openGz
from estimatorTools import computeEstimator

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
        if self.trace_kind=="bb_cover":
            regularExp=re.compile(r"([0-9]+)\|([0-9]+) : (.*) : (\S*) : ([0-9]+) : ([0,1]) : ([0,1])")
        else:
            #[67133584] unamed_filename_verrou	0	F	!
            regularExp=re.compile(r"\[([0-9]+)\]() (\S*)\t([0-9]+)\t([F,I])\t([?,!])")
            #() is there to avoid sym shift in m.groups()
        fileHandler=openGz(fileName,"r")

        line=fileHandler.readline()
        while not line in [None, ''] :
            m=(regularExp.match(line.strip()))
            if m==None :
                print("error read fileName line:",[line])
                sys.exit()
            #addr,index, sym, sourceFile, lineNum, containFloat, containFloatCmp, index=(None,None,None,None,None,None,None,None)
            if self.trace_kind=="bb_cover":
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
        if not trace_kind in ["bb_cover","back_cover"]:
            print('error trace_kind sould be in ["bb_cover","back_cover"]')
            sys.exit(42)

    def bbPrefixName(self):
        if self.trace_kind=="bb_cover":
            return "trace_bb_info.log-"
        if self.trace_kind=="back_cover":
            return "bbAddrInfo-"

    def covPrefixName(self):
        if self.trace_kind=="bb_cover":
            return "trace_bb_cov.log-"
        if self.trace_kind=="back_cover":
            return "backCoverInfo-"

    def backAddrPrefixName(self):
        if self.trace_kind=="back_cover":
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



class addrBackReader:

    def __init__(self,fileName, trace_kind):
        self.trace_kind=trace_kind
        self.read(fileName)

    def read(self,fileName):
        self.data={}
        if not self.trace_kind in ["back_cover"]:
            print("invalid trace kind")
            sys.exit(42)

        fileHandler=openGz(fileName,"r")
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
            print("\twarning: unknown addr: ?(", addr,")")
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


class bbCovReader:
    def __init__(self, pid, rep, status, trace_kind, mergeRoot=False):
        self.pid=pid
        self.rep=rep
        assert(trace_kind in ["bb_cover"])
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
        covFile=openGz(self.rep / self.tName.covName(pid), "r")
        self.dataCov=self.readBBCov(covFile)

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

    def readBBCov(self, cov):
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

            (addrindex,sep, num)=(line).strip().partition(":")
            (addr,sep,index)=addrindex.partition("|")
            if self.mergeIndex!=0:
                dictRes[addr]=(int(num), int(index))
            else:
                dictRes[addr]=[(int(num), int(index))]
        return res

    def structureData(self):
        nbCov=len(self.dataCov)
        self.dataSortCov=[ self.sortCov(self.dataCov[g] )   for  g in range(nbCov)]

    def minCallIndex(self, numIndexTab):
        return min([x for x in numIndexTab if x!=None])

    def sortCov(self, addrToNumIndexTab):
        if self.mergeIndex is None:
            res=[(addrBB, addrToNumIndexTab[addrBB][0], addrToNumIndexTab[addrBB][1]) for addrBB in addrToNumIndexTab ]
        else:
            res=[(addrBB, [x[0] for x in addrToNumIndexTab[addrBB]], self.minCallIndex([x[1] for x in  addrToNumIndexTab[addrBB]]) ) for addrBB in addrToNumIndexTab ]
        res.sort(key=lambda x: (x[2],x[0]))
        return res

    def writeData(self,handler, sortCov, outputTypeTab=["data"]):
        for bbAddr,rawDataTab,minCallIndex in sortCov:
            name=self.bbInfo.compressMarksWithoutSym(bbAddr)
            dataBBStr=None
            if self.mergeIndex is None:
                assert(outputTypeTab==["data"])
                dataBBStr=str(rawDataTab)
            else:
                dataTab=[]
                for outputType in outputTypeTab:
                    if outputType=="data":
                        dataTab+=rawDataTab
                    elif outputType in ["wdc","dc", "wdc-stol", "fdr-stol"]:
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

            fullPathName=Path(outDir) / ("%scoverBB%05d-%s"%(filenamePrefix ,numCov, pidStr))
            handler=openGz(fullPathName,"w")
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
        assert(trace_kind in ["back_cover"])
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
        covFile=openGz(self.rep / self.tName.covName(pid), "r")
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

        if not False in self.statusTab:
            print("Warning: failure expected to provide correlation estimator")

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

            if self.tName.trace_kind=="back_cover":
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
        tree.sort(key=itemgetter("minCallIndex","addr"))
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
            tree+=[{"addrKind":"addrBB", "addr":addrBB, "flatIndex":None, "minCallIndex":minCallI ,"data":dataBB}]
        else:
            lastAddrBack=tabBack[-1]
            remainBack=tabBack[0:-1]
            for subTree in tree:
                if subTree["addrKind"]=="addrBack":
                    if subTree["addr"]==lastAddrBack:
                        self.addTreeNode(subTree["child"], addrBB, remainBack, dataBB)
                        return
            tree+=[{"addrKind":"addrBack", "addr":lastAddrBack , "child":[]}]
            self.addTreeNode(tree[-1]["child"], addrBB, remainBack,dataBB)

    def writeTree(self, handler, tree, deep=0, outputTypeTab=["data"],csvFormat=False):
        for subTree in tree:
            if subTree["addrKind"]=="addrBack":
                addrBack=subTree["addr"]
                deepStr="\t"*deep
                if csvFormat:
                    deepStr= "|"+str(deep)+"\t"
                handler.write(deepStr+self.addrBackInfo.getBackStr(addrBack)+"\n")
                self.writeTree(handler, subTree["child"], deep+1, outputTypeTab=outputTypeTab, csvFormat=csvFormat)
            if subTree["addrKind"]=="addrBB":
                addrBB=subTree["addr"]
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
                        elif outputType in ["wdc","dc", "wdc-stol", "fdr-stol"]:
                            dataTab+=computeEstimator( self.statusTab  , coverTab, [outputType])
                        else:
                            print("unknown outputType", outputType)
                            sys.exit(42)
                    dataBBStr="\t".join([str(x) for x in  dataTab])
                deepStr="\t"*deep
                if csvFormat:
                    deepStr= str(deep)+"\t"

                handler.write(deepStr+ self.bbInfo.compressMarksWithoutSym(addrBB) +"\t"+dataBBStr +"\n" )


    def writePartialCover(self,outputDir=None,filenamePrefix="", pidMap=None, outputTypeTab=["data"]):
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
