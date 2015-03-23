#!/usr/bin/python

import os
import sys
import subprocess as sp
import shlex
import re

stdRounding=["nearest", "toward_zero", "downward", "upward" ]
valgrindRounding=stdRounding + ["random","average","memcheck"]


def printRes(res):
    print "stdout:"
    for line in res[0 ]:
        print line[0:-1]
    print "cerr  :"
    for line in res[1 ]:
        print line[0:-1]


def runCmd(cmd,expectedResult=0, printCmd=True, printCwd=True):    
    if printCmd:
        print  "Cmd:", cmd

    if printCwd:
        print "Cwd:", os.getcwd()
    #lancement de la commande

    resStd=[]
    resErr=[]
    
    process=sp.Popen(args=shlex.split(cmd),stdout=sp.PIPE, stderr=sp.PIPE)
        
    fd=process.stdout
    fde=process.stderr

    resStd=fd.readlines()
    resErr=fde.readlines()
    fd.close() 
    fde.close()
     
    error=process.wait()

    #Traitement des erreurs
    if error !=expectedResult: 
        msg = "Error with the execution of : " +cmd+"\n"
        msg+= "\t error is " +str(error) +"\n"
        msg+= "\t expectedResult is " +str(expectedResult)        
        print msg
        printRes((resStd,resErr))

        
        sys.exit(42)
    return (resStd, resErr)


        

class cmdPrepare:
    def __init__(self,arg):
        self.valgrindPath=self.getValgrindPath()
        self.execPath=arg
    
    def run(self,env="fenv", rounding="nearest"):
        self.checkRounding(env, rounding)
        cmd=None
        if env=="fenv":
            cmd=self.execPath + " fenv "+ rounding
            
        if env=="valgrind":
            if rounding=="memcheck":
                cmd=self.valgrindPath + " --tool=memcheck " +self.execPath +" valgrind"
            else:
                cmd=self.valgrindPath + " --tool=verrou --verrou-verbose=no --rounding-mode=" + rounding+ " " +self.execPath +" valgrind"

        return runCmd(cmd)
        # print cmd
            
    def checkRounding(self, env, rounding):
        
        if env=="fenv" and rounding in stdRounding:
            return True
        if env=="valgrind" and rounding in valgrindRounding:
            return True
        print "Failure in checkRounding"
        sys.exit(-1)
        
    
    def getValgrindPath(self):
        (res,err)=runCmd("../../config.status --version", printCwd=False)
        for line in res:
            if "--prefix" in line:
                m = re.search(".*'--prefix=(.*)'.*", line)
                prefix=m.group(1)   
                return os.path.join(prefix, "bin", "valgrind")
        return None



def generatePairOfAvailableComputation():
    res=[]
    for i in stdRounding:
        res+=[("fenv" ,i)]
    for i in valgrindRounding:
        res+=[("valgrind" ,i)]
    return res
        

def verrouCerrFilter(res):
    pidStr=(res[0].split())[0]
    # pidStr="==2958=="
    newRes=[]
    for line in res:
        newLine=line.replace(pidStr,"")
        if newLine.startswith(" Simulating ") and newLine.endswith(" rounding mode\n"):
            continue
        if newLine.startswith(" First seed : "):
            continue
        newRes+=[newLine]
    return newRes

def getDiff(outPut, testName):
    for line in outPut[0]:
        if line.startswith(testName+":"):
            return float(line.split()[-1])
    print "unknown testName: ", testName    
    return None



        
class errorCounter:

    def __init__(self,ok=0,ko=0,warn=0):
        self.ok=ok
        self.ko=ko
        self.warn=warn


    def incOK(self,v):
        self.ok+=v
    def incKO(self,v):
        self.ko+=v
    def incWarn(self,v):
        self.warn+=v
    def add(self, tupleV):
        self.ok  =tupleV[0]
        self.ko  =tupleV[1]
        self.warn=tupleV[2]
    def __add__(self, v):
        self.ok  += v.ok
        self.ko  += v.ko
        self.warn+= v.warn
        return self
    def printSummary(self):
        print "error summary"
        print "\tOK : "+str(self.ok)
        print "\tKO : "+str(self.ko)
        print "\tWarning : "+str(self.warn)
        
def checkVerrouInvariant(allResult):
    ref=allResult[("valgrind" ,"nearest")][1]
    ko=0
    ok=0
    for rounding in valgrindRounding:        
        if rounding in ["nearest","memcheck"]:
            #nearest : because it is the ref
            #memcheck : because it is not verrou
            continue
        (cout,cerr)=allResult[("valgrind" ,rounding)]
        if cerr!=ref:
            print "KO : incoherent number of operation ("+rounding+")"
            ko+=1
        else:
            print "OK : coherent number of operation ("+rounding+")"
            ok+=1
    return errorCounter(ok,ko,0)

def diffRes(res1,res2):    
    if len(res1)!=len(res2):
        print "Wrong number of line"
        print "fenv", res1
        print "val", res2
        sys.exit(-1)
    else:
        acc=0
        for i in xrange(len(res1)):
            line1=res1[i]
            line2=res2[i]
            if  line1 !=line2:
                print "\tfenv: "+line1.strip()
                print "\tfval: "+line2.strip()+"\n"
                acc+=1
        return acc

def checkRoundingInvariant(allResult):
    ok=0
    ko=0
    for rounding in stdRounding:
        fenvRes=(allResult["fenv",rounding])[0]
        valRes=(allResult["valgrind",rounding])[0]
        if fenvRes!=valRes:
            print "KO : incoherent comparison between fenv and valgrind ("+rounding+")"            
            ko+=diffRes(fenvRes,valRes)
        else:
            ok+=1
            print "OK : coherent comparison between fenv and valgrind ("+rounding+")"
    return errorCounter(ok,ko,0)



# def checkOrder(testName, *args):
#     tabValue=[x[0] for x in args]
#     nameValue=[x[0] for x in args]

#     for i in range(len(tabValue)-1):

class assertRounding:
    def __init__(self, testName):
        self.testName=testName
        self.diff_nearestMemcheck=getDiff(allResult[("valgrind" ,"memcheck")],testName)
        self.diff_nearestNative=getDiff(allResult[("fenv" ,"nearest")],testName)
        self.diff_toward_zeroNative  =getDiff(allResult[("fenv" ,"toward_zero")],testName)
        self.diff_downwardNative     =getDiff(allResult[("fenv" ,"downward")],testName)
        self.diff_upwardNative       =getDiff(allResult[("fenv" ,"upward")],testName)
        
        self.diff_nearest      =getDiff(allResult[("valgrind" ,"nearest")],testName)
        self.diff_toward_zero  =getDiff(allResult[("valgrind" ,"toward_zero")],testName)
        self.diff_downward     =getDiff(allResult[("valgrind" ,"downward")],testName)
        self.diff_upward       =getDiff(allResult[("valgrind" ,"upward")],testName)
        self.diff_random       =getDiff(allResult[("valgrind" ,"random")],testName)      
        self.diff_average      =getDiff(allResult[("valgrind" ,"average")],testName)     

        self.KoStr="Warning"
        self.warnBool=True
        self.ok=0
        self.warn=0
        self.ko=0

        self.assertEqual("nearestNative","nearestMemcheck")
        if self.ok!=0:
            self.KoStr="KO"
            self.warnBool=False
        
    def printKo(self,str):
        print self.KoStr+" : "+self.testName+ " "+str
        if self.warnBool:
            self.warn+=1
        else:
            self.ko+=1
            
    def printOk(self,str):
        print "OK : "+self.testName+ " "+str
        self.ok+=1
        
    def assertEqual(self,str1,str2):
        value1= eval("self.diff_"+str1)
        value2= eval("self.diff_"+str2)            

        if value1!= value2:
            self.printKo(str1+ "!="+str2 + " "+str(value1) + " " +str(value2))
        else:
            self.printOk(str1+ "="+str2)

    def assertLeq(self,str1,str2):
        value1= eval("self.diff_"+str1)
        value2= eval("self.diff_"+str2)            

        if value1 <= value2:
            self.printOk(str1+ "<="+str2)
        else:
            self.printKo(str1+ ">"+str2 + " "+str(value1) + " " +str(value2))

        
    def assertLess(self,str1,str2):
        value1= eval("self.diff_"+str1)
        value2= eval("self.diff_"+str2)            

        if value1 < value2:
            self.printOk(str1+ "<"+str2)
        else:
            self.printKo(str1+ ">="+str2 +  " "+str(value1) + " " +str(value2))


    def assertAbsLess(self,str1,str2):
        value1= abs(eval("self.diff_"+str1))
        value2= abs(eval("self.diff_"+str2))

        if value1 < value2:
            self.printOk("|"+str1+ "| < |"+str2+"|")
        else:
            self.printKo("|"+str1+ "| >= |"+str2+"|" +  " "+str(value1) + " " +str(value2))


    def assertNative(self):
        for rd in ["nearest","toward_zero", "downward","upward"]:
            self.assertEqual(rd,rd+"Native")


    
    
def checkTestPositiveAndOptimistRandomVerrou(allResult,testList,typeTab=["<double>","<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
        for RealType in typeTab:
            testName=test+RealType
        
            testCheck=assertRounding(testName)
            testCheck.assertNative()
            testCheck.assertEqual("toward_zero","downward")
            testCheck.assertLess("downward","upward")
            testCheck.assertLeq("downward", "nearest")
            testCheck.assertLeq("nearest", "upward")
            
            testCheck.assertLess("downward", "random")
            testCheck.assertLess("downward", "average")
            
        
            testCheck.assertLess("random","upward")
            testCheck.assertLess("average","upward")
        
            testCheck.assertAbsLess("average","random")
            testCheck.assertAbsLess("average","upward")
            testCheck.assertAbsLess("average","downward")
            testCheck.assertAbsLess("average","nearest")

            ok+=testCheck.ok
            ko+=testCheck.ko
            warn+=testCheck.warn

    return errorCounter(ok,ko,warn)
    


def checkTestPositive(allResult,testList, typeTab=["<double>","<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
       for RealType in typeTab:
        testName=test+RealType
        testCheck=assertRounding(testName)
        testCheck.assertNative()
        testCheck.assertEqual("toward_zero","downward")
        testCheck.assertLess("downward","upward")
        testCheck.assertLeq("downward", "nearest")
        testCheck.assertLeq("nearest", "upward")


        testCheck.assertLess("downward", "random")
        testCheck.assertLess("downward", "average")
        
        testCheck.assertLess("random","upward")
        testCheck.assertLess("average","upward")
        ok+=testCheck.ok
        ko+=testCheck.ko
        warn+=testCheck.warn
        
    return errorCounter(ok,ko,warn)
    
        

if __name__=='__main__':
    cmdHandler=cmdPrepare("./"+sys.argv[1])
    allResult={}
    for (env, rounding) in generatePairOfAvailableComputation():
        (cout,cerr)=cmdHandler.run(env,rounding)
        if env=="valgrind":
            allResult[(env ,rounding)]=(cout, verrouCerrFilter(cerr))
        else:
            allResult[(env ,rounding)]=(cout,cerr)
            
    # printRes(allResult[("fenv" ,"toward_zero")])
    # printRes(allResult[("valgrind" ,"toward_zero")])
    typeTab=["<double>","<float>","<long double>"]

    eCount=errorCounter()

    eCount+=checkVerrouInvariant(allResult)


    eCount+=checkTestPositiveAndOptimistRandomVerrou(allResult, testList=["test1","test2","test3"], typeTab=typeTab)
    eCount+=checkTestPositive(allResult,testList=["test4"], typeTab=typeTab)
    eCount+=checkTestPositiveAndOptimistRandomVerrou(allResult, testList=["test5"], typeTab=["<double>","<float>"])

    eCount.printSummary()
    sys.exit(eCount.ko)
    #[0]
