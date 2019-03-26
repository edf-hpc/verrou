#!/usr/bin/env python3

import os
import sys
import subprocess as sp
import shlex
import re

stdRounding=["nearest", "toward_zero", "downward", "upward" ]
valgrindRounding=stdRounding + ["random", "average", "float", "farthest", "memcheck"]


def printRes(res):
    print("stdout:")
    for line in res[0 ]:
        print(line[0:-1])
    print("cerr  :")
    for line in res[1 ]:
        print(line[0:-1])


def runCmd(cmd,expectedResult=0, printCmd=True, printCwd=True):    
    if printCmd:
        print("Cmd:", cmd)

    if printCwd:
        print("Cwd:", os.getcwd())
    #lancement de la commande
    
    process=sp.Popen(args=shlex.split(cmd),
                     stdout=sp.PIPE,
                     stderr=sp.PIPE)

    (resStdStr, resErrStr)=process.communicate()

    resStd=resStdStr.decode('utf8').splitlines()
    resErr=resErrStr.decode('utf8').splitlines()
     
    error=process.wait()

    #Traitement des erreurs
    if error !=expectedResult: 
        msg = "Error with the execution of : " +cmd+"\n"
        msg+= "\t error is " +str(error) +"\n"
        msg+= "\t expectedResult is " +str(expectedResult)        
        print(msg)
        printRes((resStd, resErr))

        
        sys.exit(42)
    return (resStd, resErr)


        

class cmdPrepare:
    def __init__(self, arg):
        self.valgrindPath=os.path.join(os.environ["INSTALLPATH"], "bin", "valgrind")
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
                cmd=self.valgrindPath + " --tool=verrou --vr-verbose=no --rounding-mode=" + rounding+ " " +self.execPath +" valgrind"

        return runCmd(cmd)
        # print cmd
            
    def checkRounding(self, env, rounding):
        
        if env=="fenv" and rounding in stdRounding:
            return True
        if env=="valgrind" and rounding in valgrindRounding:
            return True
        print("Failure in checkRounding")
        sys.exit(-1)



def generatePairOfAvailableComputation():
    res=[]
    for i in stdRounding:
        res+=[("fenv", i)]
    for i in valgrindRounding:
        res+=[("valgrind", i)]
    return res
        

def verrouCerrFilter(res):
    pidStr=(res[0].split())[0]
    # pidStr="==2958=="
    newRes=[]
    for line in res:
        newLine=line.replace(pidStr, "")
        if newLine.startswith(" Backend verrou : simulating ") and newLine.endswith(" rounding mode"):
            continue
        if newLine.startswith(" First seed : "):
            continue
        newRes+=[newLine]
    return newRes

def getDiff(outPut, testName):
    for line in outPut[0]:
        if line.startswith(testName+":"):
            return float(line.split()[-1])
    print("unknown testName: ", testName)    
    return None



        
class errorCounter:

    def __init__(self,ok=0,ko=0,warn=0):
        self.ok=ok
        self.ko=ko
        self.warn=warn


    def incOK(self, v):
        self.ok+=v
    def incKO(self, v):
        self.ko+=v
    def incWarn(self, v):
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
        print("error summary")
        print("\tOK : "+str(self.ok))
        print("\tKO : "+str(self.ko))
        print("\tWarning : "+str(self.warn))
        
def checkVerrouInvariant(allResult):
    ref=allResult[("valgrind", "nearest")][1]
    ko=0
    ok=0
    for rounding in valgrindRounding:        
        if rounding in ["nearest", "memcheck"]:
            #nearest : because it is the ref
            #memcheck : because it is not verrou
            continue
        (cout, cerr)=allResult[("valgrind", rounding)]
        if cerr!=ref:
            for i in range(len(ref)):
                if cerr[i]!=ref[i]:
                    print("cerr:", cerr[i])
                    print("ref:", ref[i])
            print("KO : incoherent number of operation ("+rounding+")")
            ko+=1
        else:
            print("OK : coherent number of operation ("+rounding+")")
            ok+=1
    return errorCounter(ok, ko, 0)

def diffRes(res1, res2):    
    if len(res1)!=len(res2):
        print("Wrong number of line")
        print("fenv", res1)
        print("val", res2)
        sys.exit(-1)
    else:
        acc=0
        for i in range(len(res1)):
            line1=res1[i]
            line2=res2[i]
            if  line1 !=line2:
                print("\tfenv: "+line1.strip())
                print("\tfval: "+line2.strip()+"\n")
                acc+=1
        return acc

def checkRoundingInvariant(allResult):
    ok=0
    ko=0
    for rounding in stdRounding:
        fenvRes=(allResult["fenv", rounding])[0]
        valRes=(allResult["valgrind", rounding])[0]
        if fenvRes!=valRes:
            print("KO : incoherent comparison between fenv and valgrind ("+rounding+")")            
            ko+=diffRes(fenvRes, valRes)
        else:
            ok+=1
            print("OK : coherent comparison between fenv and valgrind ("+rounding+")")
    return errorCounter(ok, ko, 0)



# def checkOrder(testName, *args):
#     tabValue=[x[0] for x in args]
#     nameValue=[x[0] for x in args]

#     for i in range(len(tabValue)-1):

class assertRounding:
    def __init__(self, testName):
        self.testName=testName
        self.diff_nearestMemcheck=getDiff(allResult[("valgrind", "memcheck")], testName)
        self.diff_nearestNative=getDiff(allResult[("fenv", "nearest")], testName)
        self.diff_toward_zeroNative  =getDiff(allResult[("fenv", "toward_zero")], testName)
        self.diff_downwardNative     =getDiff(allResult[("fenv", "downward")], testName)
        self.diff_upwardNative       =getDiff(allResult[("fenv", "upward")], testName)
        
        self.diff_nearest      =getDiff(allResult[("valgrind", "nearest")], testName)
        self.diff_toward_zero  =getDiff(allResult[("valgrind", "toward_zero")], testName)
        self.diff_downward     =getDiff(allResult[("valgrind", "downward")], testName)
        self.diff_upward       =getDiff(allResult[("valgrind", "upward")], testName)
        self.diff_float        =getDiff(allResult[("valgrind", "float")], testName)
        self.diff_farthest     =getDiff(allResult[("valgrind", "farthest")], testName)


        self.diff_random       =getDiff(allResult[("valgrind", "random")], testName)      
        self.diff_average      =getDiff(allResult[("valgrind", "average")], testName)     

        self.KoStr="Warning"
        self.warnBool=True
        self.ok=0
        self.warn=0
        self.ko=0

        self.assertEqual("nearestNative", "nearestMemcheck")
        if self.ok!=0:
            self.KoStr="KO"
            self.warnBool=False

    def getValue(self, str1):
        return eval("self.diff_"+str1)

    def printKo(self, str):
        print(self.KoStr+" : "+self.testName+ " "+str)
        if self.warnBool:
            self.warn+=1
        else:
            self.ko+=1
            
    def printOk(self, str):
        print("OK : "+self.testName+ " "+str)
        self.ok+=1

    def assertEqValue(self, str1, value):
        value1=eval("self.diff_"+str1)
        value2=value
        if value1!= value2:
            self.printKo(str1+ "!=" +str(value2) + " "+str(value1))
        else:
            self.printOk(str1+ "="+str(value))

        
    def assertEqual(self, str1, str2):
        value1= eval("self.diff_"+str1)
        value2= eval("self.diff_"+str2)            

        if value1!= value2:
            self.printKo(str1+ "!="+str2 + " "+str(value1) + " " +str(value2))
        else:
            self.printOk(str1+ "="+str2)

    def assertLeq(self, str1, str2):
        value1= eval("self.diff_"+str1)
        value2= eval("self.diff_"+str2)            

        if value1 <= value2:
            self.printOk(str1+ "<="+str2)
        else:
            self.printKo(str1+ ">"+str2 + " "+str(value1) + " " +str(value2))

        
    def assertLess(self, str1, str2):
        value1= eval("self.diff_"+str1)
        value2= eval("self.diff_"+str2)            

        if value1 < value2:
            self.printOk(str1+ "<"+str2)
        else:
            self.printKo(str1+ ">="+str2 +  " "+str(value1) + " " +str(value2))


    def assertAbsLess(self, str1, str2):
        value1= abs(eval("self.diff_"+str1))
        value2= abs(eval("self.diff_"+str2))

        if value1 < value2:
            self.printOk("|"+str1+ "| < |"+str2+"|")
        else:
            self.printKo("|"+str1+ "| >= |"+str2+"|" +  " "+str(value1) + " " +str(value2))


    def assertNative(self):
        for rd in ["nearest", "toward_zero", "downward", "upward"]:
            self.assertEqual(rd, rd+"Native")


    
    
def checkTestPositiveAndOptimistRandomVerrou(allResult,testList,typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
        for RealType in typeTab:
            testName=test+RealType

            testCheck=assertRounding(testName)
            testCheck.assertNative()
            testCheck.assertEqual("toward_zero", "downward")
            testCheck.assertLess("downward", "upward")
            testCheck.assertLeq("downward", "nearest")
            testCheck.assertLeq("downward", "farthest")
            testCheck.assertLeq("farthest", "upward")
            testCheck.assertLeq("nearest", "upward")

            testCheck.assertLess("downward", "random")
            testCheck.assertLess("downward", "average")

            testCheck.assertLess("random", "upward")
            testCheck.assertLess("average", "upward")

            testCheck.assertAbsLess("average", "random")
            testCheck.assertAbsLess("average", "upward")
            testCheck.assertAbsLess("average", "downward")
            testCheck.assertAbsLess("average", "nearest")

            ok+=testCheck.ok
            ko+=testCheck.ko
            warn+=testCheck.warn

    return errorCounter(ok, ko, warn)


def checkFloat(allResult, testList):
    ok=0
    warn=0
    ko=0
    for test in testList:
        testCheckFloat=assertRounding(test+"<float>")
        testCheckDouble=assertRounding(test+"<double>")
        testCheckFloat.assertEqual("nearest", "float")
        testCheckDouble.assertEqValue("float", testCheckFloat.getValue("nearest"))
        ok+=testCheckFloat.ok
        ko+=testCheckFloat.ko
        warn+=testCheckFloat.warn
        ok+=testCheckDouble.ok
        ko+=testCheckDouble.ko
        warn+=testCheckDouble.warn
    return errorCounter(ok, ko, warn)

def checkTestNegativeAndOptimistRandomVerrou(allResult,testList,typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
        for RealType in typeTab:
            testName=test+RealType

            testCheck=assertRounding(testName)
            testCheck.assertNative()
            testCheck.assertEqual("toward_zero", "upward")
            testCheck.assertLess("downward", "upward")
            testCheck.assertLeq("downward", "nearest")
            testCheck.assertLeq("nearest", "upward")

            testCheck.assertLeq("downward", "farthest")
            testCheck.assertLeq("farthest", "upward")

            testCheck.assertLess("downward", "random")
            testCheck.assertLess("downward", "average")
            
            testCheck.assertLess("random", "upward")
            testCheck.assertLess("average", "upward")

            testCheck.assertAbsLess("average", "random")
            testCheck.assertAbsLess("average", "upward")
            testCheck.assertAbsLess("average", "downward")
            testCheck.assertAbsLess("average", "nearest")

            ok+=testCheck.ok
            ko+=testCheck.ko
            warn+=testCheck.warn

    return errorCounter(ok, ko, warn)

def checkTestPositive(allResult,testList, typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
       for RealType in typeTab:
        testName=test+RealType
        testCheck=assertRounding(testName)
        testCheck.assertNative()
        testCheck.assertEqual("toward_zero", "downward")
        testCheck.assertLess("downward", "upward")
        testCheck.assertLeq("downward", "nearest")
        testCheck.assertLeq("nearest", "upward")

        testCheck.assertLeq("downward", "farthest")
        testCheck.assertLeq("farthest", "upward")

        testCheck.assertLess("downward", "random")
        testCheck.assertLess("downward", "average")

        testCheck.assertLess("random", "upward")
        testCheck.assertLess("average", "upward")

        ok+=testCheck.ok
        ko+=testCheck.ko
        warn+=testCheck.warn
        
    return errorCounter(ok, ko, warn)
        
def checkTestNegative(allResult,testList,typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
        for RealType in typeTab:
            testName=test+RealType

            testCheck=assertRounding(testName)
            testCheck.assertNative()
            testCheck.assertEqual("toward_zero", "upward")
            testCheck.assertLess("downward", "upward")
            testCheck.assertLeq("downward", "nearest")
            testCheck.assertLeq("nearest", "upward")

            testCheck.assertLeq("downward", "farthest")
            testCheck.assertLeq("farthest", "upward")

            testCheck.assertLess("downward", "random")
            testCheck.assertLess("downward", "average")

            testCheck.assertLess("random", "upward")
            testCheck.assertLess("average", "upward")

            ok+=testCheck.ok
            ko+=testCheck.ko
            warn+=testCheck.warn

    return errorCounter(ok, ko, warn)

def checkTestPositiveBetweenTwoValues(allResult,testList, typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
       for RealType in typeTab:
        testName=test+RealType
        testCheck=assertRounding(testName)
        testCheck.assertNative()
        testCheck.assertEqual("toward_zero", "downward")
        testCheck.assertLess("downward", "upward")
        testCheck.assertLeq("downward", "nearest")
        testCheck.assertLeq("nearest", "upward")

        testCheck.assertLeq("downward", "farthest")
        testCheck.assertLeq("farthest", "upward")

        testCheck.assertLeq("downward", "random")
        testCheck.assertLeq("downward", "average")

        testCheck.assertLeq("random", "upward")
        testCheck.assertLeq("average", "upward")

        ok+=testCheck.ok
        ko+=testCheck.ko
        warn+=testCheck.warn

    return errorCounter(ok, ko, warn)

def checkTestNegativeBetweenTwoValues(allResult,testList, typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
       for RealType in typeTab:
        testName=test+RealType
        testCheck=assertRounding(testName)
        testCheck.assertNative()
        testCheck.assertEqual("toward_zero", "upward")
        testCheck.assertLess("downward", "upward")
        testCheck.assertLeq("downward", "nearest")
        testCheck.assertLeq("nearest", "upward")

        testCheck.assertLeq("downward", "farthest")
        testCheck.assertLeq("farthest", "upward")

        testCheck.assertLeq("downward", "random")
        testCheck.assertLeq("downward", "average")

        testCheck.assertLeq("random", "upward")
        testCheck.assertLeq("average", "upward")

        ok+=testCheck.ok
        ko+=testCheck.ko
        warn+=testCheck.warn

    return errorCounter(ok, ko, warn)




def checkExact(allResult,testList,typeTab=["<double>", "<float>"]):
    ok=0
    warn=0
    ko=0
    for test in testList:
        for RealType in typeTab:
            testName=test+RealType

            testCheck=assertRounding(testName)
            testCheck.assertNative()
            testCheck.assertEqual("toward_zero", "downward")
            testCheck.assertEqual("downward", "upward")
            testCheck.assertEqual("upward", "nearest")
            testCheck.assertEqual("nearest", "upward")

            testCheck.assertEqual("nearest", "farthest")

            testCheck.assertEqual("downward", "random")
            testCheck.assertEqual("downward", "average")

            ok+=testCheck.ok
            ko+=testCheck.ko
            warn+=testCheck.warn

    return errorCounter(ok, ko, warn)



if __name__=='__main__':
    cmdHandler=cmdPrepare(os.path.join(os.curdir,sys.argv[1]))
    allResult={}
    for (env, rounding) in generatePairOfAvailableComputation():
        (cout, cerr)=cmdHandler.run(env, rounding)
        if env=="valgrind":
            allResult[(env, rounding)]=(cout, verrouCerrFilter(cerr))
        else:
            allResult[(env, rounding)]=(cout, cerr)
            
    # printRes(allResult[("fenv" ,"toward_zero")])
    # printRes(allResult[("valgrind" ,"toward_zero")])
    typeTab=["<double>", "<float>"]#,"<long double>"]

    eCount=errorCounter()

    eCount+=checkVerrouInvariant(allResult)


    eCount+=checkTestPositiveAndOptimistRandomVerrou(allResult, testList=["testInc0d1", "testIncSquare0d1", "testIncDiv10"], typeTab=typeTab)
    eCount+=checkTestNegativeAndOptimistRandomVerrou(allResult, testList=["testInc0d1m", "testIncSquare0d1m", "testIncDiv10m"], typeTab=typeTab)
    eCount+=checkTestPositive(allResult, testList=["testInvariantProdDiv"], typeTab=typeTab)
    eCount+=checkTestNegative(allResult, testList=["testInvariantProdDivm"], typeTab=typeTab)
    eCount+=checkTestPositiveAndOptimistRandomVerrou(allResult, testList=["testFma"], typeTab=["<double>", "<float>"])
    eCount+=checkTestNegativeAndOptimistRandomVerrou(allResult, testList=["testFmam"], typeTab=["<double>", "<float>"])
        
    eCount+=checkExact(allResult, testList=["testMixSseLlo"], typeTab=["<double>", "<float>"])

    eCount+=checkExact(allResult, testList=["testCast", "testCastm"], typeTab=["<double>"])
    eCount+=checkTestPositiveBetweenTwoValues(allResult, testList=["testCast"], typeTab=["<float>"])
    eCount+=checkTestNegativeBetweenTwoValues(allResult, testList=["testCastm"], typeTab=["<float>"])

    eCount+=checkFloat(allResult, ["testInc0d1", "testIncSquare0d1", "testIncDiv10", "testInc0d1m", "testIncSquare0d1m", "testIncDiv10m", "testFma", "testFmam", "testMixSseLlo"])
    eCount.printSummary()
    sys.exit(eCount.ko)
    #[0]
