
# This file is part of Verrou, a FPU instrumentation tool.

# Copyright (C) 2014-2021 EDF
#   F. Févotte <francois.fevotte@edf.fr>
#   B. Lathuilière <bruno.lathuiliere@edf.fr>


# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.

# The GNU Lesser General Public License is contained in the file COPYING.



import sys
import os
import subprocess
import shutil
import hashlib
import copy
import datetime
import math
import threading
import time

import convNumLineTool
import DD

from pathlib import Path

class myTask:
    def __init__(self,cmd,fname,envvars):
        self.cmd=cmd
        self.fname=fname
        self.envvars=envvars
        self.threadid()
    def time(self):
        return time.time()
    def start(self):
        self.startTime=self.time()
    def stop(self):
        self.stopTime=self.time()
    def threadid(self):
        self.tid=threading.get_native_id()

class noLogTask:
    def __init__(self):
        pass
    def initTask(self,cmd,fname,envvars):
        return None
    def closeTask(self, myTask,res):
        pass

class cmdLogTask:
    def __init__(self,fileName):
        self.handler=open(fileName,"w")
    def initTask(self,cmd,fname,envvars):
        task=myTask(cmd,fname,envvars)
        task.start()
        return task
    def closeTask(self,myTask, res):
        myTask.stop()
        self.handler.write("%s\t%s\t%s\t%s%s\n"%(myTask.cmd, str(myTask.startTime), str(myTask.stopTime), str(res), str(myTask.tid)) )

#logTool=cmdLogTask("logFile-%i.txt"%(os.getpid()))
logTool=noLogTask()

def runCmdAsync(cmd, fname, envvars=None):
    """Run CMD, adding ENVVARS to the current environment, and redirecting standard
    and error outputs to FNAME.out and FNAME.err respectively.

    Returns CMD's exit code."""
    if envvars is None:
        envvars = {}

    with open(Path(fname).with_suffix(fname.suffix+".out"), "w") as fout:
        with open(Path(fname).with_suffix(fname.suffix+".err"), "w") as ferr:
            env = copy.deepcopy(os.environ)
            for var in envvars:
                env[var] = str(envvars[var])
            taskLog=logTool.initTask(cmd,fname,envvars)
            return (subprocess.Popen(cmd, env=env, stdout=fout, stderr=ferr), taskLog)

def getResult(subProcessTuple):
    subProcess, taskLog=subProcessTuple
    subProcess.wait()
    logTool.closeTask(taskLog,subProcess.returncode)
    return subProcess.returncode



def runCmd(cmd, fname, envvars=None):
    """Run CMD, adding ENVVARS to the current environment, and redirecting standard
    and error outputs to FNAME.out and FNAME.err respectively.

    Returns CMD's exit code."""
    return getResult(runCmdAsync(cmd,fname,envvars))


subDirRun="dd.run"
fileRun="dd.run"
fileCmp="dd.compare"
subDirRunPattern=subDirRun+"%i"
ddReturnFileName="dd.return.value"

class stochTask:

    def __init__(self, dirname, refDir,runCmd, cmpCmd, runEnv, seedTab=None, seedEnvVar=None):
        self.dirname=Path(dirname)
        self.refDir=refDir
        self.pathToPrint=str((self.dirname.absolute()).relative_to(Path.cwd()))

        self.runCmd=runCmd
        self.cmpCmd=cmpCmd
        self.nbRun=None #useful when used with  runMultipleStochTask

        self.FAIL=DD.DD.FAIL #constant variable
        self.PASS=DD.DD.PASS

        self.runEnv=runEnv

        self.preRunLambda=None # lambda executed before runCmd
        self.postRunLambda=None # lambda executed after runCmd before cmpCmd

        self.seedTab=seedTab
        self.seedEnvVar_=seedEnvVar


    def setPostRun(self, postLambda):
        self.postRunLambda=postLambda

    def setPreRun(self, preLambda):
        self.preRunLambda=preLambda

    def setNbRun(self, nbRun):
        self.nbRun=nbRun

    def _nameDir(self,i,relative=False):
        if relative:
            return subDirRunPattern% (i)
        else:
            return self.dirname / (subDirRunPattern % (i))

    def _replacePattern(self, value, i):
        return str(value).replace("%DDRUN%", self._nameDir(i,True))

    def runOneSample(self, i):
        rundir= self._nameDir(i)
        rundir.mkdir(exist_ok=True)

        env={key:self._replacePattern(self.runEnv[key],i) for key in self.runEnv}
        if self.seedTab!=None:
            env[self.seedEnvVar_]=str(self.seedTab[i])
        if self.preRunLambda!=None:
            self.preRunLambda(rundir, env)
        subProcessRun=runCmdAsync([self.runCmd, rundir],
                                  rundir / fileRun,
                                  env)
        getResult(subProcessRun)
        if self.postRunLambda!=None:
            self.postRunLambda(rundir)
        return self.cmpOneSample(i)

    def cmpOneSample(self,i):
        if self.refDir==None: #if there are no reference provided cmp is ignored
            return self.PASS

        rundir= self._nameDir(i)
        retval = runCmd([self.cmpCmd, self.refDir, rundir],
                        rundir / "dd.compare")

        with open(rundir / ddReturnFileName,"w") as f:
            f.write(str(retval))
        if retval != 0:
            return self.FAIL
        else:
            return self.PASS

    def sampleToCompute(self, nbRun, earlyExit):
        """Return a dict with 4 lists:
    - indexesInCache : indexes of samples in cache (run and cmp)
    - indexesToCmp : indexes of samples which only require cmp
    - indexesToRun : indexes of samples which require run and cmp
    - failedIndexes : indexes of failed sample in cache (subset of indexesInCache)
        return None if earlyExit==True and it exists FAIL sample in cache."""

        listOfDirString=[runDir.name for runDir in self.dirname.glob(subDirRun+"*")]
        cmpDone=[]
        runDone=[]
        failureIndex=[]
        for runDir in listOfDirString:
            returnValuePath=self.dirname / runDir / ddReturnFileName
            ddRunIndex=int(runDir.replace(subDirRun,""))
            if returnValuePath.is_file():
                statusCmp=int((open(returnValuePath).readline()))
                if statusCmp!=0:
                    if earlyExit:
                        return None
                    else:
                        failureIndex+=[ddRunIndex]
                cmpDone+=[ddRunIndex]
            else:
                runPath=self.dirname / runDir / (fileRun+".out")
                if runPath.is_file():
                    runDone+=[ddRunIndex]

        indexesToRun= [x for x in range(nbRun) if (not x in runDone+cmpDone) ]
        return {"indexesToCmp":runDone, "indexesToRun":indexesToRun, "indexesInCache":cmpDone, "failedIndexes":failureIndex}



    def submitSeq(self, cmpOrRun, sampleIndex):
        if cmpOrRun=="cmp":
            return self.cmpOneSample(sampleIndex)
        if cmpOrRun=="run":
            return self.runOneSample(sampleIndex)


def getEstimatedFailProbability(dirname):
    """Return an estimated probablity of fail for the configuration"""
    returnValuePathTab=dirname.glob(subDirRun+ "*/"+ddReturnFileName)
    cacheCounter=0.
    cacheFail=0.
    for returnValuePath in returnValuePathTab:
        cacheCounter+=1.
        statusCmp=int((open(returnValuePath).readline()))
        if statusCmp!=0:
            cacheFail+=1.
    return cacheFail / cacheCounter


def runMultipleStochTask(stochTaskTab, maxNbPROC):
    maxProc=maxNbPROC
    if maxNbPROC==None:
        maxProc=1
    import concurrent.futures
    executor=concurrent.futures.ThreadPoolExecutor(max_workers=maxProc)

    futureTab=[]
    runToDoTab=[]
    for stochTask in stochTaskTab:
        sToC=stochTask.sampleToCompute(stochTask.nbRun, False)
        runToDoTab+=[sToC["indexesToRun"]]
        future=[executor.submit(stochTask.submitSeq, "run", work) for work in sToC["indexesToRun"]]
        futureTab+=[future]

    returnTab=[]
    for i in range(len(stochTaskTab)):
        stochTask=stochTaskTab[i]
        FAIL=stochTask.FAIL
        runToDo=runToDoTab[i]
        results=[futur.result() for futur in futureTab[i]]
        if FAIL in results:
            indices=[indice for indice in range(len(results)) if results[indice]==FAIL]
            failIndices=[runToDoTab[i][indice] for indice in indices ]
            failIndices.sort()
            failIndicesStr=(str(failIndices)[1:-1]).replace(" ","")
            failStr="FAIL(%s)"%(failIndicesStr)
            print(stochTask.pathToPrint + " --(/run/) -> " +failStr)
            returnTab+=[FAIL]
        else:
            passStr="PASS(+" + str(len(runToDo))+"->"+str(stochTask.nbRun)+")"
            print(stochTask.pathToPrint + " --(/run/) -> " +passStr)
            returnTab+=[stochTask.PASS]
    return returnTab

def md5Name(deltas):
    copyDeltas=copy.copy(deltas)
    copyDeltas.sort()
    return hashlib.md5(("".join(["\n"+x for x in copyDeltas])).encode('utf-8')).hexdigest()


def prepareOutput(dirname):
     shutil.rmtree(dirname, ignore_errors=True)
     dirname.mkdir()


def failure():
    sys.exit(42)





class DDStoch(DD.DD):
    def __init__(self, config,
                 selectBlocAndNumLine=lambda x: (x,0), joinBlocAndNumLine= lambda x,y: x,
                 parseRef=None):
        DD.DD.__init__(self)
        self.config_=config
        if not  self.config_.get_quiet():
            print("delta debug options :")
            print(self.config_.optionToStr())

        self.run_ =  self.config_.get_runScript()
        self.compare_ = self.config_.get_cmpScript()

        self.rddminIndex=0 #rddmin index
        self.prefix_ = Path.cwd() / self.config_.get_cacheRep()
        self.relPrefix_=self.config_.get_cacheRep()
        self.ref_ = self.prefix_ / "ref"
        self.prepareCache()
        prepareOutput(self.ref_)

        self.config_.saveParam(self.prefix_ / "cmd.last")

        self.reference() #generate the reference computation
        self.mergeList(parseRef) #generate the search space
        self.rddminHeuristicLoadRep(selectBlocAndNumLine, joinBlocAndNumLine) # at the end because need the search space

        self.initSeed()

    def internalError(self,msg1, msg2):
        print(msg1,msg2)
        import traceback
        traceback.print_stack()

    def initSeed(self):
        if self.config_.ddSeed!=None:
            import random
            random.seed(self.config_.ddSeed)
            self.seedTab=random.sample(range(0,2**32-1), self.config_.get_nbRUN())
        else:
            self.seedTab=None

    def symlink(self,src, dst):
        """Create a relative symlink"""
        if dst.is_symlink():
            dst.unlink()
        relSrc=src.relative_to(self.prefix_)
        relDist=dst.relative_to(self.prefix_)
        relPrefix=self.prefix_.relative_to(Path.cwd())
        os.symlink(relSrc, relPrefix/ relDist)


    def cleanSymLink(self):
        """Delete all symlink in the cache"""
        self.saveCleabSymLink=[]
        symLinkTab=self.searchSymLink()
        for symLink in symLinkTab:
            symLink.unlink()
            if self.config_.get_cache=="continue":
                self.saveCleanSymLink+=[symLink]


    def searchSymLink(self):
        """Return the list of symlink (created by DD_stoch) in the cache"""
        res = list(self.prefix_.glob("ddmin*"))
        res +=[self.prefix_ / sym for sym in ["rddmin-cmp","ddmax","ddmax-cmp","FullPerturbation","NoPerturbation"]]
        return [ x for x in res if x.is_symlink()]


    def rddminHeuristicLoadRep(self, selectBlocAndNumLine, joinBlocAndNumLine):
        """ Load the results of previous ddmin. Need to be called after prepareCache"""

        self.useRddminHeuristic=False
        if self.config_.get_rddminHeuristicsCache() !="none" or len(self.config_.get_rddminHeuristicsRep_Tab())!=0 or len(self.config_.get_rddminHeuristicsFile_Tab())!=0:
            self.useRddminHeuristic=True
        if self.useRddminHeuristic==False:
            return

        rddmin_heuristic_rep=[]
        if "cache" in self.config_.get_rddminHeuristicsCache():
            if self.config_.get_cache()=="rename":
                if self.oldCacheName!=None:
                    cacheRep=Path(self.oldCacheName)
                    if cacheRep.is_dir():
                        rddmin_heuristic_rep+=[cacheRep]
            else:
                if self.prefix_.is_dir():
                    rddmin_heuristic_rep+=[self.prefix_]

        if self.config_.get_rddminHeuristicsCache()=="all_cache":
            rddmin_heuristic_rep+=(self.prefix_ / "..").glob(str(self.relPrefix_)+"*-*-*-*_*h*m*s")

        for rep in self.config_.get_rddminHeuristicsRep_Tab():
            if rep not in rddmin_heuristic_rep:
                rddmin_heuristic_rep+=[rep]

        self.ddminHeuristic=[]

        for deltaFile in self.config_.get_rddminHeuristicsFile_Tab():
            self.ddminHeuristic+=[ self.loadDeltaFile(deltaFile)]

        if self.config_.get_cache=="continue":
            self.ddminHeuristic+=[ self.loadDeltaFileFromRep(rep)  for rep in self.saveCleanSymLink if "ddmin" in rep]

        if self.config_.get_rddminHeuristicsLineConv():
            for rep in rddmin_heuristic_rep:
                deltaOld=self.loadDeltaFileFromRep(rep / "ref", True)
                if deltaOld==None:
                    continue
                cvTool=convNumLineTool.convNumLineTool(deltaOld, self.getDelta0(), selectBlocAndNumLine, joinBlocAndNumLine)
                repTab=rep.glob("ddmin*")

                for repDDmin in repTab:
                    deltas=self.loadDeltaFileFromRep(repDDmin)
                    if deltas==None:
                        continue
                    deltasNew=[]
                    for delta in deltas:
                        deltasNew+= cvTool.getNewLines(delta)
                    self.ddminHeuristic+=[deltasNew]
        else:
            for rep in rddmin_heuristic_rep:
                repTab=rep.glob("ddmin*")
                for repDDmin in repTab:
                    deltas=self.loadDeltaFileFromRep(repDDmin)
                    if deltas==None:
                        continue
                    self.ddminHeuristic+=[deltas]

    def loadDeltaFileFromRep(self,rep, ref=False):
        fileName=rep /  (self.getDeltaFileName()+".include")
        if ref:
            fileName=rep / self.getDeltaFileName()
        return self.loadDeltaFile(fileName)

    def loadDeltaFile(self,fileName):
        if fileName.is_file():
            deltasTab=[ x.rstrip() for x in (open(fileName)).readlines()]
            return deltasTab
        else:
            print(fileName + " do not exist")
        return None


    def prepareCache(self):
        cache=self.config_.get_cache()
        if cache=="continue":
            self.prefix_.mkdir(exist_ok=True)
            self.cleanSymLink()
            return
        if cache=="clean":
            shutil.rmtree(self.prefix_, ignore_errors=True)
            self.prefix_.mkdir()
            return

        if cache=="rename_keep_result":
            #delete unusefull rep : rename treated later
            symLinkTab=self.searchSymLink()
            repToKeep=[os.readlink(x) for x in symLinkTab]
            print(repToKeep)
            #loop over rep with md5 format
            for md5rep in [x for x in self.prefix.glob("[a-f0-9]"*32) if x.is_dir()]:
                if not md5rep.name in repToKeep:
                    shutil.rmtree(md5rep)

        if cache.startswith("rename"):
            if self.prefix_.is_dir():
                symLinkTab=self.searchSymLink()
                if symLinkTab==[]:  #find alternative file to get time stamp
                    refPath= self.prefix_ / "ref"
                    if refPath.is_dir():
                        symLinkTab=[refPath]
                    else:
                        symLinkTab=[self.prefix_]
                timeStr=datetime.datetime.fromtimestamp(max([x.stat().st_mtime for x in symLinkTab])).strftime("-%m-%d-%Y_%Hh%Mm%Ss")
                self.oldCacheName= self.prefix_.with_name(self.prefix_.name+timeStr)
                self.prefix_.rename(self.oldCacheName )
            else:
                self.oldCacheName=None
            self.prefix_.mkdir()


        if cache=="keep_run":
            if not self.prefix_.is_dir():
                self.prefix_.mkdir()
            else:
                self.cleanSymLink()
                filesToDelete = list(self.prefix_.glob("*/"+subDirRun+"[0-9]*/"+fileCmp+".*"))
                filesToDelete +=list(self.prefix_.glob("*/"+subDirRun+"[0-9]*/"+ddReturnFileName))
                for fileToDelete in filesToDelete:
                    fileToDelete.unlink()

    def reference(self):
        """Run the reference and check the result"""
        print(self.ref_.relative_to(Path.cwd()),end="")
        print(" -- (run) -> ",end="",flush=True)
        retval = runCmd([self.run_, self.ref_],
                        self.ref_ /"dd",
                        self.referenceRunEnv())
        if retval!=0:
            print("")
            self.referenceRunFailure()
        else:
            """Check the comparison between the reference and refrence is valid"""
            retval = runCmd([self.compare_,self.ref_, self.ref_],
                            self.ref_ / "checkRef")
            if retval != 0:
                print("FAIL")
                self.referenceFailsFailure()
            else:
                print("PASS")


    def mergeList(self, parseRef):
        """merge the file name.$PID into a uniq file called name """
        dirname=self.ref_
        name=self.getDeltaFileName()

        listOfExcludeFile=[ x.name for x in dirname.iterdir() if self.isFileValidToMerge(x.name) ]
        if len(listOfExcludeFile)<1:
            self.searchSpaceGenerationFailure()

        excludeMerged=[]
        for excludeFile in listOfExcludeFile:
            lines=None
            if parseRef==None:
                with open(dirname / excludeFile, "r") as f:
                    lines=[x.rstrip() for x in f.readlines()]
            else:
                lines=parseRef(dirname / excludeFile)

            for line in lines:
                if line not in excludeMerged:
                    excludeMerged+=[line]

        with open(dirname / name, "w" )as f:
            for line in excludeMerged:
                f.write(line+"\n")


    def testWithLink(self, deltas, linkname, earlyExit):
        testResult=self._test(deltas, self.config_.get_nbRUN() , earlyExit)
        dirname = self.prefix_ / md5Name(deltas)
        self.symlink(dirname, self.prefix_ / linkname)
        return testResult

    def report_progress(self, c, title):
        if not self.config_.get_quiet:
            super().report_progress(c,title)

    def configuration_found(self, kind_str, delta_config,verbose=True):
        if verbose:
            print("%s (%s):"%(kind_str,self.coerce(delta_config)))
        earlyExit= not self.config_.resWithAllSamples
        self.testWithLink(delta_config, kind_str, earlyExit)

    def run(self, deltas=None):
        # get the search space
        if deltas==None:
            deltas=self.getDelta0()

        if(len(deltas)==0):
            self.emptySearchSpaceFailure()

        #basic verification
        testResultTab=self._testTab([deltas,[]],2*[self.config_.get_nbRUN()], earlyExit=True, firstConfFail=False, firstConfPass=False, sortOrder="outerSampleInnerConf")

        testResult=testResultTab[0]
        self.configuration_found("FullPerturbation",deltas)
        if testResult!=self.FAIL:
            self.fullPerturbationSucceedsFailure()

        testResult=testResultTab[1]
        self.configuration_found("NoPerturbation",[])
        if testResult!=self.PASS:
            self.noPerturbationFailsFailure()

        if(len(deltas)==1):
            self.configuration_found("ddmin0",deltas)
            self.configuration_found("rddmin-cmp",[])
            return deltas

        #select the right variant of algo and apply it
        algo=self.config_.get_ddAlgo()
        resConf=None
        if algo=="ddmax":
            return  self.DDMax(deltas, self.config_.get_nbRUN())

        def rddminAlgo(localDeltas):
            if algo=="rddmin":
                localConf = self.RDDMin(localDeltas, self.config_.get_nbRUN())
            if algo.startswith("srddmin"):
                localConf= self.SRDDMin(localDeltas, self.config_.get_rddMinTab())
            if algo.startswith("drddmin"):
                localConf = self.DRDDMin(localDeltas,
                                         self.config_.get_rddMinTab(),
                                         self.config_.get_splitTab(),
                                         self.config_.get_splitGranularity())
            return localConf

        if self.useRddminHeuristic and "rddmin" in algo:
            resConf=self.applyRddminWithHeuristics(deltas,rddminAlgo)
        else:
            resConf=rddminAlgo(deltas)

        if resConf!=None:
            flatRes=[c  for conf in resConf for c in conf]
            rddminCmp=self.reduceSearchSpace(deltas,flatRes)
            self.configuration_found("rddmin-cmp", rddminCmp)

            print("RDDMIN summary:")
            summaryHandler=open(self.config_.get_cacheRep() / "rddmin_summary","w")
            for ddminIndex in range(len(resConf)):
                confDirName=self.config_.get_cacheRep() / ("ddmin%i"%(ddminIndex))
                listOfDirString=[runDirPath.name for runDirPath in confDirName.glob(subDirRun+"[0-9]*")]
                failureIndex=[]

                for runDir in listOfDirString:
                    returnValuePath= confDirName / runDir / ddReturnFileName
                    ddRunIndex=int(runDir.replace(subDirRun,""))
                    if returnValuePath.is_file():
                        statusCmp=int((open(returnValuePath).readline()))
                        if statusCmp!=0:
                            failureIndex+=[ddRunIndex]
                    else:
                        failureIndex+=[ddRunIndex]
                failureIndex.sort()
                failStatusStr="Fail Ratio: %.2f%%\tFail indexes: %s"%(100. *float(len(failureIndex)) / float(len(listOfDirString)),
                                                                    ",".join([str(fail) for fail in failureIndex]))
                confSummaryStr="%s:\t%s\n%s\n"%("ddmin%i"%(ddminIndex),
                                                failStatusStr,
                                                "".join(["\t"+line.strip()+"\n" for line in (self.coerce(resConf[ddminIndex]).strip()).split("\n") ]))
                print(confSummaryStr,end="")
                summaryHandler.write(confSummaryStr)

        return resConf


    def reduceSearchSpace(self, deltas, conf):
        "return the set deltas -conf"
        return [delta for delta in deltas if delta not in conf] #reduce search space

    def applyRddminWithHeuristics(self,deltas, algo):
        """Test the previous ddmin configuration (previous run of DD_stoch) as a filter to rddmin algo"""

        nablaRddmin=[]
        deltasCurrent=deltas
        self.ddminHeuristic.sort(key=lambda x: len(x))#sort to test first small heuritics
        for heuristicsDelta in self.ddminHeuristic:
            if all(x in deltasCurrent for x in  heuristicsDelta): #check inclusion
                testResult=self._test(heuristicsDelta, self.config_.get_nbRUN())
                if testResult!=self.FAIL:
                    if not self.config_.get_quiet():
                        print("Bad rddmin heuristic : %s"%self.coerce(heuristicsDelta))
                else:
                    if not self.config_.get_quiet():
                        print("Good rddmin heuristics : %s"%self.coerce(heuristicsDelta))
                    if len(heuristicsDelta)==1:
                        nablaRddmin.append(heuristicsDelta)
                        self.configuration_found("ddmin%d"%(self.rddminIndex), heuristicsDelta)
                        self.rddminIndex+=1
                        deltasCurrent=self.reduceSearchSpace(deltasCurrent,heuristicsDelta)
                    else:
                        resTab= self.check1Min(heuristicsDelta, self.config_.get_nbRUN(),algo)
                        for resMin in resTab:
                            nablaRddmin.append(resMin)
                            deltasCurrent=self.reduceSearchSpace(deltasCurrent,resMin)

        print("Heuristics applied")
        #after the heuristic filter a classic (s)rddmin is applied
        testResult=self._test(deltasCurrent, self.config_.get_nbRUN())
        if testResult!=self.FAIL:
            return nablaRddmin
        else:
            return nablaRddmin+algo(deltasCurrent)


    def DDMax(self, deltas, nbRun):
        ddmax=self.dd_max(deltas, nbRun)
        ddmaxCmp=self.reduceSearchSpace(deltas, ddmax)
        self.configuration_found("ddmax", ddmaxCmp)
        self.configuration_found("ddmax-cmp", ddmax)
        return ddmaxCmp

    def RDDMin(self, deltas, nbRun):
        nablaRddmin=[]
        testResult=self._test(deltas)
        if testResult!=self.FAIL:
            self.internalError("RDDMIN", md5Name(deltas)+" should fail")

        deltasCurrent=deltas
        while testResult==self.FAIL:
            (ddmin, candidat) = self.dd_min(deltasCurrent,nbRun) #candidat is unused

            nablaRddmin.append(ddmin)
            self.configuration_found("ddmin%d"%(self.rddminIndex), ddmin)
            self.rddminIndex+=1

            deltasCurrent=self.reduceSearchSpace(deltasCurrent, ddmin)
            testResult=self._test(deltasCurrent,nbRun)

        return nablaRddmin

    def check1Min(self, deltasHeuristic, nbRun, algoRddmin):
        """check if deltas in 1 minimal : return [deltasHeuristic] if deltasHeuristic is 1-min else
        the result of algoRddmin applied on a failing subspace"""

        testResult=self._test(deltasHeuristic)
        if testResult!=self.FAIL:
            self.internalError("Check1-MIN", md5Name(deltasHeuristic)+" should fail")

        deltaMin1Tab=[]
        for delta1 in deltasHeuristic:
            newDelta=[delta for delta in deltasHeuristic if delta!=delta1]
            deltaMin1Tab+=[newDelta]

        resultTab=self._testTab(deltaMin1Tab, [nbRun]*len(deltaMin1Tab), earlyExit=True, firstConfFail=True)

        for indexRes in range(len(resultTab)):
            result=resultTab[indexRes]
            if result==self.FAIL:
                if not self.config_.get_quiet():
                    print("Heuristics not 1-Minimal")
                return algoRddmin(deltaMin1Tab[indexRes], nbRun)

        self.configuration_found("ddmin%d"%(self.rddminIndex), deltasHeuristic)
        self.rddminIndex+=1
        return [deltasHeuristic]


    def splitDeltas(self, deltas, nbRun, granularity):
        """ Return a list of failing non overlapping subspace"""
        if self._test(deltas, self.config_.get_nbRUN())==self.PASS:
            return [] #short exit

        nablaRes=[] #result : set of smallest (each subset with repect with granularity lead to success)
        nablaCurrent=[deltas]

        nbPara=1
        nbProc=self.config_.get_maxNbPROC()
        if not nbProc in [None,1]:
            nbPara=math.ceil( nbProc/granularity)

        while len(nablaCurrent)>0:
            nablaBloc=nablaCurrent[0:nbPara]
            ciTab=[self.split(candidat, min(granularity, len(candidat))) for candidat in nablaBloc]

            flatciTab=sum(ciTab,[]) #flat ciTab to be compatible with testTab
            flatResTab=self._testTab(flatciTab, [nbRun]* len(flatciTab),earlyExit=False)
            #unflat flatResTab
            resTab=[]
            lBegin=0
            for i in range(len(ciTab)):
                lEnd=lBegin+len(ciTab[i])
                resTab+=[flatResTab[lBegin: lEnd]]
                lBegin=lEnd

            nablaBlocCurrent=[]
            for i in range(len(ciTab)):
                ci=ciTab[i]
                subSetFailed=False
                for j in range(len(ci)):
                    deltaij=ci[j]

                    if resTab[i][j]==self.FAIL:
                        subSetFailed=True
                        if len(deltaij)==1:
                            self.configuration_found("ddmin%d"%(self.rddminIndex), deltaij)
                            self.rddminIndex+=1
                            nablaRes.append(deltaij)
                        else:
                            nablaBlocCurrent.append(deltaij)
                if not subSetFailed:
                    if len(nablaBloc[i])==1:
                        self.configuration_found("ddmin%d"%(self.rddminIndex), nablaBloc[i])
                        self.rddminIndex+=1
                    nablaRes.append(nablaBloc[i])

            nablaCurrent=nablaBlocCurrent+nablaCurrent[nbPara:]
        return nablaRes



    def SsplitDeltas(self, deltas, runTab, granularity):#runTab=splitTab ,granularity=2):
        #apply splitDeltas recussivly with increasing sample number (runTab)
        #remark: the remain treatment do not respect the binary split structure

        #name for progression
        algo_name="ssplitDelta"

        currentSplit=[deltas]
        for run in runTab:
            nextCurrent=[]
            for candidat in currentSplit:
                if len(candidat)==1:
                    nextCurrent.append(candidat)
                    continue
                self.report_progress(candidat,algo_name)
                res=self.splitDeltas(candidat,run, granularity)
                nextCurrent.extend(res)

            #the remainDeltas in recomputed from the wall list (indeed the set can increase with the apply )
            flatNextCurrent=[flatItem  for nextCurrentItem in nextCurrent for flatItem in nextCurrentItem]
            remainDeltas=self.reduceSearchSpace(deltas,flatNextCurrent)

            #apply split to remainDeltas
            self.report_progress(remainDeltas,algo_name)
            nextCurrent.extend(self.splitDeltas(remainDeltas, run, granularity))

            currentSplit=nextCurrent

        return currentSplit

    def DRDDMin(self, deltas, SrunTab, dicRunTab, granularity):#SrunTab=rddMinTab, dicRunTab=splitTab, granularity=2):
        algo_name="DRDDMin"

        #assert with the right nbRun number
        nbRun=SrunTab[-1]
        testResult=self._test(deltas,nbRun)
        if testResult!=self.FAIL:
            self.internalError("DRDDMIN", md5Name(deltas)+" should fail")

        #apply dichotomy
        candidats=self.SsplitDeltas(deltas,dicRunTab, granularity)
        candidats.sort(key=lambda x: len(x))#sort to test first small candidats
        print("Dichotomy split done: " + str([len(candidat) for candidat in candidats if len(candidat)!=1] ))

        nablaRddmin=[] #result initialization
        deltasCurrent=deltas
        for candidat in candidats: #this loop can be parallelized easily (cost SRDDMin)
            if len(candidat)==1: #is a valid ddmin
                nablaRddmin.append(candidat)
                deltasCurrent=[delta for delta in deltasCurrent if delta not in candidat]
            else:
                self.report_progress(candidat, algo_name)
                #we do not known id candidat is a valid ddmin (in case of sparse pattern)
                nablaSddmin=self.SRDDMin(candidat,SrunTab)
                for ddMin in nablaSddmin:
                    nablaRddmin.append(ddMin) #add to nablaRddmin
                    deltasCurrent=self.reduceSearchSpace(deltasCurrent,ddMin)

        print("Dichotomy split analyze done")

        #after the split filter a classic (s)rddmin is applied
        testResult=self._test(deltasCurrent,nbRun)
        if testResult!=self.FAIL:
            return nablaRddmin
        else:
            return nablaRddmin+self.SRDDMin(deltasCurrent, SrunTab)


    def SRDDMin(self, deltas,runTab):#runTab=rddMinTab):
        #name for progression
        algo_name="SRDDMin"
        #assert with the right nbRun number
        nbRun=runTab[-1]
        testResult=self._test(deltas,nbRun)
        if testResult!=self.FAIL:
            self.internalError("SRDDMIN", md5Name(deltas)+" should fail")

        nablaRddmin=[]
        deltasCurrent=deltas
        nbMin=self._getSampleNumberToExpectFail(deltas)

        filteredRunTab=[x for x in runTab if x>=nbMin]
        if len(filteredRunTab)==0:
            filteredRunTab=[nbRun]
        #increasing number of run

        for run in filteredRunTab:
            testResult=self._test(deltasCurrent,run)

            #rddmin loop
            while testResult==self.FAIL:
                self.report_progress(deltasCurrent, algo_name)
                (conf,failList) = self.dd_min(deltasCurrent,run)
                if len(conf)!=1:
                    #may be not minimal due to number of run)
                    for runIncValue in [x for x in runTab if x>run ]:
                        conf,failIncList = self.dd_min(conf,runIncValue)
                        for failInc in failIncList:
                            if failInc not in failList:
                                failList=failInc+failList
                        if len(conf)==1:
                            break

                nablaRddmin.append(conf)
                self.configuration_found("ddmin%d"%(self.rddminIndex), conf)
                self.rddminIndex+=1

                #could be nice to sort failList to begin by small failConf
                failList.sort(key=lambda x: len(x))
                #update search space
                deltasCurrent=self.reduceSearchSpace(deltasCurrent,conf)
                while len(failList)!=0:
                    failConf=failList[0]
                    failList=failList[1:]
                    if all(x in deltasCurrent for x in  failConf): #check inclusion

                        testResult=self._test(failConf,nbRun)
                        if testResult!=self.FAIL:
                            self.internalError("SRDDMIN inner", md5Name(failConf)+" should fail")
                        conf,failIncList = self.dd_min(failConf,run)

                        for failInc in failIncList:
                            if failInc not in failList:
                                failList+=failInc
                        if len(conf)!=1:
                            #may be not minimal due to number of run)
                            for runIncValue in [x for x in runTab if x>run ]:
                                conf,failIncList = self.dd_min(conf,runIncValue)
                                for failInc in failIncList:
                                    if failInc not in failList:
                                        failList+=failInc

                                if len(conf)==1:
                                    break
                        failList.sort(key=lambda x: len(x))
                        self.configuration_found("ddmin%d"%(self.rddminIndex), conf)
                        self.rddminIndex+=1
                        nablaRddmin.append(conf)
                        deltasCurrent=self.reduceSearchSpace(deltasCurrent,conf)

                #end test loop of rddmin
                testResult=self._test(deltasCurrent,nbRun)

        return nablaRddmin

    #Error Msg
    def emptySearchSpaceFailure(self):
        print("FAILURE : delta-debug search space is empty")
        failure()

    def searchSpaceGenerationFailure(self):
        print("The generation of exclusion/source files failed")
        failure()

    def fullPerturbationSucceedsFailure(self):
        print("FAILURE: nothing to debug (the run with all symbols activated succeed)")
        print("Suggestions:")
        cmpScript=str((self.compare_.absolute()).relative_to(Path.cwd()))
        print("\t1) check the correctness of the %s script : the failure criteria may be too large"%cmpScript)
        print("\t2) check if the number of samples (option --nruns=) is sufficient ")

        print("Directory to analyze: FullPerturbation")
        failure()

    def noPerturbationFailsFailure(self):
        print("FAILURE: the comparison between the reference (code instrumented with nearest mode) and the code without instrumentation failed")

        print("Suggestions:")
        cmpScript=str((self.compare_.absolute()).relative_to(Path.cwd()))
        print("\t1) check if reproducibility discrepancies are larger than the failure criteria of the script %s"%cmpScript)
        print("\t2) check the libm library is not instrumented")
        print("Directory to analyze: NoPerturbation")
        failure()

    def referenceFailsFailure(self):
        print("FAILURE: the reference is not valid ")
        print("Suggestions:")
        print("\t1) check the correctness of the %s script"%self.compare_)

        def relativePathRef(filename):
            return str((self.ref_.absolute()).relative_to(Path.cwd()) / filename)

        print("Files to analyze:")
        print("\t run output: " + relativePathRef("dd.out"))
        print("\t             " + relativePathRef("dd.err"))
        print("\t cmp output: " + relativePathRef("checkRef.out"))
        print("\t             " + relativePathRef("checkRef.err"))
        failure()

    def referenceRunFailure(self):
        print("FAILURE: the reference run fails ")
        print("Suggestions:")
        print("\t1) check the correctness of the %s script"%self.run_)

        def relativePathRef(filename):
            return str((self.ref_.absolute()).relative_to(Path.cwd()) / filename)

        print("Files to analyze:")
        print("\t run output: " + relativePathRef("dd.out"))
        print("\t             " + relativePathRef("dd.err"))
        failure()


    def getDelta0(self):
        return self.loadDeltaFileFromRep(self.ref_, True)


    def genExcludeIncludeFile(self, dirname, deltas, include=False, exclude=False):
        """Generate the *.exclude and *.include file in dirname rep from deltas"""
        excludes=self.getDelta0()
        dd=self.getDeltaFileName()

        if include:
            with open(dirname / (dd+".include"), "w") as f:
                for d in deltas:
                    f.write(d+"\n")

        if exclude:
            with open(dirname / (dd+".exclude"), "w") as f:
                for d in deltas:
                    excludes.remove(d)

                for line in excludes:
                    f.write(line+"\n")

    def _getSampleNumberToExpectFail(self, deltas):
        nbRun=self.config_.get_nbRUN()

        dirname= self.prefix_ / md5Name(deltas)
        if not dirname.is_dir():
            self.internalError("_getSampleNumberToExpectFail:", str(dirname)+" should exist")

        p=getEstimatedFailProbability(dirname)
        if p==1.:
            return 1
        else:
            alpha=0.85
            return int(min( math.ceil(math.log(1-alpha) / math.log(1-p)), nbRun))

    def _test(self, deltas,nbRun=None, earlyExit=True):
        if nbRun==None:
            nbRun=self.config_.get_nbRUN()
        resTab=self._testTab([deltas],[nbRun], earlyExit)
        return resTab[0]


    def _testTab(self, deltasTab,nbRunTab=None, earlyExit=True, firstConfFail=False, firstConfPass=False, sortOrder="outerSampleInnerConf"):
        nbDelta=len(deltasTab)
        assert(sortOrder in ["outerSampleInnerConf", "outerConfInnerSample","triangle"])

        numThread=self.config_.get_maxNbPROC()
        if numThread in [None,1]:
            return self._testTabSeq(deltasTab,nbRunTab,earlyExit,firstConfFail,firstConfPass)
        return self._testTabPar(deltasTab,nbRunTab,earlyExit,firstConfFail,firstConfPass, sortOrder)

    def _testTabSeq(self, deltasTab,nbRunTab, earlyExit=True, firstConfFail=False, firstConfPass=False, sortOrder="outerSampleInnerConf"):
        if nbRunTab==None:
            nbRunTab=[self.config_.get_nbRUN()]*nbDelta

        if sortOrder in ["outerConfInnerSample","outerSampleInnerConf","triangle"]:
            resTab,stochTaskTab,subTaskDataTab,cacheTab=self.stochTaskTabPrepare(deltasTab,nbRunTab, sortOrder, earlyExit, firstConfFail,firstConfPass)
            if subTaskDataTab==None:
                return resTab
            nbDelta=len(deltasTab)
            failIndexesTab=[[] for i in range(nbDelta)]
            passIndexesTab=[[] for i in range(nbDelta)]

            for subTaskData in subTaskDataTab:
                cmpOrRun,deltaIndex, sampleIndex=subTaskData
                sampleRes=stochTaskTab[deltaIndex].submitSeq(cmpOrRun,sampleIndex)

                if sampleRes==self.PASS:
                    passIndexesTab[deltaIndex]+=[sampleIndex]
                    if resTab[deltaIndex]==None:
                        resTab[deltaIndex]=self.PASS
                    if earlyExit and firstConfPass:
                        if set(passIndexesTab[deltaIndex])==set([task[2] for task in subTaskDataTab if task[1]==deltaIndex]):
                            self.printParProgress(resTab, stochTaskTab, passIndexesTab, failIndexesTab, cacheTab, earlyExit)
                            return resTab
                if sampleRes==self.FAIL:
                    if resTab[deltaIndex]==None and earlyExit:
                        print(stochTaskTab[deltaIndex].pathToPrint+ " --(/run/) -> FAIL(%i)"%(sampleIndex))
                    resTab[deltaIndex]=self.FAIL
                    failIndexesTab[deltaIndex]+=[sampleIndex]
                    if earlyExit and firstConfFail:
                        self.printParProgress(resTab, stochTaskTab, passIndexesTab, failIndexesTab, cacheTab, earlyExit)
                        return resTab

            self.printParProgress(resTab, stochTaskTab, passIndexesTab, failIndexesTab, cacheTab, earlyExit)
            return resTab

    def stochTaskTabPrepare(self, deltasTab,nbRunTab, sortOrder, earlyExit, firstConfFail,firstConfPass):
        nbDelta=len(deltasTab)
        if nbRunTab==None:
            nbRunTab=[self.config_.get_nbRUN()]*nbDelta

        resTab=[None] *nbDelta
        stochTaskTab=[None] *nbDelta
        cacheTab=[False for i in range(nbDelta)]

        subTaskDataUnorderTab=[]

        for deltaIndex in range(nbDelta):
            deltas=deltasTab[deltaIndex]
            dirname=self.prefix_ / md5Name(deltas)
            if not dirname.is_dir():
                dirname.mkdir()
                self.genExcludeIncludeFile(dirname, deltas, include=True, exclude=True)

            stochTaskTab[deltaIndex]=stochTask(dirname, self.ref_, self.run_, self.compare_ ,
                                               self.sampleRunEnv(dirname),
                                               seedTab=self.seedTab, seedEnvVar=self.config_.get_envVarSeed())
            sToC=stochTaskTab[deltaIndex].sampleToCompute(nbRunTab[deltaIndex], earlyExit)

            if sToC==None:
                resTab[deltaIndex]=self.FAIL
                print(stochTaskTab[deltaIndex].pathToPrint+" --(/cache/) -> FAIL")
                cacheTab[deltaIndex]=True
                continue
            if sToC["failedIndexes"]!=[]:
                resTab[deltaIndex]=self.FAIL
                print(stochTaskTab[deltaIndex].pathToPrint+" --(/cache/) -> FAIL ->")
                cacheTab[deltaIndex]=False

            if len(sToC["indexesToRun"])==0 and len(sToC["indexesToCmp"])==0 and sToC["failedIndexes"]==[]:
                #evrything in cache and pass
                resTab[deltaIndex]=self.PASS
                print(stochTaskTab[deltaIndex].pathToPrint +" --(/cache/) -> PASS("+ str(nbRunTab[deltaIndex])+")")
                cacheTab[deltaIndex]=True
                continue
            subTaskDataUnorderTab+=[("cmp",deltaIndex, cmpConf ) for cmpConf in sToC["indexesToCmp"] ]
            subTaskDataUnorderTab+=[("run",deltaIndex, runConf ) for runConf in sToC["indexesToRun"] ]
        if earlyExit:
            if firstConfFail:
                if self.FAIL in resTab:
                    return (resTab,None,None, cacheTab)
            if firstConfPass:
                if self.PASS in resTab:
                    return (resTab,None,None, cacheTab)

        subTaskDataTab=[]
        if sortOrder=="outerSampleInnerConf":
            sampleTab= [uniq_sample for uniq_sample in set([x[2] for x in subTaskDataUnorderTab])]
            sampleTab.sort()
            for sample in sampleTab:
                subTaskDataTab+=[subTaskData for subTaskData in subTaskDataUnorderTab if (subTaskData[0]=="cmp" and subTaskData[2]==sample)]
            for sample in sampleTab:
                subTaskDataTab+=[subTaskData for subTaskData in subTaskDataUnorderTab if (subTaskData[0]=="run" and subTaskData[2]==sample)]
        if sortOrder=="triangle":
            sampleTab= [uniq_sample for uniq_sample in set([x[2] for x in subTaskDataUnorderTab])]
            sampleTab.sort()
            for sample in sampleTab:
                subTaskDataTab+=[subTaskData for subTaskData in subTaskDataUnorderTab if (subTaskData[0]=="cmp" and subTaskData[2]==sample)]
            runTab=[subTaskData for subTaskData in subTaskDataUnorderTab if subTaskData[0]=="run"]
            runTab.sort(key=lambda task: task[1]+task[2])
            subTaskDataTab+=runTab

        if sortOrder=="outerConfInnerSample":
            subTaskDataTab=subTaskDataUnorderTab

        if len(subTaskDataTab)!=len(subTaskDataUnorderTab):
            print("internal error")
            print("subTaskDataUnorderTab", subTaskDataUnorderTab)
            print("subTaskDataTab", subTaskDataTab)
            failure()

        return (resTab,stochTaskTab,subTaskDataTab, cacheTab)

    def printParProgress(self,resTab, stochTaskTab, passIndexesTab, failIndexesTab, cacheTab, earlyExit):
        #affichage
        nbDelta=len(resTab)
        for deltaIndex in range(nbDelta):
            if cacheTab[deltaIndex]:#print already done
                continue
            if resTab[deltaIndex]==self.PASS:
                print(stochTaskTab[deltaIndex].pathToPrint+" --(/run/) -> PASS(+" + str(len(passIndexesTab[deltaIndex]))+"->"+str( max(passIndexesTab[deltaIndex])+1 )+")" )
            if resTab[deltaIndex]==self.FAIL:
                if not earlyExit:
                    failIndexes=failIndexesTab[deltaIndex]
                    failIndexes.sort()
                    failIndexesStr=(str(failIndexes)[1:-1]).replace(" ","")
                    print(stochTaskTab[deltaIndex].pathToPrint+" --(/run/) -> FAIL(%s)"%(failIndexesStr))

    def _testTabPar(self, deltasTab,nbRunTab=None, earlyExit=True, firstConfFail=False, firstConfPass=False, sortOrder="outerSampleInnerConf"):
        resTab,stochTaskTab,subTaskDataTab,cacheTab=self.stochTaskTabPrepare(deltasTab,nbRunTab, sortOrder, earlyExit, firstConfFail,firstConfPass)
        if subTaskDataTab==None:
            return resTab
        nbDelta=len(deltasTab)
        failIndexesTab=[[] for i in range(nbDelta)]
        passIndexesTab=[[] for i in range(nbDelta)]

        activeDataTab=[True for i in subTaskDataTab]
        numThread=self.config_.get_maxNbPROC()
        import concurrent.futures
        with concurrent.futures.ThreadPoolExecutor(max_workers=numThread) as executor:
            futurePool= [executor.submit(stochTaskTab[subTaskData[1]].submitSeq , subTaskData[0], subTaskData[2]) for subTaskData in subTaskDataTab[0:numThread]]
            dataIndexPool=[i for i in range(numThread)]
            poolLoop=futurePool
            currentWork=numThread

            while len(poolLoop)!=0:
                (doneTab,toDo)=concurrent.futures.wait(poolLoop,return_when=concurrent.futures.FIRST_COMPLETED)
                newFutures=[]
                poolSlotAvail=[]
                for future in doneTab:
                    poolIndex=futurePool.index(future)
                    poolSlotAvail+=[poolIndex]
                    indexData=dataIndexPool[poolIndex]

                    (computeType, deltaIndex, sampleIndex)=subTaskDataTab[indexData]
                    res=future.result()
                    if res==self.PASS:
                        if resTab[deltaIndex]==None:
                            resTab[deltaIndex]=self.PASS
                        passIndexesTab[deltaIndex]+=[sampleIndex]
                        if earlyExit and firstConfPass:
                            if set(passIndexesTab[deltaIndex])==set([task[2] for task in subTaskDataTab if task[1]==deltaIndex]):
                                for cWork in range(currentWork,len(subTaskDataTab)):
                                    activeDataTab[cWork]=False
                    else:
                        if resTab[deltaIndex]==None and earlyExit:
                            print(stochTaskTab[deltaIndex].pathToPrint+" --(/run/) -> FAIL(%i)"%(sampleIndex))
                        resTab[deltaIndex]=self.FAIL
                        failIndexesTab[deltaIndex]+=[sampleIndex]
                        if earlyExit and firstConfFail:
                            for cWork in range(currentWork,len(subTaskDataTab)):
                                if subTaskDataTab[cWork][1]==deltaIndex :
                                    activeDataTab[cWork]=False
                #submit new subtasks
                for poolIndex in poolSlotAvail:
                    #ignore cancelled subtask
                    while currentWork < len(subTaskDataTab) and activeDataTab[currentWork]==False:
                        currentWork+=1
                    if currentWork < len(subTaskDataTab):
                        if activeDataTab[currentWork]:
                            currentSubTask=subTaskDataTab[currentWork]
                            computeType=currentSubTask[0]
                            deltaIndex=currentSubTask[1]
                            sampleIndex=currentSubTask[2]
                            futur=executor.submit(stochTaskTab[deltaIndex].submitSeq ,computeType, sampleIndex)
                            futurePool[poolIndex]=futur
                            newFutures+=[futur]
                            dataIndexPool[poolIndex]=currentWork
                            currentWork+=1

                poolLoop=list(toDo)+newFutures
        #affichage
        self.printParProgress(resTab, stochTaskTab, passIndexesTab, failIndexesTab, cacheTab, earlyExit)
        return resTab
