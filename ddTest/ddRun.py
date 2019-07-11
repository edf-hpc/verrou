#!/usr/bin/env python3


import sys
import os
import pickle
import random

proba=1.
try:
    proba = float(os.environ["DD_TEST_PROBA"])
except:
    pass

def simulateRandom(fail):
    if fail!=0:
        if( random.random()<proba):
            return fail
    return 0



class ddConfig:
    def __init__(self,listOf1Failure=[], listOf2Failures=[]):
        self.nbSym=len(listOf1Failure)
        self.listOf1Failure=listOf1Failure
        self.listOf2Failures=listOf2Failures
        self.check2Failure()

    def check2Failure(self):
        for x in self.listOf2Failures:
            ((sym0,sym1), fail, tab)=x
            if sym0>= self.nbSym or sym1 >= self.nbSym:
                print("failure")
                sys.exit()
            #todo check tab
                #            for (s1, l1, s2, l2) in tab:

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
        includedLines.remove(["__unknown__", "0"])
        return includedLines


    def statusOfSymConfig(self, config):
        """Return the status of the config"""
        print(config)
        listOfConfigSym=self.getExcludeIntSymFromExclusionFile(config)

        #test single sym
        for sym in self.listOfIntSym():
            if sym not in listOfConfigSym and self.listOf1Failure[sym][1]!=0:
                res=simulateRandom(1)
                if res==1:
                    return 1
        #test couple sym
        for ((sym1,sym2), failure, tab) in self.listOf2Failures:
            if failure==0:
                continue
            if not sym1 in listOfConfigSym and not sym2 in listOfConfigSym:
                res=simulateRandom(1)
                if res==1:
                    return 1
        return 0

    def statusOfSourceConfig(self, configLine):
        print("configLine:", configLine)
        listOfSym=[]
        
        configLineLines=self.getIncludedLines(configLine)
        print("configLineLines:", configLineLines)
        for sym in range(self.nbSym):
            if sym not in listOfSym and self.listOf1Failure[sym][1]!=0:
                print("sym:", sym)
                print("listofLineFailure :", self.listOf1Failure[sym][2])
                
                
                selectedConfigLines=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym) ]
                print("selectedConfigLines:", selectedConfigLines)
                for (lineFailure, failure) in self.listOf1Failure[sym][2]:
                    if lineFailure in selectedConfigLines and failure :
                        print("line return : ", lineFailure)
                        return 1

        #test couple sym
        for ((sym1,sym2), failure, tab) in self.listOf2Failures:
            print ("sym1 sym2 tab", sym1, sym2, tab)
            if failure==0:
                continue
            if not sym1 in listOfSym and not sym2 in listOfSym:

                selectedConfigLines1=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym1) ]
                selectedConfigLines2=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym2) ]
                print("selectedConfigLines1:", selectedConfigLines1)
                print("selectedConfigLines2:", selectedConfigLines2)
                for (s1, l1, s2,l2) in tab:
                    if s1==sym1 and s2==sym2:
                        if l1 in selectedConfigLines1 and l2 in selectedConfigLines2:
                            return 1
        return 0

    
        
        
def generateFakeExclusion(ddCase):
    genExcludeFile=os.environ["VERROU_GEN_EXCLUDE"]
    genExcludeFile=genExcludeFile.replace("%p", "4242")
    
    
    f=open(genExcludeFile, "w")
    dataToWrite=ddCase.listOfTxtSym()
    import random
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
                    [((0,1), 1, [(0,line, 1,max(0,line-1)) for line in range(4)])  ]
    )
#    ddCase=ddConfig([(0, 0, []),
#                     (1, 1, [(0, 0),(1,1)] )])
    
    if "ref" in sys.argv[1]:
        sys.exit(runRef(sys.argv[1], ddCase))
    else:
        sys.exit(runNorm(sys.argv[1], ddCase))
    




