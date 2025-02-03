#!/usr/bin/env python3


import sys
import os
import pickle
import random

proba=1.
random_shuffle_seed=None
random_seed=""
try:
    proba = float(os.environ["DD_TEST_PROBA"])
except:
    pass

try:
    random_shuffle_seed = os.environ["DD_TEST_SHUFFLE_SEED"]
except:
    pass
try:
    random_seed = os.environ["DD_TEST_SEED"]
except:
    pass



def simulateRandom(config):
    random.seed(random_seed+config)
    if fail!=0:
        if( random.random()<proba):
            return fail
    return 0



class ddConfig:
    """listOf1Failure [(sym, max(0, sym-16), [(line, max(0, line-8)) for line in range(11) ] ) for sym in range(20)]
           sym : int symbol Index : 0,1,....19
           max(0,sym-16) : positif number are active : 17,18,19
           [(line, max(0, line-8)) for line in range(11) ] for each symbol there are 11 lines (0 to 10). Line 9,10 are active.

    listOf2Failures [((0,1), 1, [(0,line, 1,max(0,line-1)) for line in range(4)])  ]
      Couple (0,1) sym is active (,1,)  (Fail if both are present)

      [(0,line, 1, line+1)) for line in range(4)]
       =>
     [(0,0,1,1),(0,1,1,2),(0,2,1,3),(0,3,1,4)]  => [ (sym0, line0, sym1, line1), (sym0, line1, sym1, line2) , (sym0, line2, sym1, line3), (sym0, line3, sym1, line4)]

    """

    def __init__(self,listOf1Failure=[], listOf2Failures=[]):
        self.nbSym=len(listOf1Failure)
        self.listOf1Failure=listOf1Failure
        self.listOf2Failures=listOf2Failures
        self.check1Failure()
        self.check2Failure()

    def check1Failure(self):
        """"check definition of self.listOf1Failure"""
        definedRange=list(set([x[0] for x in self.listOf1Failure]))
        definedRange.sort()
        if definedRange!=list(range(self.nbSym)):
            print("assert check1Failure")
            print("definedRange:", definedRange)
            sys.exit(42)

    def check2Failure(self):
        """"check definition of self.listOf2Failure"""
        tupleList=self.getListOfTupleLine()
        for tuple4 in self.getListOf2FailureLine():
            (s0,line0,s1,line1)=tuple4
            if (not (s0 in range(self.nbSym)))  or (not (s1 in range(self.nbSym))):
                print("assert check2Failure sym : ", tuple4)
                sys.exit(42)
            if (   (not ( (s0,line0) in tupleList))
                or (not ( (s1,line1) in tupleList))):
                print("assert check2Failure line : ", tupleList )
                sys.exit(42)
            if s0 > s1:
                print("assert check2Failure: please order sym")

        tuple2ErrorList=self.getListOf2FailureLine()
        for tuple4 in tuple2ErrorList:
            if tuple4[0:2] not in tupleList:
                print("assert check2Failure : tuple definition inclusion", tuple4[0:2] )
                sys.exit(42)
            if tuple4[2:4] not in tupleList:
                print("assert check2Failure : tuple definition inclusion", tuple4[2:4] )
                sys.exit(42)



    def getListOfTupleLine(self):
        """return the list of tuple line
        ie (sym,line)with sym and line integer indexes """
        # self.listOf1Failure: [(sym, max(0, sym-16), [(line, max(0, line-8)) for line in range(11) ] ) for sym in range(20)]
        res=[]
        for (sym, active, tabLine) in self.listOf1Failure:
            for (line,active) in tabLine:
                if (sym,line) in res:
                    print("assert getListOfTupleLine : double definition" )
                    sys.exit(42)
                res+=[(sym,line)]
        return res


    def getListOf2FailureLine(self):
        """return the list of error of type2 with  tuple line format
        ie (sym0,line0,sym1,line1) with sym? and line? integer indexes """
        res=[]
        for  failure2 in self.listOf2Failures:
            ((sym0,sym1), active, linesTab)=failure2
            if active==0:
                continue
            for tuple4 in linesTab:
                (s0,l0,s1,l1)=tuple4
                if s0!=sym0 or s1!=sym1:
                    print("invalid definition :", failure2 )
                    sys.exit(42)
                if tuple4 in res:
                    print("assert getListOf2FailureLine : double definition" )
                res+=[tuple4]
        return res


    def pickle(self, fileName):
        """To serialize the ddConfig object in the file fileName"""
        fileHandler= open(fileName, "wb")
        pickle.dump(self.listOf1Failure,fileHandler)
        pickle.dump(self.listOf2Failures,fileHandler)


    def unpickle(self, fileName):
        """To deserialize the ddConfig object from the file fileName"""
        fileHandler=open(fileName, "rb")
        self.listOf1Failure=pickle.load(fileHandler)
        self.listOf2Failures=pickle.load(fileHandler)
        self.nbSym=len(self.listOf1Failure)

    def listOfIntSym(self):
        """Return the int list of symbol"""
        return range(self.nbSym)

    def listOfTxtSym(self):
        """Return a fake list of symbol"""
        return [("sym-"+str(i), "fake.so") for i in self.listOfIntSym()]


    def getExcludeIntSymFromExclusionFile(self, excludeFile):
        """ Return the Int Symbol list excluded with excludeFile """
        if excludeFile==None:
            return []
        return [int((line.split()[0]).replace("sym-", "")) for line in ((open(excludeFile.strip(), "r")).readlines()) ]

    def getIncludeIntSymFromExclusionFile(self,excludeFile):
        """ Return the Int Symbol list included defined through the excludeFile"""
        return [i for i in self.listOfIntSym() if i not in self.getExcludeIntSymFromExclusionFile(excludeFile)]


    def listOfTxtLine(self, excludeFile):
        """Generate a fake list of line : it takes into account the excludeFile"""
        listOfSymIncluded=self.getIncludeIntSymFromExclusionFile(excludeFile)
        res=[]
        for (symFailureIndex, failure, listOfLine) in self.listOf1Failure:
            if symFailureIndex in listOfSymIncluded:
                    for (lineIndex, failureLine) in listOfLine:
                            res+=[("sym"+str(symFailureIndex)+".c", lineIndex, "sym-"+str(symFailureIndex))]                
        print("print listOfLine", res)
        return res

    def getIncludedLines(self, sourceFile):
        includedLines=[line.split() for line in  (open(sourceFile.strip(), "r")).readlines()]
        return includedLines


    def statusOfSymConfig(self, config):
        """Return the status of the config"""
        print(config)
        listOfConfigSym=self.getExcludeIntSymFromExclusionFile(config)

        #test single sym
        for sym in self.listOfIntSym():
            if sym not in listOfConfigSym and self.listOf1Failure[sym][1]!=0:
                res=simulateRandom(config)
                if res==1:
                    return 1
        #test couple sym
        for ((sym1,sym2), failure, tab) in self.listOf2Failures:
            if failure==0:
                continue
            if not sym1 in listOfConfigSym and not sym2 in listOfConfigSym:
                res=simulateRandom(config)
                if res==1:
                    return 1
        return 0

    def statusOfSourceConfig(self, configLine):
        print("configLine:", configLine)

        configLineLines=self.getIncludedLines(configLine)
        print("configLineLines:", configLineLines)
        for sym in range(self.nbSym):
            if self.listOf1Failure[sym][1]!=0:
                print("sym:", sym)
                print("listofLineFailure :", self.listOf1Failure[sym][2])

                selectedConfigLines=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym) ]
                print("selectedConfigLines:", selectedConfigLines)
                for (lineFailure, failure) in self.listOf1Failure[sym][2]:
                    if lineFailure in selectedConfigLines and failure :
                        print("line return : ", lineFailure)
                        res=simulateRandom(configLine)
                        if res==1:
                            return 1

        #test couple sym
        for ((sym1,sym2), failure, tab) in self.listOf2Failures:
            print ("sym1 sym2 tab", sym1, sym2, tab)
            if failure==0:
                continue
            selectedConfigLines1=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym1) ]
            selectedConfigLines2=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym2) ]
            print("selectedConfigLines1:", selectedConfigLines1)
            print("selectedConfigLines2:", selectedConfigLines2)
            for (s1, l1, s2,l2) in tab:
                if s1==sym1 and s2==sym2:
                    if l1 in selectedConfigLines1 and l2 in selectedConfigLines2:
                        res=simulateRandom(configLine)
                        if res==1:
                            return 1
        return 0


    def fullListSym(self,loadRes):
        fullList=[int(line.split()[0].replace("sym-",""))  for  line in loadRes["full"]]
        fullList.sort()
        if fullList != [x for x in range(self.nbSym)]:
            print("invalid full perturbation number")
            print("fullList", fullList)
            print("fullList expected", [x for x in range(self.nbSym)])
            return False
        if len(loadRes["noperturbation"])!=0:
            print("invalid no empty noperturbation")
            return False
        return fullList

    def checkDDmaxSymResult(self,loadRes):
        fullList=self.fullListSym(loadRes)
        if fullList==False:
            return False
        ddmaxList=[int(line.split()[0].replace("sym-",""))  for  line in loadRes["ddmax"]]
        ddmaxCmpList=[int(line.split()[0].replace("sym-",""))  for  line in loadRes["ddmaxcmp"]]
        #print("ddmaxList:", ddmaxList)
        #print("ddmaxCmpList:", ddmaxCmpList)

        if len(fullList)!=len(ddmaxCmpList)+len(ddmaxList):
            print("ddmax/ddmaxcmp : invalid size")
            return False

        ddminList1Expected=[sym for sym in range(self.nbSym)  if  self.listOf1Failure[sym][1]!=0]
        checkNb=0
        for ddmin1 in ddminList1Expected:
            if ddmin1 in ddmaxList:
                print("1-min failure:", ddmin1 )
                return False
            if ddmin1 in ddmaxCmpList:
                checkNb+=1
        if checkNb!=len(ddminList1Expected):
            return False
            print("#ddmax wrong check number (after ddmin1):",checkNb)
        ddminList2Expected=[[tup[0][0], tup[0][1]]for tup in self.listOf2Failures if tup[1]!=0]
        for ddmin2 in ddminList2Expected:
            index1,index2=ddmin2
            if (index1 in ddmaxList) and (index2 in ddmaxCmpList):
                checkNb+=1
                continue
            if (index2 in ddmaxList) and (index1 in ddmaxCmpList):
                checkNb+=1
                continue
            print("#ddmin2 in ddmax : invalid partition", ddmin2)
            return False
        if self.nbSym- checkNb !=len(ddmaxList):
            print("#ddmax wrong ddmaxList size")
            return False
        print("ddmax/ddmaxcmp checked")
        return True

    def checkRddminSymResult(self,loadRes):
        fullList=self.fullListSym(loadRes)

        ddminList=[ [int(line.split()[0].replace("sym-",""))
                     for line in ddmin
                     ]
                     for  ddmin  in loadRes["ddmin"]]
        ddminList1=[x[0] for x in ddminList if len(x)==1]
        ddminList1.sort()
        ddminList2=[[min(x), max(x)] for x in ddminList if len(x)==2]
        ddminList2.sort()
        ddminListRes=[x for x in ddminList if not len(x) in [1,2]]
        if len(ddminListRes)!=0:
            print("unexpected ddmin size")
            print("ddminListRes" , ddminListRes)
            return False

        ddminList1Expected=[sym for sym in range(self.nbSym)  if  self.listOf1Failure[sym][1]!=0]
        ddminList1Expected.sort()
        if ddminList1 != ddminList1Expected:
            print("unexpected ddmin of size 1")
            print("ddminList1Expected", ddminList1Expected)
            print("ddminList1", ddminList1)
            return False

        ddminList2Expected=[[tup[0][0], tup[0][1]]for tup in self.listOf2Failures if tup[1]!=0]
        ddmin2error=False
        for x in ddminList2Expected:
            if not x in ddminList2:
                ddmin2error=True
        for x in ddminList2:
            if not x in ddminList2Expected:
                ddmin2error=True
        if ddmin2error:
            print("unexpected ddminList2")
            print("ddminList2Expected", ddminList2Expected)
            print("ddminList2",ddminList2)
            return False

        rddmincmpExpected=[x for x in range(self.nbSym) if not x in ddminList1Expected and not x in [t[0] for t in ddminList2Expected]  and not x in [t[1] for t in ddminList2Expected]] 
        rddmincmp=[int(line.split()[0].replace("sym-",""))  for  line in loadRes["rddmincmp"]]
        rddmincmp.sort()

        if rddmincmp != rddmincmpExpected:
            print("unexpected rddmincmp")
            print("rddmincmpExpected", rddmincmpExpected)
            print("rddmincmp", rddmincmp)
            return False

        return True

    def fullListLine(self,loadRes):
        fullList=[(int(line.split("\t")[2].replace("sym-","")),
                   int(line.split("\t")[1]))
                   for  line in loadRes["full"]]
        fullList.sort()
        fullListExpected=[(symFailureIndex, line[0]) for (symFailureIndex, failure, listOfLine) in self.listOf1Failure for line in listOfLine]
        fullListExpected.sort()

        if fullListExpected!= fullList:
            print("invalid full perturbation number")
            print("fullList", fullList)
            print("fullList expected", fullListExpected)
            return False
        #check empty list
        if len(loadRes["noperturbation"])!=0:
            print("invalid no empty noperturbation")
            return False
        return fullList


    def checkDDmaxLineResult(self,loadRes):
        #check fulllist
        fullList=self.fullListLine(loadRes)
        if fullList==False:
            return False

        ddmaxList=[(int(line.split("\t")[2].replace("sym-","")),
                      int(line.split("\t")[1]))
                     for line in loadRes["ddmax"]]
        ddmaxCmpList=[(int(line.split("\t")[2].replace("sym-","")),
                      int(line.split("\t")[1]))
                     for line in loadRes["ddmaxcmp"]]

        if len(ddmaxList)+len(ddmaxCmpList)!=len(fullList):
            print("ddmax/ddmaxcmp : invalid size")
            return False

        ddminList1Expected=self.ddminLine1_expected()
        for tupleSymLine in ddminList1Expected:
            if not tupleSymLine in ddmaxCmpList:
                return False

        if len(self.getListOf2FailureLine()) + len(ddminList1Expected) != len(ddmaxCmpList):
            print("Wrong ddmaxCmp size")
            return False
        for (s0,line0,s1,line1) in self.getListOf2FailureLine():
            if (s0,line0) in ddmaxList and  (s1,line1) in ddmaxList:
                print("Wrong ddmax separation")
                return False
            if (s0,line0) in ddmaxCmpList and  (s1,line1) in ddmaxCmpList:
                print("Wrong ddmaxCmp separation")
                return False

        return True

    def ddminLine1_expected(self):
        ddminList1Expected=[(sym, lineTuple[0]) for sym in range(self.nbSym)  if  self.listOf1Failure[sym][1]!=0 for lineTuple in self.listOf1Failure[sym][2] if lineTuple[1]!=0]
        ddminList1Expected.sort()
        return ddminList1Expected

    def checkRddminLineResult(self,loadRes):
        #check fulllist
        fullList=self.fullListLine(loadRes)
        if fullList==False:
            return False

        #extract result form loadRes
        ddminList=[ [(int(line.split("\t")[2].replace("sym-","")),
                     int(line.split("\t")[1]))
                     for line in ddmin]
                    for  ddmin  in loadRes["ddmin"]]
        ddminList1=[x[0] for x in ddminList if len(x)==1]
        ddminList1.sort()
        ddminList2=[x for x in ddminList if len(x)==2]
        ddminList2.sort()
        ddminListRes=[x for x in ddminList if not len(x) in [1,2]]

        #check there are only result with length 1 or 2
        if len(ddminListRes)!=0:
            print("unexpected ddmin size")
            print("ddminListRes" , ddminListRes)
            return False

        #check ddmin with length 1
        ddminList1Expected=self.ddminLine1_expected()
        #[(sym, lineTuple[0]) for sym in range(self.nbSym)  if  self.listOf1Failure[sym][1]!=0 for lineTuple in self.listOf1Failure[sym][2] if lineTuple[1]!=0]
        #ddminList1Expected.sort()
        if ddminList1 != ddminList1Expected:
            print("unexpected ddmin of size 1")
            print("ddminList1Expected", ddminList1Expected)
            print("ddminList1", ddminList1)
            return False

        #check ddmin with length 2
        ddminList2ExpectedSym=[[tup[0][0], tup[0][1]]for tup in self.listOf2Failures if tup[1]!=0]
        ddminList2Sym=set([(x[0][0],x[1][0]) for x in ddminList2 ])
        for (x,y) in ddminList2Sym:
            if not [min(x,y), max(x,y)] in ddminList2ExpectedSym:
                print("to much ddmin2 found")
                return False

        for ddmin2ExpectedSym in ddminList2ExpectedSym:
            line0=[[y[1] for y in  x[2]]  for x in self.listOf2Failures if x[1]!=0  and x[0]==(min(ddmin2ExpectedSym), max(ddmin2ExpectedSym)) ][0]
            line1=[[y[3] for y in  x[2]]  for x in self.listOf2Failures if x[1]!=0  and x[0]==(min(ddmin2ExpectedSym), max(ddmin2ExpectedSym)) ][0]
            line0=list(set(line0))
            line1=list(set(line1))

            lineTupleRes=[(ddmin[0][1], ddmin[1][1])
                          for ddmin in ddminList2 if ddmin[0][0]==min(ddmin2ExpectedSym) and  ddmin[1][0]==max(ddmin2ExpectedSym)]

            errorLine=False
            if len(lineTupleRes) != min( len(line0),len(line1)) :
               errorLine=True

            line0Res=[line[0] for line in lineTupleRes]
            line1Res=[line[1] for line in lineTupleRes]

            for i in line0Res:
               if not i in line0:
                   errorLine=True
            for i in line1Res:
               if not i in line1:
                    errorLine=True

            if errorLine:
               print("ddmin2 Error for ", ddmin2ExpectedSym)
               print("lineTupleRes" , lineTupleRes)
               print("line0, line1", line0, line1)
               return False


        #check rddmincmp
        rddmincmpExpected=[x for x in fullListExpected if not x in ddminList1Expected and not x in sum(ddminList2,[])] 

        rddmincmp=[(int(line.split("\t")[2].replace("sym-","")),
                   int(line.split("\t")[1]))
                   for  line in loadRes["rddmincmp"]]
        rddmincmp.sort()

        if rddmincmp != rddmincmpExpected:
            print("unexpected rddmincmp")
            print("rddmincmpExpected", rddmincmpExpected)
            print("rddmincmp", rddmincmp)
            return False

        return True

def generateFakeExclusion(ddCase):
    genExcludeFile=os.environ["VERROU_GEN_EXCLUDE"]
    genExcludeFile=genExcludeFile.replace("%p", "4242")

    f=open(genExcludeFile, "w")
    dataToWrite=ddCase.listOfTxtSym()
    import random
    random.seed(random_shuffle_seed)
    random.shuffle(dataToWrite)
    for (sym, name,) in dataToWrite:
        f.write(sym +"\t" + name+"\n")
    f.close()

def generateFakeSource(ddCase):


    genSourceFile=os.environ["VERROU_GEN_SOURCE"]
    genSourceFile=genSourceFile.replace("%p", "4242")

    excludeFile=None
    try:
        excludeFile= os.environ["VERROU_EXCLUDE"]
    except:
        excludeFile=None
    print('excludeFile:',excludeFile)
    f=open(genSourceFile, "w")
    for (source, line,  symName) in ddCase.listOfTxtLine(excludeFile):
        f.write(source +"\t" + str(line)+"\t"+symName+"\n")

    f.close()


def runRef(dir_path, ddCase):
    print("ref")
    if "dd.sym" in dir_path and not "dd.line" in dir_path:
        generateFakeExclusion(ddCase)
        ddCase.pickle(os.path.join(dir_path,"dd.pickle"))
        return 0
    if "dd.line" in dir_path:
        generateFakeSource(ddCase)
        ddCase.pickle(os.path.join(dir_path,"dd.pickle"))
        return 0


def runNorm(dir_path, ddCase):
    print("norm")
    if "dd.sym" in dir_path and not "dd.line" in dir_path:
        f=open(os.path.join(dir_path , "path_exclude"), "w")
        f.write(os.environ["VERROU_EXCLUDE"]+"\n")
        f.close()
        return 0
    if "dd.line" in dir_path:
        f=open(os.path.join(dir_path,"path_source"), "w")
        f.write(os.environ["VERROU_SOURCE"]+"\n")
        f.close()


if __name__=="__main__":
    ddCase=ddConfig([(sym, max(0, sym-16), [(line, max(0, line-8)) for line in range(11) ] ) for sym in range(20)],
                    [((0,1), 1, [(0,line, 1, line+1) for line in range(4)])  ]
    )
#    ddCase=ddConfig([(0, 0, []),
#                     (1, 1, [(0, 0),(1,1)] )])
    # os.system("sleep 1"); #to fake time
    if "ref" in sys.argv[1]:
        sys.exit(runRef(sys.argv[1], ddCase))
    else:
        sys.exit(runNorm(sys.argv[1], ddCase))
