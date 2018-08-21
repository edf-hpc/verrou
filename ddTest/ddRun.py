#!/usr/bin/env python


import sys
import os
import pickle

class ddConfig:
    def __init__(self,listOfFailure=[]):
        self.nbSym=len(listOfFailure)
        self.listOfFailure=listOfFailure

    def listOfSym(self):
        return [("sym-"+str(i),"fake.so") for i in range(self.nbSym)]

    def listOfLine(self, excludeFile):
        listOfSymExcluded=[int((line.split()[0]).replace("sym-","")) for line in ((open(excludeFile.strip(),"r")).readlines()) ]
        listOfSymIncluded=[i for i in range(self.nbSym) if i not in listOfSymExcluded]
        res=[]
        for sym in listOfSymIncluded:
            for (symFailureIndex, failure, listOfLine) in self.listOfFailure:
                if failure!=0:
                    for (lineIndex, failureLine) in listOfLine:
                            res+=[("sym"+str(sym)+".c", lineIndex, "sym-"+str(sym))]
                
        print "print listOfLine", res
        return res
        
    def pickle(self, fileName):
        pickle.dump(self.listOfFailure,open(fileName,"w"))
#        f=open(fileName,"w")
#        f.write(str(self.nb)+"\n")
#        f.close()

    def unpickle(self,fileName):
        self.listOfFailure=pickle.load(open(fileName,"r"))
        self.nbSym=len(self.listOfFailure)
#        self.nb=int(f.readline())
#        f.close()

    def statusOfSymConfig(self,config):
        print config
        listOfLine=[int((line.split()[0]).replace("sym-","")) for line in ((open(config.strip(),"r")).readlines()) ]
        print listOfLine

        for line in range(self.nbSym):
            if line not in listOfLine and self.listOfFailure[line][1]!=0:
                return 1
        return 0

    def statusOfSourceConfig(self,configSym, configLine):
        print "configSym:", configSym
        print "configLine:", configLine
        print (open(configSym.strip(),"r")).readlines()
        listOfSym=[int((line.split()[0]).replace("sym-","")) for line in ((open(configSym.strip(),"r")).readlines()) ]
        print listOfSym

        configLineLines=[line.split() for line in  (open(configLine.strip(),"r")).readlines()]
        configLineLines.remove(["__unknown__","0"])
        print "configLineLines:",configLineLines
        for sym in range(self.nbSym):
            if sym not in listOfSym and self.listOfFailure[sym][1]!=0:
                print "sym:", sym
                print "listofLineFailure :", self.listOfFailure[sym][2]
                
                
                selectedConfigLines=[int(line[1]) for line in configLineLines if line[2]=="sym-"+str(sym) ]
                print "selectedConfigLines:", selectedConfigLines
                for (lineFailure, failure) in self.listOfFailure[sym][2]: 
                    if lineFailure in selectedConfigLines and failure :
                        print "line return : ",lineFailure
                        return 1
                
                    

        print "Oups"
        return 0

    
        
        
def generateFakeExclusion(ddCase):
    genExcludeFile=os.environ["VERROU_GEN_EXCLUDE"]
    genExcludeFile=genExcludeFile.replace("%p","4242")
    
    
    f=open(genExcludeFile,"w")
    for (sym,name,) in ddCase.listOfSym():
        f.write(sym +"\t" + name+"\n")
    f.close()

def generateFakeSource(ddCase):


    genSourceFile=os.environ["VERROU_GEN_SOURCE"]
    genSourceFile=genSourceFile.replace("%p","4242")

    
    excludeFile= os.environ["VERROU_EXCLUDE"]
    
    f=open(genSourceFile,"w")
    for (source , line ,  symName) in ddCase.listOfLine(excludeFile):
        f.write(source +"\t" + str(line)+"\t"+symName+"\n")
        
    f.close()


    
def runRef(dir_path, ddCase):
    print "ref"
    if "dd.sym" in dir_path:
        generateFakeExclusion(ddCase)
        ddCase.pickle(dir_path+"/dd.pickle")
        return 0
    if "dd.line" in dir_path:
        generateFakeSource(ddCase)
        ddCase.pickle(dir_path+"/dd.pickle")
        return 0

        
def runNorm(dir_path,ddCase):
    print "norm"
    if "dd.sym" in dir_path:
        f=open(dir_path +"/path_exclude","w")
        f.write(os.environ["VERROU_EXCLUDE"]+"\n")
        f.close()
        return 0
    if "dd.line" in dir_path:
        f=open(dir_path+"/path_source","w")
        f.write(os.environ["VERROU_SOURCE"]+"\n")
        f.close()
        
        
if __name__=="__main__":
    ddCase=ddConfig([(sym, max(0, sym-6),[(line, max(0, line-8)) for line in range(11) ] ) for sym in range(10)])
    if "ref" in sys.argv[1]:
        sys.exit(runRef(sys.argv[1],ddCase))
    else:
        sys.exit(runNorm(sys.argv[1],ddCase))
    




