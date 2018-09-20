#!/usr/bin/env python3


import sys
import os
import pickle

class ddConfig:
    def __init__(self,listOfFailure=[]):
        self.nbSym=len(listOfFailure)
        self.listOfFailure=listOfFailure

    def pickle(self, fileName):
        """To serialize the ddConfig object in the file fileName"""
        pickle.dump(self.listOfFailure, open(fileName, "wb"))

    def unpickle(self, fileName):
        """To deserialize the ddConfig object from the file fileName"""
        self.listOfFailure=pickle.load(open(fileName, "rb"))
        self.nbSym=len(self.listOfFailure)
        
    def listOfIntSym(self):
        """Return the int list of symbol"""
        return range(self.nbSym)
    
    def listOfTxtSym(self):
        """Return a fake list of symbol"""
        return [("sym-"+str(i), "fake.so") for i in self.listOfIntSym()]


    
    def getExcludeIntSymFromExclusionFile(self, excludeFile):
        """ Return the Int Symbol list excluded with excludeFile """
        return [int((line.split()[0]).replace("sym-", "")) for line in ((open(excludeFile.strip(), "r")).readlines()) ]

    def getIncludeIntSymFromExclusionFile(self,excludeFile):
        """ Return the Int Symbol list included defined through the excludeFile"""
        return [i for i in self.listOfIntSym() if i not in self.getExcludeIntSymFromExclusionFile(excludeFile)]

        
    def listOfTxtLine(self, excludeFile):
        """Generate a fake list of line : it takes into account the excludeFile"""
        listOfSymIncluded=self.getIncludeIntSymFromExclusionFile(excludeFile)
        res=[]
        for (symFailureIndex, failure, listOfLine) in self.listOfFailure:
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

        for sym in self.listOfIntSym():
            if sym not in listOfConfigSym and self.listOfFailure[sym][1]!=0:
                return 1
        return 0

    def statusOfSourceConfig(self, configSym, configLine):
        print("configSym:", configSym)
        print("configLine:", configLine)
        listOfSym=self.getExcludeIntSymFromExclusionFile(configSym)
        print("listOfsym: ", listOfSym)
        
        configLineLines=self.getIncludedLines(configLine)
        print("configLineLines:", configLineLines)
        for sym in range(self.nbSym):
            if sym not in listOfSym and self.listOfFailure[sym][1]!=0:
                print("sym:", sym)
                print("listofLineFailure :", self.listOfFailure[sym][2])
                
                
                selectedConfigLines=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym) ]
                print("selectedConfigLines:", selectedConfigLines)
                for (lineFailure, failure) in self.listOfFailure[sym][2]: 
                    if lineFailure in selectedConfigLines and failure :
                        print("line return : ", lineFailure)
                        return 1
                            
        return 0

    
        
        
def generateFakeExclusion(ddCase):
    genExcludeFile=os.environ["VERROU_GEN_EXCLUDE"]
    genExcludeFile=genExcludeFile.replace("%p", "4242")
    
    
    f=open(genExcludeFile, "w")
    for (sym, name,) in ddCase.listOfTxtSym():
        f.write(sym +"\t" + name+"\n")
    f.close()

def generateFakeSource(ddCase):


    genSourceFile=os.environ["VERROU_GEN_SOURCE"]
    genSourceFile=genSourceFile.replace("%p", "4242")

    
    excludeFile= os.environ["VERROU_EXCLUDE"]
    
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
#    ddCase=ddConfig([(sym, max(0, sym-6), [(line, max(0, line-8)) for line in range(11) ] ) for sym in range(10)])
    ddCase=ddConfig([(0, 0, []),
                     (1, 1, [(0, 0),(1,1)] )])
    
    if "ref" in sys.argv[1]:
        sys.exit(runRef(sys.argv[1], ddCase))
    else:
        sys.exit(runNorm(sys.argv[1], ddCase))
    




