#!/usr/bin/python


import sys
import os
import pickle

class ddConfig:
    def __init__(self,listOfFailure=[]):
        self.nb=len(listOfFailure)
        self.listOfFailure=listOfFailure

    def listOfSym(self):
        return [("sym-"+str(i),"fake.so") for i in range(self.nb)]

    def listOfLine(self, excludeFile):
        listOfSymExcluded=[int((line.split()[0]).replace("sym-","")) for line in ((open(excludeFile.strip(),"r")).readlines()) ]
        listOfSymIncluded=[i for i in range(self.nb) if i not in listOfSymExcluded]

        print "print list included", listOfSymIncluded
    
    def pickle(self, fileName):
        pickle.dump(self.listOfFailure,open(fileName,"w"))
#        f=open(fileName,"w")
#        f.write(str(self.nb)+"\n")
#        f.close()

    def unpickle(self,fileName):
        self.listOfFailure=pickle.load(open(fileName,"r"))
        self.nb=len(self.listOfFailure)
#        self.nb=int(f.readline())
#        f.close()

    def statusOfSymConfig(self,config):
        print config
        listOfLine=[int((line.split()[0]).replace("sym-","")) for line in ((open(config.strip(),"r")).readlines()) ]
        print listOfLine

        for line in range(self.nb):
            if line not in listOfLine and self.listOfFailure[line][1]!=0:
                return 1
        return 0

    def statusOfSourceConfig(self,config):
        print (open(config.strip(),"r")).readlines()
        listOfLine=[int((line.split()[0]).replace("sym-","")) for line in ((open(config.strip(),"r")).readlines()) ]
        print listOfLine

        for line in range(self.nb):
            if line not in listOfLine and self.listOfFailure[line][1]!=0:
                return 1
        return 0

    
        
        
def generateFakeExclusion(ddCase):
    genExcludeFile=os.environ["VERROU_GEN_EXCLUDE"]
    genExcludeFile=genExcludeFile.replace("%p","4242")
    
    
    f=open(genExcludeFile,"w")
    for (sym,name) in ddCase.listOfSym():
        f.write(sym +"\t" + name+"\n")
    f.close()

def generateFakeSource(ddCase):
    genSourceFile=os.environ["VERROU_GEN_SOURCE"]
    genSourceFile=genSourceFile.replace("%p","4242")

    excludeFile= os.environ["VERROU_EXCLUDE"]
    
    f=open(genSourceFile,"w")
    for (source , line ,  sym) in ddCase.listOfLine(excludeFile):
        f.write(source +"\t" + str(line)+"\t"+ name+"\n")
        
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
    ddCase=ddConfig([(i, max(0, i-4.)) for i in range(10)])
    if "ref" in sys.argv[1]:
        sys.exit(runRef(sys.argv[1],ddCase))
    else:
        sys.exit(runNorm(sys.argv[1],ddCase))
    




