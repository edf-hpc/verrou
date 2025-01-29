
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
import glob
import datetime
import math
from valgrind import convNumLineTool
from valgrind import DD


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


def runCmd(cmd, fname, envvars=None):
    """Run CMD, adding ENVVARS to the current environment, and redirecting standard
    and error outputs to FNAME.out and FNAME.err respectively.

    Returns CMD's exit code."""

    return getResult(runCmdAsync(cmd,fname,envvars))



class verrouTask:

    def __init__(self, dirname, refDir,runCmd, cmpCmd,nbRun, maxNbPROC, runEnv , verbose=True, seedTab=None, seedEnvVar=None):
        self.dirname=dirname
        self.refDir=refDir
        self.runCmd=runCmd
        self.cmpCmd=cmpCmd
        self.nbRun=nbRun
        self.FAIL=DD.DD.FAIL
        self.PASS=DD.DD.PASS

        self.subProcessRun={}
        self.maxNbPROC= maxNbPROC
        self.runEnv=runEnv
        self.verbose=verbose
        self.alreadyFail=False
        self.pathToPrint=os.path.relpath(self.dirname, os.getcwd())
        self.preRunLambda=None
        self.postRunLambda=None
        self.seedTab=seedTab
        self.bufferizedPrint=False
        self.bufferPrint=""
        self.seedEnvVar_=seedEnvVar

    def printProgress(self,string, end="\n", flush=None):
        if not self.bufferizedPrint:
            if flush==True:
                print(string, end=end, flush=True)
            else:
                print(string,end=end)
        else:
            self.bufferPrint+=(string+end)
    def flushProgress(self):
        print(self.bufferPrint, flush=True, end="")
        self.bufferPrint=""

    def setPostRun(self, postLambda):
        self.postRunLambda=postLambda

    def setPreRun(self, preLambda):
        self.preRunLambda=preLambda


    def printDir(self):
        self.printProgress(self.pathToPrint,end="")

    def nameDir(self,i,relative=False):
        if relative:
            return "dd.run%i" % (i)
        else:
            return  os.path.join(self.dirname,"dd.run%i" % (i))

    def mkdir(self,i):
         os.mkdir(self.nameDir(i))
    def rmdir(self,i):
        shutil.rmtree(self.nameDir(i))

    def replacePattern(self, value, i):
        return value.replace("%DDRUN%", self.nameDir(i,True))

    def runOneSample(self,i):
        rundir= self.nameDir(i)
        env={key:self.replacePattern(self.runEnv[key],i) for key in self.runEnv}
        if self.seedTab!=None:
            env[self.seedEnvVar_]=str(self.seedTab[i])
        if self.preRunLambda!=None:
            self.preRunLambda(rundir, env)
        self.subProcessRun[i]=runCmdAsync([self.runCmd, rundir],
                                          os.path.join(rundir,"dd.run"),
                                          env)


    def cmpOneSample(self,i, assertRun=True):

        rundir= self.nameDir(i)
        if assertRun:
            if self.subProcessRun[i]!=None:
                getResult(self.subProcessRun[i])
                if self.postRunLambda!=None:
                    self.postRunLambda(rundir)

        if self.refDir==None: #if there are no reference provided cmp is ignored
            return self.PASS
        retval = runCmd([self.cmpCmd, self.refDir, rundir],
                        os.path.join(rundir,"dd.compare"))

        with open(os.path.join(rundir, "dd.return.value"),"w") as f:
            f.write(str(retval))
        if retval != 0:
            self.alreadyFail=True
#            if self.verbose:
#                print("FAIL(%d)" % i)
            return self.FAIL
        else:
#            if self.alreadyFail:
#                print("PASS(%d)" % i)
            return self.PASS

    def sampleToCompute(self, nbRun, earlyExit):
        """Return the two lists of samples which have to be compared or computed (and compared) to perforn nbRun Success run : None means Failure ([],[]) means Success """
        listOfDirString=[runDir for runDir in os.listdir(self.dirname) if runDir.startswith("dd.run")]
        listOfDirIndex=[ int(x.replace("dd.run",""))  for x in listOfDirString  ]

        cmpDone=[]
        runDone=[]
        workToCmpOnly=[]
        failureIndex=[]
        for runDir in listOfDirString:

            returnValuePath=os.path.join(self.dirname, runDir, "dd.return.value")
            ddRunIndex=int(runDir.replace("dd.run",""))
            if os.path.exists(returnValuePath):
                statusCmp=int((open(returnValuePath).readline()))
                if statusCmp!=0:
                    if earlyExit:
                        return None
                    else:
                        failureIndex+=[ddRunIndex]
                cmpDone+=[ddRunIndex]
            else:
                runPath=os.path.join(self.dirname, runDir, "dd.run.out")
                if os.path.exists(runPath):
                    runDone+=[ddRunIndex]

        workToRun= [x for x in range(nbRun) if (((not x in runDone+cmpDone) and (x in listOfDirIndex )) or (not (x in listOfDirIndex))) ]
        return (runDone, workToRun, cmpDone, failureIndex)

    def getEstimatedFailProbability(self):
        """Return an estimated probablity of fail for the configuration"""
        listOfDirString=[runDir for runDir in os.listdir(self.dirname) if runDir.startswith("dd.run")]
        listOfDirIndex=[ int(x.replace("dd.run",""))  for x in listOfDirString  ]

        cacheCounter=0.
        cacheFail=0.
        for runDir in listOfDirString:
            returnValuePath=os.path.join(self.dirname, runDir, "dd.return.value")
            if os.path.exists(returnValuePath):
                cacheCounter+=1.
                statusCmp=int((open(returnValuePath).readline()))
                if statusCmp!=0:
                    cacheFail+=1.
        return cacheFail / cacheCounter

    def run(self, earlyExit=True):
        if self.verbose:
            self.printDir()

        workToDo=self.sampleToCompute(self.nbRun, earlyExit)
        if workToDo==None:
            self.printProgress(" --(cache) -> FAIL")
            return self.FAIL
        cmpOnlyToDo=workToDo[0]
        runToDo=workToDo[1]
        cmpDone=workToDo[2]
        failureIndex=workToDo[3]

        if len(cmpOnlyToDo)==0 and len(runToDo)==0:
            if(len(failureIndex)==0):
                self.printProgress(" --(cache) -> PASS("+str(self.nbRun)+")")
                return self.PASS
            else:
                self.printProgress(" --(cache) -> FAIL(%s)"%((str(failureIndex)[1:-1]).replace(" ","")))
                return self.FAIL

        if len(cmpOnlyToDo)!=0:
            self.printProgress(" --( cmp ) -> ",end="",flush=True)
            returnVal=self.cmpSeq(cmpOnlyToDo, earlyExit)
            if returnVal==self.FAIL:
                if earlyExit:
                    self.printProgress("FAIL", end="\n",flush=True)
                    return self.FAIL
                else:
                    self.printProgress("FAIL", end="",flush= True)
            else:
                self.printProgress("PASS(+" + str(len(cmpOnlyToDo))+"->"+str(len(cmpDone) +len(cmpOnlyToDo))+")" , end="", flush=True)

        self.futures=[]
        self.dataAsyncPrint=None
        if len(runToDo)!=0:

            if self.maxNbPROC in [None,1]:
                returnVal=self.runSeq(runToDo, earlyExit, self.verbose)
            elif self.maxNbPROC=="async":
                self.runParAsync(runToDo)
                self.dataAsyncPrint="PASS(+" + str(len(runToDo))+"->"+str( len(cmpOnlyToDo) +len(cmpDone) +len(runToDo) )+")"
                return None
            else:
                returnVal=self.runPar(runToDo)

            if(returnVal==self.PASS):
                self.printProgress("PASS(+" + str(len(runToDo))+"->"+str( len(cmpOnlyToDo) +len(cmpDone) +len(runToDo) )+")" )
            return returnVal
        else:
            self.printProgress("")
        return self.PASS



    def cmpSeq(self,workToDo, earlyExit):
        res=self.PASS
        for run in workToDo:
            retVal=self.cmpOneSample(run,assertRun=False)
            if retVal=="FAIL":
                res=self.FAIL
                if earlyExit:
                    return res
        return res


    def runSeq(self,workToDo, earlyExit,printStatus=False):
        if printStatus:
            self.printProgress(" --( run ) -> ",end="",flush=True)
        res=self.PASS
        for run in workToDo:
            if not os.path.exists(self.nameDir(run)):
                self.mkdir(run)
            else:
                print(self.nameDir(run) +" : manual cache modification detected (runSeq)")

            if self.alreadyFail:
                if printStatus:
                    self.printProgress(" "*len(self.pathToPrint)+" --( run ) -> ", end="", flush=True)
            self.runOneSample(run)
            retVal=self.cmpOneSample(run)

            if retVal=="FAIL":
                res=self.FAIL

                if earlyExit:
                    if printStatus:
                        self.printProgress("FAIL(%i)"%(run))
                    return res
                else:
                    if printStatus:
                        self.printProgress("FAIL(%i)"%(run))
                        self.alreadyFail=True
        return res

    def submitSeq(self, cmpOrRun, sampleIndex, earlyExit=False):
        if cmpOrRun=="cmp":
            return self.cmpSeq([sampleIndex],earlyExit)
        if cmpOrRun=="run":
            return self.runSeq([sampleIndex],earlyExit)


    def runPar(self,workToDo, earlyExit=True):
        self.printProgress(" --(/run ) -> ",end="",flush=True)
        import concurrent.futures

        numThread=self.maxNbPROC
        activeDataTab=[True for i in range(len(workToDo))]
        failIndices=[]

        with concurrent.futures.ThreadPoolExecutor(max_workers=numThread) as executor:
            futurePool=[executor.submit(self.runSeq, [work],False, False) for work in workToDo[0:numThread]]
            dataIndexPool    = [i for i in range(numThread)]
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
                    indexSample=workToDo[indexData]
                    FAILorPASS =future.result()
                    if FAILorPASS==self.FAIL:
                        failIndices+=[indexSample]
                        if earlyExit:
                            for cWork in range(currentWork, len(workToDo)):
                                activeDataTab[cWork]=False
                    else:
                        pass

                for poolIndex in poolSlotAvail:
                    while currentWork < len(workToDo) and activeDataTab[currentWork]==False:
                        currentWork+=1
                    if currentWork < len(workToDo):
                        if activeDataTab[currentWork]:
                            futur=executor.submit(self.runSeq, [workToDo[currentWork]],False,False)
                            futurePool[poolIndex]=futur
                            newFutures+=[futur]
                            dataIndexPool[poolIndex]=currentWork
                            currentWork+=1

                poolLoop=list(toDo)+newFutures

        if len(failIndices)!=0:
            failIndices.sort()
            self.printProgress("FAIL(%s)"%((str(failIndices)[1:-1])).replace(" ",""))
            return self.FAIL
        return self.PASS

    def runParAsync(self, workToDo):
        self.printProgress(" --(/run/) -> ",end="",flush=True)
        self.futures=[self.executor.submit(self.runSeq, [work],False, False) for work in workToDo]

    def runParAsyncWait(self):
        import concurrent.futures
        concurrent.futures.wait(self.futures)
        results=[futur.result() for futur in self.futures]
        if self.FAIL in results:
            indices=[i for i in range(len(self.futures)) if self.futures[i].result()==self.FAIL]
            failIndices=[workToDo[indice] for indice in indices ]
            self.printProgress("FAIL(%s)"%((str(failIndices)[1:-1])).replace(" ",""))
            self.flushProgress()
            return self.FAIL
        if self.dataAsyncPrint!=None:
            self.printProgress(self.dataAsyncPrint);
        self.flushProgress()
        return self.PASS

def runMultipleVerrouTask(verrouTaskTab, maxNbPROC):
    import concurrent.futures
    executor=concurrent.futures.ThreadPoolExecutor(max_workers=maxNbPROC)
    for verrouTask in verrouTaskTab:
        verrouTask.executor=executor
        verrouTask.maxNbPROC="async"
        verrouTask.bufferizedPrint=True
        verrouTask.run()

    for verrouTask in verrouTaskTab:
        verrouTask.runParAsyncWait()

    for verrouTask in verrouTaskTab:
        verrouTask.executor=None


def md5Name(deltas):
    copyDeltas=copy.copy(deltas)
    copyDeltas.sort()
    return hashlib.md5(("".join(["\n"+x for x in copyDeltas])).encode('utf-8')).hexdigest()


def prepareOutput(dirname):
     shutil.rmtree(dirname, ignore_errors=True)
     os.makedirs(dirname)


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
        self.cache_outcomes = False # the cache of DD.DD is ignored
        self.index=0
        self.prefix_ = os.path.join(os.getcwd(), self.config_.get_cacheRep())
        self.relPrefix_=self.config_.get_cacheRep()
        self.ref_ = os.path.join(self.prefix_, "ref")
        self.prepareCache()
        prepareOutput(self.ref_)

        self.config_.saveParam(os.path.join(self.prefix_,"cmd.last"))

        self.reference() #generate the reference computation
        self.mergeList(parseRef) #generate the search space
        self.rddminHeuristicLoadRep(selectBlocAndNumLine, joinBlocAndNumLine) # at the end because need the search space

        self.initSeed()

    def initSeed(self):
        if self.config_.ddSeed!=None:
            import random
            random.seed(self.config_.ddSeed)
            self.seedTab=random.sample(range(0,2**32-1), self.config_.get_nbRUN())
        else:
            self.seedTab=None

    def symlink(self,src, dst):
        """Create a relative symlink"""
        if os.path.lexists(dst):
            os.remove(dst)
        relSrc=os.path.relpath(src, self.prefix_ )
        relDist=os.path.relpath(dst, self.prefix_)
        relPrefix=os.path.relpath(self.prefix_, os.getcwd())
        os.symlink(relSrc, os.path.join(relPrefix, relDist))


    def cleanSymLink(self):
        """Delete all symlink in the cache"""
        self.saveCleabSymLink=[]
        symLinkTab=self.searchSymLink()
        for symLink in symLinkTab:
            if os.path.lexists(symLink):
                os.remove(symLink)
                if self.config_.get_cache=="continue":
                    self.saveCleanSymLink+=[symLink]


    def searchSymLink(self):
        """Return the list of symlink (created by DD_stoch) in the cache"""
        res =glob.glob(os.path.join(self.prefix_, "ddmin*"))
        res+=glob.glob(os.path.join(self.prefix_, "ddmax"))
        res+=glob.glob(os.path.join(self.prefix_, "rddmin-cmp"))
        res+=glob.glob(os.path.join(self.prefix_, "FullPerturbation"))
        res+=glob.glob(os.path.join(self.prefix_, "NoPerturbation"))
        return res


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
                    cacheRep=self.oldCacheName
                    if os.path.isdir(cacheRep):
                        rddmin_heuristic_rep+=[cacheRep]
            else:
                cacheRep=self.prefix_
                if os.path.isdir(cacheRep):
                    rddmin_heuristic_rep+=[cacheRep]

        if self.config_.get_rddminHeuristicsCache()=="all_cache":
            rddmin_heuristic_rep+=glob.glob(self.prefix_+"-*-*-*_*h*m*s")

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
                deltaOld=self.loadDeltaFileFromRep(os.path.join(rep,"ref"), True)
                if deltaOld==None:
                    continue
                cvTool=convNumLineTool.convNumLineTool(deltaOld, self.getDelta0(), selectBlocAndNumLine, joinBlocAndNumLine)
                repTab=glob.glob(os.path.join(rep, "ddmin*"))

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
                repTab=glob.glob(os.path.join(rep, "ddmin*"))
                for repDDmin in repTab:
                    deltas=self.loadDeltaFileFromRep(repDDmin)
                    if deltas==None:
                        continue
                    self.ddminHeuristic+=[deltas]

    def loadDeltaFileFromRep(self,rep, ref=False):
        fileName=os.path.join(rep, self.getDeltaFileName()+".include")
        if ref:
            fileName=os.path.join(rep, self.getDeltaFileName())
        return self.loadDeltaFile(fileName)

    def loadDeltaFile(self,fileName):
        if os.path.exists(fileName):
            deltasTab=[ x.rstrip() for x in (open(fileName)).readlines()]
            return deltasTab
        else:
            print(fileName + " do not exist")
        return None


    def prepareCache(self):
        cache=self.config_.get_cache()
        if cache=="continue":
            if not os.path.exists(self.prefix_):
                os.mkdir(self.prefix_)
            self.cleanSymLink()
            return
        if cache=="clean":
            shutil.rmtree(self.prefix_, ignore_errors=True)
            os.mkdir(self.prefix_)
            return

        if cache=="rename_keep_result":
            #delete unusefull rep : rename treated later
            symLinkTab=self.searchSymLink()
            repToKeep=[os.readlink(x) for x in symLinkTab]
            print(repToKeep)
            for item in os.listdir(self.prefix_):
                if len(item)==32 and all(i in ['a', 'b', 'c', 'd', 'e', 'f']+[str(x) for x in range(10)] for i in item) :
                    #check md5sum format
                    if not item in repToKeep:
                        shutil.rmtree(os.path.join(self.prefix_, item))

        if cache.startswith("rename"):
            if os.path.exists(self.prefix_):
                symLinkTab=self.searchSymLink()
                if symLinkTab==[]:  #find alternative file to get time stamp
                    refPath=os.path.join(self.prefix_, "ref")
                    if os.path.exists(refPath):
                        symLinkTab=[refPath]
                    else:
                        symLinkTab=[self.prefix_]
                timeStr=datetime.datetime.fromtimestamp(max([os.path.getmtime(x) for x in symLinkTab])).strftime("%m-%d-%Y_%Hh%Mm%Ss")
                self.oldCacheName=self.prefix_+"-"+timeStr
                os.rename(self.prefix_,self.oldCacheName )
            else:
                self.oldCacheName=None
            os.mkdir(self.prefix_)


        if cache=="keep_run":
            if not os.path.exists(self.prefix_):
                os.mkdir(self.prefix_)
            else:
                self.cleanSymLink()
                filesToDelete =glob.glob(os.path.join(self.prefix_, "*/dd.run[0-9]*/dd.compare.*"))
                filesToDelete +=glob.glob(os.path.join(self.prefix_, "*/dd.run[0-9]*/dd.return.value"))
                for fileToDelete in filesToDelete:
                    os.remove(fileToDelete)

    def reference(self):
        """Run the reference and check the result"""
        print(os.path.relpath(self.ref_, os.getcwd()),end="")
        print(" -- (run) -> ",end="",flush=True)
        retval = runCmd([self.run_, self.ref_],
                        os.path.join(self.ref_,"dd"),
                        self.referenceRunEnv())
        if retval!=0:
            print("")
            self.referenceRunFailure()
        else:
            """Check the comparison between the reference and refrence is valid"""
            retval = runCmd([self.compare_,self.ref_, self.ref_],
                            os.path.join(self.ref_,"checkRef"))
            if retval != 0:
                print("FAIL")
                self.referenceFailsFailure()
            else:
                print("PASS")


    def mergeList(self, parseRef):
        """merge the file name.$PID into a uniq file called name """
        dirname=self.ref_
        name=self.getDeltaFileName()

        listOfExcludeFile=[ x for x in os.listdir(dirname) if self.isFileValidToMerge(x) ]
        if len(listOfExcludeFile)<1:
            self.searchSpaceGenerationFailure()

#        with open(os.path.join(dirname,listOfExcludeFile[0]), "r") as f:
#                excludeMerged=f.readlines()
#        for excludeFile in listOfExcludeFile[1:]:
        excludeMerged=[]
        for excludeFile in listOfExcludeFile:
            lines=None
            if parseRef==None:
                with open(os.path.join(dirname,excludeFile), "r") as f:
                    lines=[x.rstrip() for x in f.readlines()]
            else:
                lines=parseRef(os.path.join(dirname,excludeFile))

            for line in lines:
                if line not in excludeMerged:
                    excludeMerged+=[line]

        with open(os.path.join(dirname, name), "w" )as f:
            for line in excludeMerged:
                f.write(line+"\n")


    def testWithLink(self, deltas, linkname, earlyExit=True):
        testResult=self._test(deltas, self.config_.get_nbRUN() , earlyExit)
        dirname = os.path.join(self.prefix_, md5Name(deltas))
        self.symlink(dirname, os.path.join(self.prefix_,linkname))
        return testResult

    def report_progress(self, c, title):
        if not self.config_.get_quiet:
            super().report_progress(c,title)

    def configuration_found(self, kind_str, delta_config,verbose=True):
        if verbose:
            print("%s (%s):"%(kind_str,self.coerce(delta_config)))
        earlyExit=True
        if self.config_.resWithAllSamples:
            earlyExit=False
        self.testWithLink(delta_config, kind_str, earlyExit)

    def run(self, deltas=None):

        # get the search space
        if deltas==None:
            deltas=self.getDelta0()

        if(len(deltas)==0):
            self.emptySearchSpaceFailure()

        #basic verification
        testResult=self._test(deltas)
        self.configuration_found("FullPerturbation",deltas)
        if testResult!=self.FAIL:
            self.fullPerturbationSucceedsFailure()

        testResult=self._test([])
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
            return  self.DDMax(deltas)

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
            cmp= [delta for delta in deltas if  delta not in flatRes ]
            self.configuration_found("rddmin-cmp", cmp)

        return resConf

    def applyRddminWithHeuristics(self,deltas, algo):
        """Test the previous ddmin configuration (previous run of DD_stoch) as a filter to rddmin algo"""

        res=[]
        self.ddminHeuristic.sort(key=lambda x: len(x))#sort to test first small heuritics
        for heuristicsDelta in self.ddminHeuristic:
            if all(x in deltas for x in  heuristicsDelta): #check inclusion
                testResult=self._test(heuristicsDelta, self.config_.get_nbRUN())
                if testResult!=self.FAIL:
                    if not self.config_.get_quiet():
                        print("Bad rddmin heuristic : %s"%self.coerce(heuristicsDelta))
                else:
                    if not self.config_.get_quiet():
                        print("Good rddmin heuristics : %s"%self.coerce(heuristicsDelta))
                    if len(heuristicsDelta)==1:
                        res+=[heuristicsDelta]
                        self.configuration_found("ddmin%d"%(self.index), heuristicsDelta)
                        self.index+=1
                        deltas=[delta for delta in deltas if delta not in heuristicsDelta]
                    else:
                        resTab= self.check1Min(heuristicsDelta, self.config_.get_nbRUN())
                        for resMin in resTab:
                            res+=[resMin] #add to res
                            deltas=[delta for delta in deltas if delta not in resMin] #reduce search space

        print("Heuristics applied")
        #after the heuristic filter a classic (s)rddmin is applied
        testResult=self._test(deltas, self.config_.get_nbRUN())
        if testResult!=self.FAIL:
            return res
        else:
            return res+algo(deltas)





    def DDMax(self, deltas):
        res=self.verrou_dd_max(deltas)
        cmp=[delta for delta in deltas if delta not in res]
        self.configuration_found("ddmax", cmp)
        self.configuration_found("ddmax-cmp", res)

        return cmp

    def RDDMin(self, deltas,nbRun):
        ddminTab=[]
        testResult=self._test(deltas)
        if testResult!=self.FAIL:
            self.internalError("RDDMIN", md5Name(deltas)+" should fail")

        while testResult==self.FAIL:
            conf = self.verrou_dd_min(deltas,nbRun)

            ddminTab += [conf]
            self.configuration_found("ddmin%d"%(self.index), conf)
            #print("ddmin%d (%s):"%(self.index,self.coerce(conf)))

            #update deltas
            deltas=[delta for delta in deltas if delta not in conf]
            testResult=self._test(deltas,nbRun)
            self.index+=1
        return ddminTab

    def check1Min(self, deltas,nbRun):
        ddminTab=[]
        testResult=self._test(deltas)
        if testResult!=self.FAIL:
            self.internalError("Check1-MIN", md5Name(deltas)+" should fail")
        nbProc=self.config_.get_maxNbPROC()
        deltaMin1Tab=[]

        for delta1 in deltas:
            newDelta=[delta for delta in deltas if delta!=delta1]
            deltaMin1Tab+=[newDelta]
        resultTab=[]
        if nbProc in [None,1]:
            for deltaMin1 in deltaMin1Tab:
                result=self._test(deltaMin1, nbRun)
                resultTab+=[result]
                if result==self.FAIL:
                    break
        else:
            resultTab=self._testTab(deltaMin1Tab, [nbRun]*len(deltaMin1Tab), earlyExit=True, firstConfFail=True)

        for indexRes in range(len(resultTab)):
            result=resultTab[indexRes]
            if result==self.FAIL:
                if not self.config_.get_quiet():
                    print("Heuristics not 1-Minimal")
                return self.RDDMin(deltaMin1Tab[indexRes], nbRun)

        self.configuration_found("ddmin%d"%(self.index), deltas)
        self.index+=1
        return [deltas]


    def splitDeltas(self, deltas,nbRun,granularity):
        nbProc=self.config_.get_maxNbPROC()
        if nbProc in [None,1]:
            return self.splitDeltasSeq(deltas, nbRun, granularity)
        return self.splitDeltasPar(deltas, nbRun, granularity,nbProc)



    def splitDeltasPar(self, deltas,nbRun,granularity, nbProc):
        if self._test(deltas, self.config_.get_nbRUN())==self.PASS:
            return [] #short exit

        res=[] #result : set of smallest (each subset with repect with granularity lead to success)

        toTreat=[deltas]

        #name for progression
        algo_name="splitDeltasPara"

        nbPara=math.ceil( nbProc/granularity)
        while len(toTreat)>0:
            toTreatNow=toTreat[0:nbPara]
            toTreatLater=toTreat[nbPara:]

            ciTab=[self.split(candidat, min(granularity, len(candidat))) for candidat in toTreatNow]
            flatciTab=sum(ciTab,[])
            flatResTab=self._testTab(flatciTab, [nbRun]* len(flatciTab))
            resTab=[]
            lBegin=0
            for i in range(len(ciTab)): #unflat flatRes
                lEnd=lBegin+len(ciTab[i])
                resTab+=[flatResTab[lBegin: lEnd]]
                lBegin=lEnd
            remainToTreat=[]
            for i in range(len(ciTab)):

                ci=ciTab[i]
                splitFailed=False
                for j in range(len(ci)):
                    conf=ci[j]

                    if resTab[i][j]==self.FAIL:
                        splitFailed=True
                        if len(conf)==1:
                            self.configuration_found("ddmin%d"%(self.index), conf)
                            self.index+=1
                            res.append(conf)
                        else:
                            remainToTreat+=[conf]
                if not splitFailed:
                    res+=[toTreatNow[i]]

            toTreat=remainToTreat+toTreatLater
        return res


    def splitDeltasSeq(self, deltas,nbRun,granularity):
        if self._test(deltas, self.config_.get_nbRUN())==self.PASS:
            return [] #short exit

        res=[] #result : set of smallest (each subset with repect with granularity lead to success)

        #two lists which contain tasks
        # -the fail status is known
        toTreatFailed=[deltas]
        # -the status is no not known
        toTreatUnknown=[]

        #name for progression
        algo_name="splitDeltas"

        def treatFailedCandidat(candidat):
            #treat a failing configuration
            self.report_progress(candidat, algo_name)

            # create subset
            cutSize=min(granularity, len(candidat))
            ciTab=self.split(candidat, cutSize)

            cutAbleStatus=False
            for i in range(len(ciTab)):
                ci=ciTab[i]
                #test each subset
                status=self._test(ci ,nbRun)
                if status==self.FAIL:
                    if len(ci)==1:
                        #if the subset size is one the subset is a valid ddmin : treat as such
                        self.configuration_found("ddmin%d"%(self.index), ci)
                        #print("ddmin%d (%s):"%(self.index,self.coerce(ci)))
                        self.index+=1
                        res.append(ci)
                    else:
                        #insert the subset in the begin of the failed task list
                        toTreatFailed.insert(0,ci)
                        #insert the remaining subsets to the unknown task list
                        tail= ciTab[i+1:]
                        tail.reverse() # to keep the same order
                        for cip in tail:
                            toTreatUnknown.insert(0,cip)
                        return
                    cutAbleStatus=True
            #the failing configuration is failing
            if cutAbleStatus==False:
                res.append(candidat)

        def treatUnknownStatusCandidat(candidat):
            #test the configuration : do nothing in case of success and add to the failed task list in case of success
            self.report_progress(candidat, algo_name+ "(unknownStatus)")
            status=self._test(candidat, nbRun)
            if status==self.FAIL:
                toTreatFailed.insert(0,candidat)
            else:
                pass

        # loop over tasks
        while len(toTreatFailed)!=0 or len(toTreatUnknown)!=0:

            unknownStatusSize=len(deltas) #to get a max
            if len(toTreatUnknown)!=0:
                unknownStatusSize=len(toTreatUnknown[0])

            if len(toTreatFailed)==0:
                treatUnknownStatusCandidat(toTreatUnknown[0])
                toTreatUnknown=toTreatUnknown[1:]
                continue

            #select the smallest candidat : in case of equality select a fail
            toTreatCandidat=toTreatFailed[0]
            if  len(toTreatCandidat) <= unknownStatusSize:
                cutCandidat=toTreatCandidat
                toTreatFailed=toTreatFailed[1:]
                treatFailedCandidat(cutCandidat)
            else:
                treatUnknownStatusCandidat(toTreatUnknown[0])
                toTreatUnknown=toTreatUnknown[1:]
        return res

    def SsplitDeltas(self, deltas, runTab, granularity):#runTab=splitTab ,granularity=2):
        #apply splitDeltas recussivly with increasing sample number (runTab)
        #remarks the remain treatment do not respect the binary split structure

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
            remainDeltas=[delta for delta in deltas if delta not in flatNextCurrent ]

            #apply split to remainDeltas
            self.report_progress(remainDeltas,algo_name)
            nextCurrent.extend(self.splitDeltas(remainDeltas, run, granularity))

            currentSplit=nextCurrent

        return currentSplit

    def DRDDMin(self, deltas, SrunTab, dicRunTab, granularity):#SrunTab=rddMinTab, dicRunTab=splitTab, granularity=2):
        #name for progression
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

        res=[]
        for candidat in candidats:
            if len(candidat)==1: #is a valid ddmin
                res+=[candidat]
                deltas=[delta for delta in deltas if delta not in candidat]
            else:
                self.report_progress(candidat, algo_name)
                #we do not known id candidat is a valid ddmin (in case of sparse pattern)
                resTab=self.SRDDMin(candidat,SrunTab)
                for resMin in resTab:
                    res+=[resMin] #add to res
                    deltas=[delta for delta in deltas if delta not in resMin] #reduce search space
        print("Dichotomy split analyze done")

        #after the split filter a classic (s)rddmin is applied
        testResult=self._test(deltas,nbRun)
        if testResult!=self.FAIL:
            return res
        else:
            return res+self.SRDDMin(deltas, SrunTab)

    def verrou_dd_min_seqpar(self, deltas, nbRun):
        if self.config_.get_maxNbPROC() in [None,1] or not self.config_.get_use_dd_min_par():
            return (self.verrou_dd_min(deltas,nbRun),[])
        else:
            ddmin,expFail=self.verrou_dd_min_par(deltas,nbRun)
            return (ddmin,expFail)

    def SRDDMin(self, deltas,runTab):#runTab=rddMinTab):
        #name for progression
        algo_name="SRDDMin"
        #assert with the right nbRun number
        nbRun=runTab[-1]
        testResult=self._test(deltas,nbRun)
        if testResult!=self.FAIL:
            self.internalError("SRDDMIN", md5Name(deltas)+" should fail")

        ddminTab=[]
        nbMin=self._getSampleNumberToExpectFail(deltas)
        nbProc=self.config_.get_maxNbPROC()
        if nbProc!=None:
            nbMin=max(nbMin,nbProc)

        filteredRunTab=[x for x in runTab if x>=nbMin]
        if len(filteredRunTab)==0:
            filteredRunTab=[nbRun]
        #increasing number of run
        for run in filteredRunTab:
            testResult=self._test(deltas,run)

            #rddmin loop
            while testResult==self.FAIL:
                self.report_progress(deltas, algo_name)
                (conf,failList) = self.verrou_dd_min_seqpar(deltas,run)
                if len(conf)!=1:
                    #may be not minimal due to number of run)
                    for runIncValue in [x for x in runTab if x>run ]:
                        conf,failIncList = self.verrou_dd_min_seqpar(conf,runIncValue)
                        for failInc in failIncList:
                            if failInc not in failList:
                                failList+=failInc
                        if len(conf)==1:
                            break

                ddminTab += [conf]
                self.configuration_found("ddmin%d"%(self.index), conf)
                #print("ddmin%d (%s):"%(self.index,self.coerce(conf)))
                self.index+=1

                #could be nice to sort failList to begin by small failConf
                failList.sort(key=lambda x: len(x))
                #update search space
                deltas=[delta for delta in deltas if delta not in conf]
                while len(failList)!=0:
                    #print("failList:", failList)
                    failConf=failList[0]
                    failList=failList[1:]
                    if all(x in deltas for x in  failConf): #check inclusion
                        #print("// optim : ", failConf)
                        testResult=self._test(failConf,nbRun)
                        if testResult!=self.FAIL:
                            self.internalError("SRDDMIN inner", md5Name(failConf)+" should fail")
                        conf,failIncList = self.verrou_dd_min_seqpar(failConf,run)
                        #print("resConf:",conf)
                        #print("failIncList:",failIncList)
                        for failInc in failIncList:
                            if failInc not in failList:
                                failList+=failInc
                        if len(conf)!=1:
                            #may be not minimal due to number of run)
                            for runIncValue in [x for x in runTab if x>run ]:
                                conf,failIncList = self.verrou_dd_min_seqpar(conf,runIncValue)
                                for failInc in failIncList:
                                    if failInc not in failList:
                                        failList+=failInc

                                if len(conf)==1:
                                    break
                            failList.sort(key=lambda x: len(x))
                            self.configuration_found("ddmin%d"%(self.index), conf)
                            self.index+=1
                        ddminTab += [conf]
                        deltas=[delta for delta in deltas if delta not in conf]

                #end test loop of rddmin
                testResult=self._test(deltas,nbRun)

        return ddminTab

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
        cmpScript=os.path.relpath(self.compare_, os.getcwd())
        print("\t1) check the correctness of the %s script : the failure criteria may be too large"%cmpScript)
        print("\t2) check if the number of samples (option --nruns=) is sufficient ")

        print("Directory to analyze: FullPerturbation")
        failure()

    def noPerturbationFailsFailure(self):
        print("FAILURE: the comparison between the reference (code instrumented with nearest mode) and the code without instrumentation failed")

        print("Suggestions:")
        cmpScript=os.path.relpath(self.compare_, os.getcwd())
        print("\t1) check if reproducibility discrepancies are larger than the failure criteria of the script %s"%cmpScript)
        print("\t2) check the libm library is not instrumented")
        print("Directory to analyze: NoPerturbation")
        failure()

    def referenceFailsFailure(self):
        print("FAILURE: the reference is not valid ")
        print("Suggestions:")
        print("\t1) check the correctness of the %s script"%self.compare_)

        def relativePathRef(filename):
            return os.path.relpath(os.path.join(self.ref_,filename), os.getcwd())

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
            return os.path.relpath(os.path.join(self.ref_,filename), os.getcwd())

        print("Files to analyze:")
        print("\t run output: " + relativePathRef("dd.out"))
        print("\t             " + relativePathRef("dd.err"))

        failure()




    def getDelta0(self):
        return self.loadDeltaFileFromRep(self.ref_, True)
#        with open(os.path.join(self.ref_ ,self.getDeltaFileName()), "r") as f:
#            return f.readlines()


    def genExcludeIncludeFile(self, dirname, deltas, include=False, exclude=False):
        """Generate the *.exclude and *.include file in dirname rep from deltas"""
        excludes=self.getDelta0()
        dd=self.getDeltaFileName()

        if include:
            with open(os.path.join(dirname,dd+".include"), "w") as f:
                for d in deltas:
                    f.write(d+"\n")

        if exclude:
            with open(os.path.join(dirname,dd+".exclude"), "w") as f:
                for d in deltas:
                    excludes.remove(d)

                for line in excludes:
                    f.write(line+"\n")



    def _test(self, deltas,nbRun=None, earlyExit=True):
        if nbRun==None:
            nbRun=self.config_.get_nbRUN()
#        return self._testTab([deltas],[nbRun])[0]

        dirname=os.path.join(self.prefix_, md5Name(deltas))
        if not os.path.exists(dirname):
            os.makedirs(dirname)
            self.genExcludeIncludeFile(dirname, deltas, include=True, exclude=True)

        vT=verrouTask(dirname, self.ref_, self.run_, self.compare_ ,nbRun, self.config_.get_maxNbPROC() , self.sampleRunEnv(dirname), seedTab=self.seedTab, seedEnvVar=self.config_.get_envVarSeed())
        return vT.run(earlyExit=earlyExit)

    def _getSampleNumberToExpectFail(self, deltas):
        nbRun=self.config_.get_nbRUN()

        dirname=os.path.join(self.prefix_, md5Name(deltas))
        if not os.path.exists(dirname):
            self.internalError("_getSampleNumberToExpectFail:", dirname+" should exist")

        vT=verrouTask(dirname,None, None, None ,None, None, None)
        p=vT.getEstimatedFailProbability()
        if p==1.:
            return 1
        else:
            alpha=0.85
            return int(min( math.ceil(math.log(1-alpha) / math.log(1-p)), nbRun))




    def _testTab(self, deltasTab,nbRunTab=None, earlyExit=True, firstConfFail=False):
        nbDelta=len(deltasTab)
        if nbRunTab==None:
            nbRunTab=[self.config_.get_nbRUN()]*nbDelta

        resTab=[None] *nbDelta
        failIndexesTab=[[] for i in range(nbDelta)]
        passIndexesTab=[[] for i in range(nbDelta)]
        cacheTab=[False for i in range(nbDelta)]
        verrouTaskTab=[None] *nbDelta

        subTaskDataUnorderTab=[]

        for deltaIndex in range(nbDelta):
            deltas=deltasTab[deltaIndex]
            dirname=os.path.join(self.prefix_, md5Name(deltas))
            if not os.path.exists(dirname):
                os.makedirs(dirname)
                self.genExcludeIncludeFile(dirname, deltas, include=True, exclude=True)
            #the node is there to avoid inner/outer parallelism
            verrouTaskTab[deltaIndex]=verrouTask(dirname, self.ref_, self.run_, self.compare_ ,nbRunTab[deltaIndex], None , self.sampleRunEnv(dirname),verbose=False, seedTab=self.seedTab, seedEnvVar=self.config_.get_envVarSeed())
            workToDo=verrouTaskTab[deltaIndex].sampleToCompute(nbRunTab[deltaIndex], earlyExit=True)

            if workToDo==None:
#                resTab[deltaIndex]=(verrouTaskTab[deltaIndex].FAIL,"cache")
                resTab[deltaIndex]=verrouTaskTab[deltaIndex].FAIL
                verrouTaskTab[deltaIndex].printDir()
                print(" --(/cache/) -> FAIL")
                cacheTab[deltaIndex]=True
                continue
            cmpOnlyToDo,runToDo,cmpDone, failureIndex_unused=workToDo

            if len(cmpOnlyToDo)==0 and len(runToDo)==0: #evrything in cache
                resTab[deltaIndex]=(verrouTaskTab[deltaIndex].PASS,"cache")
                verrouTaskTab[deltaIndex].printDir()
                print(" --(/cache/) -> PASS("+ str(nbRunTab[deltaIndex])+")")
                cacheTab[deltaIndex]=True
                continue
            subTaskDataUnorderTab+=[("cmp",deltaIndex, cmpConf ) for cmpConf in cmpOnlyToDo ]
            subTaskDataUnorderTab+=[("run",deltaIndex, runConf ) for runConf in runToDo ]

        subTaskDataTab=[]

        sampleTab= [uniq_sample for uniq_sample in set([x[2] for x in subTaskDataUnorderTab])]
        sampleTab.sort()
        for sample in sampleTab:
            subTaskDataTab+=[subTaskData for subTaskData in subTaskDataUnorderTab if (subTaskData[0]=="cmp" and subTaskData[2]==sample)]
        for sample in sampleTab:
            subTaskDataTab+=[subTaskData for subTaskData in subTaskDataUnorderTab if (subTaskData[0]=="run" and subTaskData[2]==sample)]

        if len(subTaskDataTab)!=len(subTaskDataUnorderTab):
            print("internal error")
            print("subTaskDataUnorderTab", subTaskDataUnorderTab)
            print("subTaskDataTab", subTaskDataTab)
            sys.exit(42)

        activeDataTab=[True for i in subTaskDataTab]
        numThread=self.config_.get_maxNbPROC()
        import concurrent.futures
        with concurrent.futures.ThreadPoolExecutor(max_workers=numThread) as executor:
            futurePool= [executor.submit(verrouTaskTab[subTaskData[1]].submitSeq , subTaskData[0], subTaskData[2]) for subTaskData in subTaskDataTab[0:numThread]]
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
                    if res==verrouTaskTab[deltaIndex].PASS:
                        if resTab[deltaIndex]==None:
                            resTab[deltaIndex]=verrouTaskTab[deltaIndex].PASS
                        passIndexesTab[deltaIndex]+=[sampleIndex]
                    else:
                        if resTab[deltaIndex]==None and earlyExit:
                            verrouTaskTab[deltaIndex].printDir()
                            print(" --(/run/) -> FAIL(%i)"%(sampleIndex))
                        resTab[deltaIndex]=verrouTaskTab[deltaIndex].FAIL
                        failIndexesTab[deltaIndex]+=[sampleIndex]
                        if earlyExit:
                            for cWork in range(currentWork,len(subTaskDataTab)):
                                if firstConfFail or subTaskDataTab[cWork][1]==deltaIndex :
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
                            futur=executor.submit(verrouTaskTab[deltaIndex].submitSeq ,computeType, sampleIndex)
                            futurePool[poolIndex]=futur
                            newFutures+=[futur]
                            dataIndexPool[poolIndex]=currentWork
                            currentWork+=1

                poolLoop=list(toDo)+newFutures
        #affichage
        for deltaIndex in range(nbDelta):
            if cacheTab[deltaIndex]:#print already done
                continue
            if resTab[deltaIndex]==self.PASS:
                verrouTaskTab[deltaIndex].printDir()
                print(" --(/run/) -> PASS(+" + str(len(passIndexesTab[deltaIndex]))+"->"+str( max(passIndexesTab[deltaIndex])+1 )+")" )
            else:
                if not earlyExit:
                    verrouTaskTab[deltaIndex].printDir()
                    print(" --(/run/) -> FAIL(%i)"%(str(failIndexesTab[deltaIndex])[1,-1]  ))

        return resTab
