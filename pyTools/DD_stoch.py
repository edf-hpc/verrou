
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
from valgrind import DD
import glob
import datetime

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

    def __init__(self, dirname, refDir,runCmd, cmpCmd,nbRun, maxNbPROC, runEnv):
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

        print(os.path.relpath(self.dirname, os.getcwd()),end="")

    def nameDir(self,i):
        return  os.path.join(self.dirname,"dd.run%i" % (i))

    def mkdir(self,i):
         os.mkdir(self.nameDir(i))
    def rmdir(self,i):
        shutil.rmtree(self.nameDir(i))

    def runOneSample(self,i):
        rundir= self.nameDir(i)

        self.subProcessRun[i]=runCmdAsync([self.runCmd, rundir],
                                          os.path.join(rundir,"dd.run"),
                                          self.runEnv)

    def cmpOneSample(self,i, assertRun=True):
        rundir= self.nameDir(i)
        if assertRun:
            if self.subProcessRun[i]!=None:
                getResult(self.subProcessRun[i])
        retval = runCmd([self.cmpCmd, self.refDir, rundir],
                        os.path.join(rundir,"dd.compare"))

        with open(os.path.join(self.dirname, rundir, "dd.return.value"),"w") as f:
            f.write(str(retval))
        if retval != 0:
            print("FAIL(%d)" % i)
            return self.FAIL
        else:
            return self.PASS

    def sampleToComputeToGetFailure(self, nbRun):
        """Return the two lists of samples which have to be compared or computed (and compared) to perforn nbRun Success run : None means Failure ([],[]) means Success """
        listOfDirString=[runDir for runDir in os.listdir(self.dirname) if runDir.startswith("dd.run")]
        listOfDirIndex=[ int(x.replace("dd.run",""))  for x in listOfDirString  ]

        cmpDone=[]
        runDone=[]
        workToCmpOnly=[]
        for runDir in listOfDirString:

            returnValuePath=os.path.join(self.dirname, runDir, "dd.return.value")
            if os.path.exists(returnValuePath):
                statusCmp=int((open(returnValuePath).readline()))
                if statusCmp!=0:
                    return None
                cmpDone+=[int(runDir.replace("dd.run",""))]
            else:
                runPath=os.path.join(self.dirname, runDir, "dd.run.out")
                if os.path.exists(runPath):
                    runDone+=[int(runDir.replace("dd.run",""))]

        workToRun= [x for x in range(nbRun) if (((not x in runDone+cmpDone) and (x in listOfDirIndex )) or (not (x in listOfDirIndex))) ]
        return (runDone, workToRun, cmpDone)

    def run(self):
        workToDo=self.sampleToComputeToGetFailure(self.nbRun)
        if workToDo==None:
            print(" --(cache) -> FAIL")
            return self.FAIL
        cmpOnlyToDo=workToDo[0]
        runToDo=workToDo[1]
        cmpDone=workToDo[2]

        if len(cmpOnlyToDo)==0 and len(runToDo)==0:
            print(" --(cache)-> PASS("+str(self.nbRun)+")")
            return self.PASS

        if len(cmpOnlyToDo)!=0:
            print(" --( cmp )-> ",end="",flush=True)
            returnVal=self.cmpSeq(cmpOnlyToDo)
            if returnVal==self.FAIL:
                return self.FAIL
            else:
                print("PASS(+" + str(len(cmpOnlyToDo))+"->"+str(len(cmpDone) +len(cmpOnlyToDo))+")" , end="", flush=True)

        if len(runToDo)!=0:
            print(" --( run )-> ",end="",flush=True)

            if self.maxNbPROC==None:
                returnVal=self.runSeq(runToDo)
            else:
                returnVal=self.runPar(runToDo)

            if(returnVal==self.PASS):
                print("PASS(+" + str(len(runToDo))+"->"+str( len(cmpOnlyToDo) +len(cmpDone) +len(runToDo) )+")" )
            return returnVal
        else:
            print("")
        return self.PASS



    def cmpSeq(self,workToDo):
        for run in workToDo:
            retVal=self.cmpOneSample(run,assertRun=False)
            if retVal=="FAIL":
                return self.FAIL
        return self.PASS


    def runSeq(self,workToDo):

        for run in workToDo:
            if not os.path.exists(self.nameDir(run)):
                self.mkdir(run)
            else:
                print("Manual cache modification detected (runSeq)")

            self.runOneSample(run)
            retVal=self.cmpOneSample(run)

            if retVal=="FAIL":
                return self.FAIL
        return self.PASS

    def runPar(self,workToDo):
        import concurrent.futures
        with concurrent.futures.ThreadPoolExecutor(max_workers=self.maxNbPROC) as executor:
            futures={executor.submit(self.runSeq, [work]) for work in workToDo}
            concurrent.futures.wait(futures)
        if self.FAIL in [futur.result() for futur in futures]:
            return self.FAIL
        return self.PASS


def md5Name(deltas):
    copyDeltas=copy.copy(deltas)
    copyDeltas.sort()
    return hashlib.md5(("".join(copyDeltas)).encode('utf-8')).hexdigest()


def prepareOutput(dirname):
     shutil.rmtree(dirname, ignore_errors=True)
     os.makedirs(dirname)


def failure():
    sys.exit(42)





class DDStoch(DD.DD):
    def __init__(self, config, prefix):
        DD.DD.__init__(self)
        self.config_=config
        if not  self.config_.get_quiet():
            print("delta debug options :")
            print(self.config_.optionToStr())

        self.run_ =  self.config_.get_runScript()
        self.compare_ = self.config_.get_cmpScript()
        self.cache_outcomes = False # the cache of DD.DD is ignored
        self.index=0
        self.prefix_ = os.path.join(os.getcwd(),prefix)
        self.relPrefix_=prefix
        self.ref_ = os.path.join(self.prefix_, "ref")
        self.prepareCache()
        self.rddminHeuristicLoadRep()
        prepareOutput(self.ref_)
        self.reference()
        self.mergeList()


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


    def rddminHeuristicLoadRep(self):
        """ Load the results of previous ddmin. Need to be called after prepareCache"""

        self.useRddminHeuristic=False
        if self.config_.get_rddminHeuristicsCache() !="none" or len(self.config_.get_rddminHeuristicsRep_Tab())!=0:
            self.useRddminHeuristic=True
        if self.useRddminHeuristic==False:
            return

        rddmin_heuristic_rep=[]
        if "cache" in self.config_.get_rddminHeuristicsCache():
            if self.config_.get_cache=="rename":
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
        if self.config_.get_cache=="continue":
            self.ddminHeuristic+=[ (open(os.path.join(rep, self.getDeltaFileName()+".include"))).readlines()  for rep in self.saveCleanSymLink if "ddmin" in rep]

        for rep in rddmin_heuristic_rep:
            repTab=glob.glob(os.path.join(rep, "ddmin*"))
            self.ddminHeuristic+=[ (open(os.path.join(rep, self.getDeltaFileName()+".include"))).readlines()  for rep in repTab]

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


    def mergeList(self):
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
            with open(os.path.join(dirname,excludeFile), "r") as f:
                for line in f.readlines():
                    if line not in excludeMerged:
                        excludeMerged+=[line]
        with open(os.path.join(dirname, name), "w" )as f:
            for line in excludeMerged:
                f.write(line)


    def testWithLink(self, deltas, linkname):
        testResult=self._test(deltas)
        dirname = os.path.join(self.prefix_, md5Name(deltas))
        self.symlink(dirname, os.path.join(self.prefix_,linkname))
        return testResult

    def report_progress(self, c, title):
        if not self.config_.get_quiet:
            super().report_progress(c,title)

    def configuration_found(self, kind_str, delta_config,verbose=True):
        if verbose:
            print("%s (%s):"%(kind_str,self.coerce(delta_config)))
        self.testWithLink(delta_config, kind_str)

    def run(self, deltas=None):

        # get the search space
        if deltas==None:
            deltas=self.getDelta0()

        if(len(deltas)==0):
            emptySearchSpaceFailure()

        #basic verification
        testResult=self._test(deltas)
        self.configuration_found("FullPerturbation",deltas)
        if testResult!=self.FAIL:
            self.fullPerturbationSucceedsFailure()

        testResult=self._test([])
        self.configuration_found("NoPerturbation",[])
        if testResult!=self.PASS:
            self.noPerturbationFailsFailure()


        #select the right variant of algo and apply it
        algo=self.config_.get_ddAlgo()
        resConf=None

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

        if algo=="ddmax":
            resConf= self.DDMax(deltas)
        else:
            if resConf!=None:
                flatRes=[c  for conf in resConf for c in conf]
                cmp= [delta for delta in deltas if  delta not in flatRes ]
                self.configuration_found("rddmin-cmp", cmp)

        return resConf

    def applyRddminWithHeuristics(self,deltas, algo):
        """Test the previous ddmin configuration (previous run of DD_stoch) as a filter to rddmin algo"""

        res=[]

        for heuristicsDelta in self.ddminHeuristic:
            if all(x in deltas for x in  heuristicsDelta): #check inclusion
                testResult=self._test(heuristicsDelta, self.config_.get_nbRUN())
                if testResult!=self.FAIL:
                    if not self.config_.get_quiet():
                        print("Bad rddmin heuristic : %s"%self.coerce(heuristicsDelta))
                else:
                    #print("Good rddmin heuristics : %s"%self.coerce(heuristicsDelta))
                    if len(heuristicsDelta)==1:
                        res+=[heuristicsDelta]
                        self.configuration_found("ddmin%d"%(self.index), heuristicsDelta)
                        self.index+=1
                        deltas=[delta for delta in deltas if delta not in heuristicsDelta]
                    else:
                        resTab=algo(heuristicsDelta)
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

    def splitDeltas(self, deltas,nbRun,granularity):
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
        print("Dichotomy split done")

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



    def SRDDMin(self, deltas,runTab):#runTab=rddMinTab):
        #name for progression
        algo_name="SRDDMin"
        #assert with the right nbRun number
        nbRun=runTab[-1]
        testResult=self._test(deltas,nbRun)
        if testResult!=self.FAIL:
            self.internalError("SRDDMIN", md5Name(deltas)+" should fail")

        ddminTab=[]

        #increasing number of run
        for run in runTab:
            testResult=self._test(deltas,run)

            #rddmin loop
            while testResult==self.FAIL:
                self.report_progress(deltas, algo_name)
                conf = self.verrou_dd_min(deltas,run)
                if len(conf)!=1:
                    #may be not minimal due to number of run)
                    for runIncValue in [x for x in runTab if x>run ]:
                        conf = self.verrou_dd_min(conf,runIncValue)
                        if len(conf)==1:
                            break

                ddminTab += [conf]
                self.configuration_found("ddmin%d"%(self.index), conf)
                #print("ddmin%d (%s):"%(self.index,self.coerce(conf)))
                self.index+=1
                #update search space
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
        print("\t1) check the correctness of the %s script : the failure criteria may be too large"%self.compare_)
        print("\t2) check if the number of samples (option --nruns=) is sufficient ")

        print("Directory to analyze: FullPerturbation")
        failure()

    def noPerturbationFailsFailure(self):
        print("FAILURE: the comparison between the reference (code instrumented with nearest mode) andthe code without instrumentation failed")

        print("Suggestions:")
        print("\t1) check if reproducibilty discrepancies are larger than the failure criteria of the script %s"%self.compare_)
        print("\t2) check the libm library is not instrumented")
        print("Directory to analyze: NoPerturbation")
        failure()

    def referenceFailsFailure(self):
        print("FAILURE: the reference is not valid ")
        print("Suggestions:")
        print("\t1) check the correctness of the %s script"%self.compare_)

        print("Files to analyze:")
        print("\t run output: " +  os.path.join(self.ref_,"dd.out") + " " + os.path.join(self.ref_,"dd.err"))
        print("\t cmp output: " +  os.path.join(self.ref_,"checkRef.out") + " "+ os.path.join(self.ref_,"checkRef.err"))
        failure()

    def referenceRunFailure(self):
        print("FAILURE: the reference run fails ")
        print("Suggestions:")
        print("\t1) check the correctness of the %s script"%self.run_)

        print("Files to analyze:")
        print("\t run output: " +  os.path.join(self.ref_,"dd.out") + " " + os.path.join(self.ref_,"dd.err"))
        failure()




    def getDelta0(self):
        with open(os.path.join(self.ref_ ,self.getDeltaFileName()), "r") as f:
            return f.readlines()


    def genExcludeIncludeFile(self, dirname, deltas, include=False, exclude=False):
        """Generate the *.exclude and *.include file in dirname rep from deltas"""
        excludes=self.getDelta0()
        dd=self.getDeltaFileName()

        if include:
            with open(os.path.join(dirname,dd+".include"), "w") as f:
                for d in deltas:
                    f.write(d)

        if exclude:
            with open(os.path.join(dirname,dd+".exclude"), "w") as f:
                for d in deltas:
                    excludes.remove(d)

                for line in excludes:
                    f.write(line)


    def _test(self, deltas,nbRun=None):
        if nbRun==None:
            nbRun=self.config_.get_nbRUN()

        dirname=os.path.join(self.prefix_, md5Name(deltas))
        if not os.path.exists(dirname):
            os.makedirs(dirname)
            self.genExcludeIncludeFile(dirname, deltas, include=True, exclude=True)

        vT=verrouTask(dirname, self.ref_, self.run_, self.compare_ ,nbRun, self.config_.get_maxNbPROC() , self.sampleRunEnv(dirname))

        return vT.run()
