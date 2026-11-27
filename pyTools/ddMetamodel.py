# This file is part of Verrou, a FPU instrumentation tool.

# Copyright (C) 2014-2026 EDF
#   B. Lathuilière <bruno.lathuiliere@edf.fr>


# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the
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

"""Anonymized metamodel of a code analysed by a delta-debug (verrou_dd_*), built from a DD_stoch cache.

The metamodel does not depend on the verrou_dd_* variant : the N deltas of the search space are only
identified by their index in the ordered list ref/<deltaFile>. The order is kept because the split of
ddmin and of the dichotomy cuts the list into contiguous parts.

Failure model of one sample of the configuration S (set of indexes) : pFail(S)=1-exp(-max(h(S),0)) with
  h(S) = sum_k     lambda_k w_k(S)                   clauses C_k : the ddmin found by the delta-debug
       + sum_{k<l} mu_kl    w_k(S) w_l(S)            interactions between two ddmin (mu<0 masking, mu>0 synergy)
       + eps * (|S\\U| / |{0..N-1}\\U|)^beta          diffuse background (U : union of the clauses)
  w_k(S) = [C_k in S]                                strict clause : all the deltas of C_k are required
        or (|S n C_k| / |C_k|)^gamma_k               graded clause : each delta of C_k contributes (accumulation
                                                     of small errors), gamma_k=1 additive, gamma_k large strict
The ddmin are independent when mu=0 and eps=0 : pFail(S)=1-prod_{C_k in S}(1-q_k) with q_k=1-exp(-lambda_k).

The clauses can overlap : with the cause a and (b or c) the delta-debug finds {a,b} or {a,c}, and {a,c} can
not be found once a is removed by rddmin. The alternative clauses are found by the enrichment
(substitutionSearch) or come from other delta-debug caches of the same search space (ddCache.merge).

Time model of one sample : t(S)=(a + b |S|/N + c fail) * exp(sigma z) with z a standard normal variable.

The samples stopped by earlyExit do not bias the likelihood (the stopping rule depends only on the
observed results) : the fit uses the binomial likelihood of the samples in cache.
"""

import importlib
import json
import math
import random
import shlex
import sys
import os
from pathlib import Path

#DD_stoch is only imported to read a cache (ddCache) : the simulation scripts of the tests, run for each
#sample, only use the metamodel

formatName="verrou_dd_metamodel"
formatVersion=2 #2 : graded clauses (failure.gamma)
linkNames=["FullPerturbation", "NoPerturbation", "rddmin-cmp", "ddmax", "ddmax-cmp"]


def isMd5Name(name):
    return len(name)==32 and all(c in "0123456789abcdef" for c in name)

def loadDeltaLines(fileName):
    """Same parsing as DDStoch.loadDeltaFile"""
    with open(str(fileName)) as f:
        return [x.rstrip() for x in f.readlines()]

def toRanges(conf):
    """[0,1,2,5,7,8] -> [[0,2],[5,5],[7,8]] (inclusive intervals)"""
    res=[]
    for i in sorted(conf):
        if len(res)!=0 and res[-1][1]==i-1:
            res[-1][1]=i
        else:
            res+=[[i,i]]
    return res

def fromRanges(ranges):
    return tuple(i for (a,b) in ranges for i in range(a,b+1))


def parseCmdLast(fileName):
    """Parse the cmd.last file written by gen_config.saveParam : return a dict with the tool path,
    the command line (argv), the working directory and the recorded environment variables"""
    res={"tool":None, "argv":None, "pwd":None, "env":{}}
    oldFormat=False
    with open(str(fileName)) as f:
        for line in f:
            line=line.rstrip("\n")
            if line.startswith("ENV ") and res["tool"]==None:
                res["tool"]=line[4:].rstrip(":")
            elif line.startswith("\t") and res["argv"]==None and "=" in line:
                key,value=line.strip().split("=",1)
                if key=="PWD": #cmd.last without Cwd: line (old format)
                    oldFormat=True
                    if res["pwd"]==None:
                        res["pwd"]=value
                else:
                    res["env"][key]=value
            elif line.startswith("Cwd: "):
                res["pwd"]=line[len("Cwd: "):]
            elif line.startswith("Cmd: "):
                cmdLine=line[len("Cmd: "):]
                #the old format does not quote the arguments : split on the blanks (exact without blank in an argument)
                res["argv"]=cmdLine.split() if oldFormat else shlex.split(cmdLine)
                break
    return res

def argvOption(argv, option, default=None):
    """Value of the last --option=value of argv"""
    value=default
    for arg in argv:
        if arg.startswith(option):
            value=arg[len(option):]
    return value

ddEnvPrefixes=["INTERFLOP_DD", "VERROU_DD"] #config_keys of the verrou_dd_* tools

def cmdOption(cmd, option, envName, default=None):
    """Value of an option of the delta-debug recorded in cmd.last (dict of parseCmdLast) : as in gen_config,
    the environment variables <prefix>_<envName> override the command line, the last prefix wins"""
    value=argvOption(cmd["argv"], option, default)
    for prefix in ddEnvPrefixes:
        value=cmd["env"].get(prefix+"_"+envName, value)
    return value


def loadTraceRunTimes(traceFiles):
    """Durations of the run tasks of ddTrace files : dict (md5, sampleIndex) -> seconds"""
    res={}
    for traceFile in traceFiles:
        begin={}
        with open(str(traceFile)) as f:
            for line in f:
                try:
                    ev=json.loads(line)
                except json.JSONDecodeError:
                    continue
                if ev["ev"] not in ["task_begin","task_end"] or ev.get("kind")!="run" or ev.get("sample")==None:
                    continue
                key=(ev["conf"], ev["sample"])
                if ev["ev"]=="task_begin":
                    begin[key]=ev["t"]
                elif key in begin:
                    res[key]=ev["t"]-begin.pop(key)
    return res


class observation:
    """Samples in cache of one configuration : samples is a dict sampleIndex -> (fail, time or None).
    origin : "dd" (delta-debug), kind of the enrichment plan, or "dd+"kind for a configuration of the
    delta-debug with new samples ; ddSamples : indexes of the samples computed by the delta-debug"""
    def __init__(self, conf, md5=None, samples=None, origin="dd", ddSamples=None):
        self.conf=tuple(sorted(conf))
        self.md5=md5
        self.samples={} if samples==None else samples
        self.origin=origin
        if ddSamples==None:
            ddSamples=set(self.samples) if origin=="dd" else set()
        self.ddSamples=ddSamples

    def nbSample(self):
        return len(self.samples)

    def nbFail(self):
        return len([s for s in self.samples.values() if s[0]])

    def toJson(self):
        return {"conf":toRanges(self.conf), "origin":self.origin,
                "samples":[[index, int(fail), timeValue] for index,(fail,timeValue) in sorted(self.samples.items())]}

    @staticmethod
    def fromJson(data):
        return observation(fromRanges(data["conf"]),
                           samples={s[0]:(bool(s[1]), s[2]) for s in data["samples"]},
                           origin=data.get("origin","dd"))


class ddCache:
    """Generic reading of a DD_stoch cache (the same format for all the verrou_dd_* variants)"""

    planFileName="metamodel_plan.json"
    residualKey="residualClauses"
    alternativeKey="alternativeClauses"
    mergeSampleShift=1000000 #shift of the sample indexes of the merged caches

    def __init__(self, rep, traceFiles=[], verbose=True):
        self.rep=Path(rep)
        if not (self.rep / "ref").is_dir():
            raise ValueError("%s is not a delta-debug cache (no ref directory)"%(self.rep))
        self.cmd=None
        if (self.rep / "cmd.last").is_file():
            self.cmd=parseCmdLast(self.rep / "cmd.last")
        self.deltaFileName=self._findDeltaFileName()
        self.deltas=loadDeltaLines(self.rep / "ref" / self.deltaFileName)
        self.index={}
        for i,delta in enumerate(self.deltas):
            self.index.setdefault(delta, i)

        traceFiles=list(traceFiles)
        traceFromCmd=self._traceFileFromCmd()
        if traceFromCmd!=None and traceFromCmd not in traceFiles:
            traceFiles+=[traceFromCmd]
        self.traceTimes=loadTraceRunTimes(traceFiles)

        self.planOrigin={} #md5 -> kind (or dict kind, ddSamples) of the configurations of the enrichment
        self.residualMd5=[] #localized causes of the residual found by the enrichment (residualSearch)
        self.alternativeMd5=[] #alternative clauses found by the enrichment (substitutionSearch)
        self.merged=[] #other caches of the same search space (merge)
        self.mergedDdmin=[] #ddmin of the merged caches
        if (self.rep / self.planFileName).is_file():
            with open(str(self.rep / self.planFileName)) as f:
                plan=json.load(f)
            self.residualMd5=plan.pop(self.residualKey, [])
            self.alternativeMd5=plan.pop(self.alternativeKey, [])
            self.planOrigin=plan

        self.observations={} #md5 -> observation
        self.nbIgnoredConf=0
        for confDir in sorted(self.rep.iterdir()):
            if confDir.is_symlink() or not confDir.is_dir() or not isMd5Name(confDir.name):
                continue
            obs=self._loadObservation(confDir)
            if obs!=None:
                self.observations[confDir.name]=obs
        if verbose and self.nbIgnoredConf!=0:
            print("Warning : %i configurations with deltas outside the search space are ignored"%(self.nbIgnoredConf))

        self.links={}
        for link in self.rep.iterdir():
            if link.is_symlink():
                target=Path(os.readlink(str(link))).name
                if target in self.observations:
                    self.links[link.name]=target

    def _findDeltaFileName(self):
        for confDir in self.rep.iterdir():
            if confDir.is_dir() and isMd5Name(confDir.name):
                for includeFile in confDir.glob("*.include"):
                    deltaFileName=includeFile.name[:-len(".include")]
                    if (self.rep / "ref" / deltaFileName).is_file():
                        return deltaFileName
        raise ValueError("%s : no configuration in the cache"%(self.rep))

    def _traceFileFromCmd(self):
        if self.cmd==None or self.cmd["argv"]==None:
            return None
        traceFile=cmdOption(self.cmd, "--trace-file=", "TRACE_FILE")
        if traceFile==None:
            return None
        tracePath=Path(traceFile)
        if not tracePath.is_absolute() and self.cmd["pwd"]!=None:
            tracePath=Path(self.cmd["pwd"]) / tracePath
        return tracePath if tracePath.is_file() else None

    def _loadObservation(self, confDir):
        import DD_stoch
        includeFile=confDir / (self.deltaFileName+".include")
        if not includeFile.is_file():
            return None
        conf=[]
        for delta in loadDeltaLines(includeFile):
            if delta not in self.index:
                self.nbIgnoredConf+=1
                return None
            conf+=[self.index[delta]]
        samples={}
        for runDir in confDir.glob(DD_stoch.subDirRun+"[0-9]*"):
            returnValuePath=runDir / DD_stoch.ddReturnFileName
            if not returnValuePath.is_file():
                continue
            sampleIndex=int(runDir.name[len(DD_stoch.subDirRun):])
            with open(str(returnValuePath)) as f:
                fail=int(f.readline())!=0
            timeValue=None
            timePath=runDir / DD_stoch.ddTimeFileName
            if timePath.is_file():
                with open(str(timePath)) as f:
                    timeValue=float(f.readline())
            else:
                timeValue=self.traceTimes.get((confDir.name, sampleIndex))
            samples[sampleIndex]=(fail, timeValue)
        origin,ddSamples=self._origin(confDir.name, samples)
        return observation(conf, confDir.name, samples, origin, ddSamples)

    def _origin(self, md5, samples):
        """Origin of a configuration and indexes of its samples computed by the delta-debug"""
        plan=self.planOrigin.get(md5)
        if plan==None:
            #the delta-debug computes at most --nruns= samples : the others come from an enrichment
            #recorded before the ddSamples field of the plan
            nbRun=self.nbRun()
            ddSamples=set(i for i in samples if i<nbRun)
            return ("dd" if len(ddSamples)==len(samples) else "dd+enrichment"), ddSamples
        if isinstance(plan, str):
            return plan, set()
        return "dd+"+plan["kind"], set(plan["ddSamples"]) & set(samples)

    def nbDelta(self):
        return len(self.deltas)

    def md5(self, conf):
        """Name of the directory of a configuration (set of indexes)"""
        import DD_stoch
        return DD_stoch.md5Name([self.deltas[i] for i in conf])

    def merge(self, other):
        """Add the samples and the ddmin of another delta-debug cache of the same search space (same code,
        another seed or other options) : the ddmin found by the two delta-debug can be different assemblies
        of the same cause. The sample indexes of the other cache are shifted : the samples of two caches
        are supposed to be different draws"""
        if sorted(other.deltas)!=sorted(self.deltas):
            raise ValueError("%s and %s do not have the same search space"%(self.rep, other.rep))
        self.merged+=[other]
        self._mergeObservations(len(self.merged), other)
        known=set(self.ddminClauses())
        for conf in other.ddminClauses():
            conf=self._confFrom(other, conf)
            if conf not in known:
                known.add(conf)
                self.mergedDdmin+=[(len(self.merged), conf)]

    def _confFrom(self, other, conf):
        return tuple(sorted(self.index[other.deltas[i]] for i in conf))

    def _mergeObservations(self, rank, other, md5Tab=None):
        shift=rank*self.mergeSampleShift
        for obsOther in other.observations.values():
            conf=self._confFrom(other, obsOther.conf)
            md5=self.md5(conf)
            if md5Tab!=None and md5 not in md5Tab:
                continue
            obs=self.observations.get(md5)
            if obs==None:
                obs=observation(conf, md5, {}, obsOther.origin, set())
                self.observations[md5]=obs
            for index,sample in obsOther.samples.items():
                obs.samples[shift+index]=sample
            obs.ddSamples|=set(shift+index for index in obsOther.ddSamples)

    def ddminClauses(self):
        """The ddmin found by the delta-debug, sorted by index of the link ddmin<k>, then the ddmin of the
        merged caches"""
        ddminLinks=[name for name in self.links if name.startswith("ddmin") and name[5:].isdigit()]
        ddminLinks.sort(key=lambda name: int(name[5:]))
        return [self.observations[self.links[name]].conf for name in ddminLinks]+[conf for rank,conf in self.mergedDdmin]

    def _planClauses(self, md5Tab, known):
        res=[]
        for md5 in md5Tab:
            if md5 in self.observations and self.observations[md5].conf not in known+res:
                res+=[self.observations[md5].conf]
        return res

    def residualClauses(self):
        """Localized causes of the residual found by the enrichment (not found by the delta-debug)"""
        return self._planClauses(self.residualMd5, self.ddminClauses())

    def alternativeClauses(self):
        """Alternative clauses found by the enrichment : a part of a ddmin and its substitute"""
        return self._planClauses(self.alternativeMd5, self.ddminClauses()+self.residualClauses())

    def clauses(self):
        """Clauses of the metamodel : ddmin of the delta-debug, residual clauses then alternative clauses"""
        return self.ddminClauses()+self.residualClauses()+self.alternativeClauses()

    def clauseNames(self):
        nbOwn=len(self.ddminClauses())-len(self.mergedDdmin)
        return (["ddmin%i"%(k) for k in range(nbOwn)]
                +["ddmin(cache%i)"%(rank) for rank,conf in self.mergedDdmin]
                +["residual%i"%(k) for k in range(len(self.residualClauses()))]
                +["alternative%i"%(k) for k in range(len(self.alternativeClauses()))])

    def savePlan(self):
        plan=dict(self.planOrigin)
        if len(self.residualMd5)!=0:
            plan[self.residualKey]=self.residualMd5
        if len(self.alternativeMd5)!=0:
            plan[self.alternativeKey]=self.alternativeMd5
        with open(str(self.rep / self.planFileName),"w") as f:
            json.dump(plan, f, indent=1)

    def reloadObservations(self, md5Tab):
        """Read again the samples of some configurations (after a computation)"""
        for md5 in md5Tab:
            confDir=self.rep / md5
            if confDir.is_dir():
                obs=self._loadObservation(confDir)
                if obs!=None:
                    self.observations[md5]=obs
        for rank,other in enumerate(self.merged):
            self._mergeObservations(rank+1, other, set(md5Tab))

    def nbRun(self):
        if self.cmd==None or self.cmd["argv"]==None:
            return 5 #default of dd_config
        return int(cmdOption(self.cmd, "--nruns=", "NRUNS", 5))

    def sourceInfo(self):
        """Non-identifying description of the delta-debug which generated the cache"""
        info={"deltaKind": self.deltaFileName}
        if len(self.merged)!=0:
            info["nbCache"]=1+len(self.merged)
        if self.cmd!=None and self.cmd["argv"]!=None:
            info["tool"]=Path(self.cmd["argv"][0]).name
            info["options"]=[arg for arg in self.cmd["argv"][1:]
                             if any(arg.startswith(opt) for opt in ["--nruns=", "--algo=", "--rddmin=", "--num-threads=",
                                                                    "--seed=", "--dicho-", "--rddmin-tab=", "--rddmin-adaptive"])]
        return info


class metamodel:
    def __init__(self, nbDelta, clauses, lambdas, interactions=None, eps=0., beta=1., timeModel=None, gammas=None):
        self.nbDelta=nbDelta
        self.clauses=[frozenset(c) for c in clauses]
        self.lambdas=list(lambdas)
        self.gammas=[None]*len(self.clauses) if gammas==None else list(gammas) #None : strict clause
        self.interactions=dict(interactions) if interactions!=None else {} #(k,l) -> mu with k<l
        self.eps=eps
        self.beta=beta
        self.timeModel=timeModel #dict a, b, c, sigma or None
        self.union=frozenset().union(*self.clauses)
        self.nbBackground=max(1, nbDelta-len(self.union))

    def clausesIn(self, S):
        return [k for k,clause in enumerate(self.clauses) if clause<=S]

    def backgroundFraction(self, S):
        return len(S-self.union)/self.nbBackground

    def weight(self, k, S):
        """Weight of the clause k in the hazard of S : 1 when the clause is included in S"""
        clause=self.clauses[k]
        if clause<=S:
            return 1.
        if self.gammas[k]==None:
            return 0.
        nb=len(clause&S)
        return (nb/len(clause))**self.gammas[k] if nb!=0 else 0.

    def hazard(self, S):
        S=frozenset(S)
        w=[self.weight(k,S) for k in range(len(self.clauses))]
        h=sum(lam*wk for lam,wk in zip(self.lambdas, w))
        h+=sum(mu*w[k]*w[l] for (k,l),mu in self.interactions.items())
        m=self.backgroundFraction(S)
        if m>0:
            h+=self.eps*m**self.beta
        return h

    def pFail(self, S):
        return -math.expm1(-max(self.hazard(S), 0.))

    def meanTime(self, S, fail=False):
        if self.timeModel==None:
            return None
        tm=self.timeModel
        return max(0., tm["a"] + tm["b"]*len(S)/self.nbDelta + tm["c"]*float(fail))

    def sample(self, S, rng=random, pFail=None, size=None):
        """Simulate one sample : return (fail, time). pFail and size (|S|) avoid the computation from S"""
        fail=rng.random()<(self.pFail(S) if pFail==None else pFail)
        timeValue=self.meanTime(S if size==None else range(size), fail)
        if timeValue!=None and self.timeModel["sigma"]>0:
            timeValue*=math.exp(rng.gauss(0., self.timeModel["sigma"]))
        return fail, timeValue

    def failureJson(self):
        return {"clauses":[toRanges(c) for c in self.clauses],
                "lambda":self.lambdas,
                "gamma":self.gammas,
                "pFail":[-math.expm1(-lam) for lam in self.lambdas],
                "interactions":[[k,l,mu] for (k,l),mu in sorted(self.interactions.items())],
                "background":{"eps":self.eps, "beta":self.beta, "pFailFull":-math.expm1(-self.eps)}}

    @staticmethod
    def fromJson(data):
        failure=data["failure"]
        return metamodel(data["nbDelta"], [fromRanges(c) for c in failure["clauses"]], failure["lambda"],
                         {(k,l):mu for k,l,mu in failure["interactions"]},
                         failure["background"]["eps"], failure["background"]["beta"], data.get("time"),
                         failure.get("gamma"))


def loadMetamodel(fileName, observations=True):
    """Return (metamodel, observations, data) from a json file written by verrou_dd_metamodel
    observations=False : the observations are not decoded (costly for large search spaces), [] is returned"""
    with open(str(fileName)) as f:
        data=json.load(f)
    if data.get("format")!=formatName:
        raise ValueError("%s is not a %s file"%(fileName, formatName))
    obsTab=[observation.fromJson(obs) for obs in data.get("observations",[])] if observations else []
    return metamodel.fromJson(data), obsTab, data

def slimMetamodelJson(data):
    """Only the keys of a metamodel json used by metamodel.fromJson (without the observations)"""
    return {key:data[key] for key in ["format", "version", "nbDelta", "failure", "time"] if key in data}


## Fit

def _aggregate(observations):
    return [(frozenset(obs.conf), obs.nbFail(), obs.nbSample()) for obs in observations if obs.nbSample()!=0]

def informativePairs(observations, clauses):
    """Pairs (k,l) of clauses for which an observation contains exactly these two clauses"""
    clauseSets=[frozenset(c) for c in clauses]
    res=set()
    for S,nbFail,nbSample in _aggregate(observations):
        inside=[k for k,c in enumerate(clauseSets) if c<=S]
        if len(inside)==2:
            res.add(tuple(inside))
    return sorted(res)

def logLikelihood(model, observations):
    res=0.
    for S,nbFail,nbSample in _aggregate(observations):
        h=max(model.hazard(S), 1e-9)
        res+=nbFail*math.log(-math.expm1(-h)) - (nbSample-nbFail)*h
    return res

_scipyWarning=True

def _scipy(module):
    """Import a scipy module, None without scipy (a warning is printed once)"""
    global _scipyWarning
    try:
        return importlib.import_module("scipy."+module)
    except ImportError:
        if _scipyWarning:
            print("Warning : scipy not found, the internal optimizer and statistical tests are used", file=sys.stderr)
            _scipyWarning=False
        return None

class _boxResult:
    def __init__(self, x, fun):
        self.x=x
        self.fun=fun

def _minimizeBox(fun, x0, lower, upper, maxIter=15000, pgtol=1e-5, ftol=2.2e-9, memory=10):
    """Minimization of fun (return the value and the gradient) on the box [lower, upper] without scipy :
    projected L-BFGS (the variables at a bound with a gradient pointing outside are fixed, the direction of
    the free variables is the L-BFGS one) with a backtracking line search on the projected path. Same
    stopping criteria as scipy L-BFGS-B (projected gradient, relative decrease of the function)"""
    import numpy as np
    def project(x):
        return np.minimum(np.maximum(x, lower), upper)
    x=project(np.array(x0, dtype=float))
    f,g=fun(x)
    sTab=[]
    yTab=[]
    for it in range(maxIter):
        if np.max(np.abs(project(x-g)-x))<=pgtol:
            break
        fixed=((x<=lower) & (g>0)) | ((x>=upper) & (g<0))
        q=np.where(fixed, 0., g)
        coefs=[]
        for sv,yv in reversed(list(zip(sTab, yTab))):
            rho=1./float(yv@sv)
            a=rho*float(sv@q)
            coefs.append((rho, a, sv, yv))
            q=q-a*yv
        if len(sTab)!=0:
            q*=float(sTab[-1]@yTab[-1])/float(yTab[-1]@yTab[-1])
        else:
            q/=max(np.max(np.abs(q)), 1.)
        for rho,a,sv,yv in reversed(coefs):
            q=q+(a-rho*float(yv@q))*sv
        d=-np.where(fixed, 0., q)
        if float(d@g)>=0: #not a descent direction : steepest descent and new memory
            d=-np.where(fixed, 0., g)
            sTab=[]
            yTab=[]
        step=1.
        while True:
            xNew=project(x+step*d)
            fNew,gNew=fun(xNew)
            if np.isfinite(fNew) and fNew<=f+1e-4*float(g@(xNew-x)):
                break
            step*=0.5
            if step<1e-14:
                return _boxResult(x, f)
        sv=xNew-x
        yv=gNew-g
        if float(sv@yv)>1e-12*float(yv@yv):
            sTab=(sTab+[sv])[-memory:]
            yTab=(yTab+[yv])[-memory:]
        decrease=(f-fNew)/max(abs(f), abs(fNew), 1.)
        x,f,g=xNew,fNew,gNew
        if decrease<=ftol:
            break
    return _boxResult(x, f)

def fitFailure(nbDelta, clauses, observations, interactions=True, background=True, penalty=0.1, init=None, graded=()):
    """Maximum likelihood fit of the failure model. Return (metamodel, logLikelihood, nbParam).
    penalty : L2 penalty on the interaction terms mu (they are often defined by few samples)
    graded : clauses (of size>=2) fitted as graded clauses (exponent gamma), the others are strict
    init : metamodel used as warm start (parameters of the same clauses and pairs, the others at 0) :
    when init is a nested model, the fit can not be worse than init. The default start is also tried."""
    import numpy as np
    optimize=_scipy("optimize")

    data=_aggregate(observations)
    template=metamodel(nbDelta, clauses, [0.]*len(clauses))
    pairs=informativePairs(observations, clauses) if interactions else []
    nbClause=len(clauses)
    nbPair=len(pairs)
    gradedSet=set(frozenset(c) for c in graded)
    gradedIndex=[k for k,c in enumerate(template.clauses) if c in gradedSet and len(c)>=2]
    nbGraded=len(gradedIndex)

    F=np.zeros((len(data), nbClause)) #fraction of each clause in each configuration
    m=np.zeros(len(data))
    nbFail=np.array([d[1] for d in data], dtype=float)
    nbPass=np.array([d[2]-d[1] for d in data], dtype=float)
    for i,(S,_,_) in enumerate(data):
        for k,clause in enumerate(template.clauses):
            F[i,k]=len(clause&S)/len(clause)
        m[i]=template.backgroundFraction(S)
    A=(F==1.).astype(float) #weights of the strict clauses
    logF=np.log(np.where((F>0)&(F<1), F, 1.))
    logM=np.log(np.where(m>0, m, 1.))
    pairK=[k for k,l in pairs]
    pairL=[l for k,l in pairs]

    def unpack(x):
        lam=x[:nbClause]
        mu=x[nbClause:nbClause+nbPair]
        gam=x[nbClause+nbPair:nbClause+nbPair+nbGraded]
        eps,beta=(x[-2],x[-1]) if background else (0.,1.)
        return lam,mu,gam,eps,beta

    def negLogLik(x):
        """Penalized negative log likelihood and its gradient"""
        lam,mu,gam,eps,beta=unpack(x)
        W=A
        if nbGraded!=0:
            W=A.copy()
            for j,k in enumerate(gradedIndex):
                W[:,k]=np.where(F[:,k]>0, np.power(F[:,k], gam[j]), 0.)
        P=W[:,pairK]*W[:,pairL]
        mBeta=np.where(m>0, np.power(m, beta), 0.)
        hRaw=W@lam + P@mu + eps*mBeta
        h=np.maximum(hRaw, 1e-9)
        ll=np.sum(nbFail*np.log(-np.expm1(-h))) - np.sum(nbPass*h)
        dh=np.where(hRaw>1e-9, nbFail*np.exp(-h)/(-np.expm1(-h)) - nbPass, 0.) #d ll / d h
        gradGamma=[]
        for j,k in enumerate(gradedIndex):
            dhdW=lam[k]+sum(mu[p]*W[:,l if k==kp else kp] for p,(kp,l) in enumerate(pairs) if k in (kp,l))
            gradGamma+=[-(dh@(W[:,k]*logF[:,k]*dhdW))]
        grad=np.concatenate([-(dh@W), -(dh@P)+2.*penalty*mu, gradGamma])
        if background:
            grad=np.concatenate([grad, [-(dh@mBeta), -eps*(dh@(mBeta*logM))]])
        return -ll + penalty*np.sum(mu**2), grad

    gammaBounds=(0.25, 50.)
    bounds=[(0.,30.)]*nbClause+[(-30.,30.)]*nbPair+[gammaBounds]*nbGraded
    tail=[0.1, 1.] if background else []
    if background:
        bounds+=[(0.,30.), (0.25, 8.)]
    if len(bounds)==0:
        model=metamodel(nbDelta, clauses, [])
        return model, logLikelihood(model, observations), 0
    starts=[[1.]*nbClause+[0.]*nbPair+[2.]*nbGraded+tail]
    if init!=None:
        initLambda={c:lam for c,lam in zip(init.clauses, init.lambdas)}
        initGamma={c:gamma for c,gamma in zip(init.clauses, init.gammas) if gamma!=None}
        initMu={(init.clauses[k],init.clauses[l]):mu for (k,l),mu in init.interactions.items()}
        xInit=[initLambda.get(c, 0.) for c in template.clauses]
        xInit+=[initMu.get((template.clauses[k],template.clauses[l]), initMu.get((template.clauses[l],template.clauses[k]), 0.)) for k,l in pairs]
        if background:
            tail=[init.eps, init.beta]
        #a large gamma is a strict clause with a null gradient : two starts for the new graded clauses
        for gammaStart in [1., 6.]:
            start=xInit+[initGamma.get(template.clauses[k], gammaStart) for k in gradedIndex]+tail
            if start not in starts:
                starts+=[start]
    lower=np.array([lo for lo,hi in bounds])
    upper=np.array([hi for lo,hi in bounds])
    def minimize(start):
        x0=np.clip(np.array(start, dtype=float), lower, upper)
        if optimize==None:
            return _minimizeBox(negLogLik, x0, lower, upper)
        return optimize.minimize(negLogLik, x0, method="L-BFGS-B", jac=True, bounds=bounds)
    res=min([minimize(start) for start in starts], key=lambda r: r.fun)
    lam,mu,gam,eps,beta=unpack(res.x)
    gammas=[None]*nbClause
    for j,k in enumerate(gradedIndex):
        gammas[k]=float(gam[j])
    model=metamodel(nbDelta, clauses, [float(v) for v in lam],
                    {pair:float(v) for pair,v in zip(pairs,mu)}, float(eps), float(beta), gammas=gammas)
    nbParam=nbClause+nbPair+nbGraded+(2 if background else 0)
    return model, logLikelihood(model, observations), nbParam

def fitTime(nbDelta, observations):
    """Least square fit of t=a + b |S|/N + c fail, sigma : standard deviation of log(t/tmodel)"""
    import numpy as np
    rows=[(len(obs.conf)/nbDelta, float(fail), t) for obs in observations for fail,t in obs.samples.values() if t!=None]
    if len(rows)<3:
        return None
    X=np.array([[1., s, f] for s,f,t in rows])
    T=np.array([t for s,f,t in rows])
    usedColumns=[0]+[j for j in [1,2] if np.ptp(X[:,j])>0]
    coef=np.zeros(3)
    coef[usedColumns]=np.linalg.lstsq(X[:,usedColumns], T, rcond=None)[0]
    pred=np.maximum(X@coef, 1e-12)
    sigma=float(np.std(np.log(np.maximum(T,1e-12)/pred)))
    return {"a":float(coef[0]), "b":float(coef[1]), "c":float(coef[2]), "sigma":sigma, "nbSample":len(rows)}


def binomialLogPmf(k, nbSample, p):
    """Logarithm of the probability of k failures among nbSample samples (failure probability 0<p<1)"""
    return (math.lgamma(nbSample+1)-math.lgamma(k+1)-math.lgamma(nbSample-k+1)
            +k*math.log(p)+(nbSample-k)*math.log1p(-p))

def binomialPValue(nbFail, nbSample, p):
    """Two-sided p-value of nbFail failures among nbSample samples with the failure probability p"""
    p=min(max(p, 1e-12), 1.-1e-12)
    stats=_scipy("stats")
    if stats!=None:
        return float(stats.binomtest(nbFail, nbSample, p).pvalue)
    #exact test without scipy : sum of the probabilities of the results not more likely than nbFail
    threshold=binomialLogPmf(nbFail, nbSample, p)+math.log1p(1e-7) #relative tolerance of scipy
    logPmfTab=[binomialLogPmf(k, nbSample, p) for k in range(nbSample+1)]
    return min(1., sum(math.exp(lp) for lp in logPmfTab if lp<=threshold))

def diagnostics(model, observations):
    """Comparison of the observations with the model, sorted by increasing p-value"""
    res=[]
    for index,obs in enumerate(observations):
        if obs.nbSample()==0:
            continue
        p=model.pFail(frozenset(obs.conf))
        res+=[{"observation":index, "size":len(obs.conf), "origin":obs.origin,
               "clauses":model.clausesIn(frozenset(obs.conf)),
               "nbSample":obs.nbSample(), "nbFail":obs.nbFail(), "pFail":p,
               "pValue":binomialPValue(obs.nbFail(), obs.nbSample(), p)}]
    res.sort(key=lambda d: d["pValue"])
    return res

def fitAll(nbDelta, clauses, observations, penalty=0.1, minBetaRemoval=1., maxMergeCandidate=60, withBackground=True):
    """Fit the nested models (independent ddmin, + background, + interactions), each one started from the
    previous one, and select the smallest AIC. Then, greedily while the AIC decreases :
      merge   : two clauses are replaced by their union as a graded clause. With an accumulation of small
                errors the delta-debug splits the cause in several ddmin (not the same ones with another
                seed) : each one fails rarely alone, and their unions fail more than independent ddmin.
                With many clauses only the maxMergeCandidate pairs of closest clauses are tried.
      graded  : a clause of size>=2 becomes a graded clause : the samples of the parts of the clause
                (1-minimality tests of the delta-debug, enrichment) show that the failure does not require
                all its deltas.
      removal : a clause better explained by the diffuse background is removed : with a diffuse error, the
                delta-debug finds large ddmin with a small failure probability. A removal is only accepted
                if the background stays convex (beta>=minBetaRemoval) : with beta<1 the small subsets fail
                more than proportionally, so the background would absorb a localized cause and spread it
                on all the deltas.
    Without withBackground the diffuse background is ignored (eps=0) : the nested models are the independent
    ddmin with and without interactions, and no clause is removed.
    Return the selected model, the comparison table, the indexes of the removed clauses and the merged
    clauses (lists of indexes). The indexes C<k> of the table are the ones of the parameter clauses"""
    clauses=[tuple(sorted(c)) for c in clauses]
    variants=[("independent", False, False), ("background", False, True), ("background+interactions", True, True)]
    if not withBackground:
        variants=[("independent", False, False), ("interactions", True, False)]
    table=[]
    best=None #(aic, model, interactions, background)
    model=None
    for name, interactions, background in variants:
        model, ll, nbParam=fitFailure(nbDelta, clauses, observations, interactions, background, penalty, init=model)
        aic=2*nbParam-2*ll
        table+=[{"model":name, "logLikelihood":ll, "nbParam":nbParam, "aic":aic}]
        if best==None or aic<best[0]:
            best=(aic, model, interactions, background)
    bestAic, model, interactions, background=best

    current=list(clauses)
    groups=[[k] for k in range(len(clauses))] #indexes of the parameter clauses of each current clause
    graded=[] #graded clauses of current
    def groupName(group):
        return "+".join("C%i"%(k) for k in group)
    def select(candidates, action):
        """Apply the candidate (aic, model, logLikelihood, nbParam, current, groups, graded, name) with the
        smallest AIC if it is better than the selected model"""
        nonlocal bestAic, model, current, groups, graded
        if len(candidates)==0:
            return False
        aic, subModel, ll, nbParam, newCurrent, newGroups, newGraded, name=min(candidates, key=lambda c: c[0])
        if aic>=bestAic:
            return False
        bestAic, model, current, groups, graded=aic, subModel, newCurrent, newGroups, newGraded
        table.append({"model":action+" "+name, "logLikelihood":ll, "nbParam":nbParam, "aic":aic})
        return True
    def candidate(newCurrent, newGroups, newGraded, name, withInteractions=None, withBackground=None, init=None):
        subModel, ll, nbParam=fitFailure(nbDelta, newCurrent, observations,
                                         interactions if withInteractions==None else withInteractions,
                                         background if withBackground==None else withBackground,
                                         penalty, init=model if init==None else init, graded=newGraded)
        return (2*nbParam-2*ll, subModel, ll, nbParam, newCurrent, newGroups, newGraded, name)

    while True:
        pairs=[(a,b) for a in range(len(current)) for b in range(a+1, len(current))]
        if len(pairs)>maxMergeCandidate:
            def gap(pair):
                return min(abs(i-j) for i in current[pair[0]] for j in current[pair[1]])
            pairs=sorted(pairs, key=gap)[:maxMergeCandidate]
        candidates=[]
        for a,b in pairs:
            union=tuple(sorted(set(current[a])|set(current[b])))
            #warm start : the union with the sum of the two clauses
            init=metamodel(nbDelta, model.clauses+[union], model.lambdas+[model.lambdas[a]+model.lambdas[b]],
                           model.interactions, model.eps, model.beta, gammas=model.gammas+[None])
            candidates+=[candidate([union if i==a else c for i,c in enumerate(current) if i!=b],
                                   [groups[a]+groups[b] if i==a else g for i,g in enumerate(groups) if i!=b],
                                   [g for g in graded if g not in (current[a], current[b])]+[union],
                                   groupName(groups[a]+groups[b]), init=init)]
        if not select(candidates, "merge"):
            break

    while True:
        candidates=[candidate(current, groups, graded+[c], groupName(groups[k]))
                    for k,c in enumerate(current) if len(c)>=2 and c not in graded]
        if not select(candidates, "graded"):
            break

    if len(graded)!=0 and not interactions:
        #the interactions were compared with strict clauses
        if select([candidate(current, groups, graded, "", True, withBackground)], "interactions"):
            interactions=True

    removed=[]
    while withBackground:
        candidates=[]
        for k in range(len(current)):
            c=candidate([c for i,c in enumerate(current) if i!=k], [g for i,g in enumerate(groups) if i!=k],
                        [g for g in graded if g!=current[k]], groupName(groups[k]), withBackground=True)
            if c[1].beta>=minBetaRemoval:
                candidates+=[c+(groups[k],)]
        if len(candidates)==0:
            break
        removedGroup=min(candidates, key=lambda c: c[0])[-1]
        if not select([c[:-1] for c in candidates], "removal"):
            break
        removed+=removedGroup
        background=True
    for line in table:
        line["selected"]=(line["aic"]==bestAic)
    return model, table, removed, [group for group in groups if len(group)>1]


## Enrichment

def missingSamples(obs, nbSample):
    """Number of samples to compute to get the nbSample first samples of an observation (None : not in cache)"""
    if obs==None:
        return nbSample
    return len([i for i in range(nbSample) if i not in obs.samples])

def clauseParts(clause, maxLeaveOneOut=6, nbRandom=2):
    """Parts of a clause sampled to fit a graded clause : the two contiguous halves, the clause without one
    delta (at most maxLeaveOneOut deltas) and random parts (3/4 of the clause). The draw only depends on
    the clause : the parts are the same for each enrichment"""
    clause=tuple(sorted(clause))
    if len(clause)<2:
        return []
    rng=random.Random("clauseParts"+str(clause))
    res=list(splitHalves(clause))
    removed=clause if len(clause)<=maxLeaveOneOut else rng.sample(clause, maxLeaveOneOut)
    res+=[tuple(i for i in clause if i!=r) for r in sorted(removed)]
    if len(clause)>=8:
        res+=[tuple(sorted(rng.sample(clause, (3*len(clause))//4))) for i in range(nbRandom)]
    unique=[]
    for part in res:
        if part not in unique:
            unique+=[part]
    return unique

def enrichmentPlan(nbDelta, clauses, observations, budget, nbSample, rng, residualFactor=4, pairsRatio=0.5, withRandom=True, partsRatio=0.25):
    """Configurations to sample within budget new samples.
    Return a list of (conf, kind, cost, nbSampleConf) by decreasing priority :
      clause      : each clause alone (failure probability without the rest of the code)
      full        : all the deltas
      residual    : complement of the clauses (residualFactor*nbSample samples : the failure probability of the
                    residual decides if the final check of rddmin passes) and its two contiguous halves
                    (localized or diffuse residual, see residualSearch)
      clause+background : one clause and the background (independence between a clause and the background)
      clause part : parts of the clauses of size>=2 (clauseParts), at most partsRatio*budget samples : graded
                    or strict clause
      pair        : union of two clauses (interaction), at most pairsRatio*budget samples, the most
                    informative pairs first (failure probability under independence close to 1/2)
      random      : random subsets (shape of the background and validation), only with withRandom"""
    cached={obs.conf:obs for obs in observations}
    def cost(conf, nb):
        return missingSamples(cached.get(conf), nb)

    union=set().union(*[set(c) for c in clauses])
    full=tuple(range(nbDelta))
    background=tuple(i for i in full if i not in union)

    plan=[]
    planned=set()
    def add(conf, kind, nb=nbSample, maxCost=None):
        nonlocal budget
        conf=tuple(sorted(conf))
        if len(conf)==0 or conf in planned:
            return 0
        c=cost(conf, nb)
        if c==0 or c>budget or (maxCost!=None and c>maxCost):
            return 0
        planned.add(conf)
        plan.append((conf, kind, c, nb))
        budget-=c
        return c

    for c in clauses:
        add(c, "clause")
    add(full, "full")
    if len(background)!=0:
        add(background, "residual", residualFactor*nbSample)
        for half in splitHalves(background):
            add(half, "residual", residualFactor*nbSample)
        for c in clauses:
            add(set(background)|set(c), "clause+background")

    partsBudget=int(partsRatio*budget)
    partsTab=[clauseParts(c) for c in clauses]
    for rank in range(max([len(parts) for parts in partsTab]+[0])): #the halves of each clause first
        for parts in partsTab:
            if rank<len(parts):
                partsBudget-=add(parts[rank], "clause part", maxCost=partsBudget)

    def pFailEstimate(conf):
        obs=cached.get(tuple(sorted(conf)))
        return 0.5 if obs==None else (obs.nbFail()+0.5)/(obs.nbSample()+1.)
    q=[pFailEstimate(c) for c in clauses]
    pairs=[(k,l) for k in range(len(clauses)) for l in range(k+1, len(clauses))]
    def information(pair):
        p=1.-(1.-q[pair[0]])*(1.-q[pair[1]])
        return p*(1.-p)
    pairs.sort(key=information, reverse=True)
    pairBudget=int(pairsRatio*budget)
    for k,l in pairs:
        pairBudget-=add(set(clauses[k])|set(clauses[l]), "pair", maxCost=pairBudget)

    #random subsets with a log-uniform size, alternatively in the full space and in the background
    for trial in range(10000 if withRandom else 0):
        if budget<1:
            break
        space=full if (trial%2==0 or len(background)==0) else background
        size=max(1, int(round(math.exp(rng.uniform(0., math.log(len(space)))))))
        add(rng.sample(space, size), "random")
    return plan


## Residual : failures outside the clauses

def splitHalves(conf):
    """Two contiguous halves in the order of the search space (as DD.split)"""
    conf=tuple(sorted(conf))
    return conf[:len(conf)//2], conf[len(conf)//2:]

def proportionTest(nbFail1, nbSample1, nbFail2, nbSample2):
    """Likelihood ratio statistic of two different failure probabilities (chi2 with 1 degree of freedom)"""
    def ll(f, n, p):
        res=0.
        if f>0:
            res+=f*math.log(p)
        if n-f>0:
            res+=(n-f)*math.log(1.-p)
        return res
    if nbSample1==0 or nbSample2==0:
        return 0.
    p=(nbFail1+nbFail2)/(nbSample1+nbSample2)
    return 2.*(ll(nbFail1, nbSample1, nbFail1/nbSample1)+ll(nbFail2, nbSample2, nbFail2/nbSample2)
               -ll(nbFail1, nbSample1, p)-ll(nbFail2, nbSample2, p))

def binomialSf(nbFail, nbSample, p):
    """Probability of at least nbFail failures among nbSample samples (failure probability p)"""
    if nbFail<=0:
        return 1.
    if p<=0.:
        return 0.
    if p>=1.:
        return 1.
    return min(1., sum(math.exp(binomialLogPmf(k, nbSample, p)) for k in range(nbFail, nbSample+1)))

def clopperPearson(nbFail, nbSample, alpha=0.05):
    """Exact confidence interval of a failure probability"""
    stats=_scipy("stats")
    if stats!=None:
        low=0. if nbFail==0 else float(stats.beta.ppf(alpha/2, nbFail, nbSample-nbFail+1))
        high=1. if nbFail==nbSample else float(stats.beta.ppf(1-alpha/2, nbFail+1, nbSample-nbFail))
        return low, high
    def solve(func, target):
        """p such that func(p)=target, func increasing (bisection)"""
        a,b=0.,1.
        for i in range(60):
            mid=0.5*(a+b)
            if func(mid)<target:
                a=mid
            else:
                b=mid
        return 0.5*(a+b)
    #low : P(X>=nbFail | low)=alpha/2 ; high : P(X<=nbFail | high)=alpha/2, i.e. P(X>=nbFail+1 | high)=1-alpha/2
    low=0. if nbFail==0 else solve(lambda p: binomialSf(nbFail, nbSample, p), alpha/2)
    high=1. if nbFail==nbSample else solve(lambda p: binomialSf(nbFail+1, nbSample, p), 1.-alpha/2)
    return low, high

def residualSearch(nbDelta, clauses, observations, nbSample, residualFactor=4, threshold=3.84, context=(), residual=None):
    """Dichotomy on the residual R (complement of the clauses) with the samples in cache.
    Level 0 : R and its two contiguous halves are sampled (nbR=residualFactor*nbSample samples each) :
      R passes -> "pass" ; the halves fail with different probabilities (likelihood ratio > threshold)
      -> "localized" and the dichotomy continues in the failing half, otherwise -> "diffuse".
    Next levels (the node is known to fail) : the halves are sampled with earlyExit (as the delta-debug) :
      only one half fails -> continue in this half ; both or none (cause across the split) or a node of
      size 1 -> the node is a localized cause (clause).
    With a context (deltas added to each configuration of the dichotomy, see substitutionSearch) and a given
    residual, the dichotomy looks for the deltas of the residual which fail with the context : level 0
    passes when R with the context (only nbSample samples : this screening is done for each part of each
    clause) does not fail more than R alone. The next configurations are sampled until a failure is
    missed with a probability below 1% (at most nbR samples) : few samples for a context which fails often.
    Return a dict : verdict ("pass", "diffuse", "localized", "incomplete" when samples are missing),
    complement (R and its counters), levels, clause (localized cause with the context or None), node (the
    localized cause without the context) and toCompute (list of (conf, nbSample, earlyExit) required to
    continue)"""
    cached={obs.conf:obs for obs in observations}
    nbR=residualFactor*nbSample
    contextSet=set(context)
    def withContext(node):
        return tuple(sorted(set(node)|contextSet)) if len(contextSet)!=0 else tuple(node)
    def counters(node):
        obs=cached.get(withContext(node))
        return (0,0) if obs==None else (obs.nbFail(), obs.nbSample())
    nbNode=nbR #number of samples of the nodes of the dichotomy
    def complete(node, earlyExit):
        f,n=counters(node)
        return n>=nbNode or (earlyExit and f>0)

    if residual==None:
        union=set().union(*[set(c) for c in clauses])
        residual=tuple(i for i in range(nbDelta) if i not in union)
    residual=tuple(sorted(set(residual)-contextSet))
    res={"verdict":"incomplete", "complement":None, "levels":[], "clause":None, "node":None, "toCompute":[]}
    if len(residual)==0:
        res["verdict"]="pass"
        return res
    f,n=counters(residual)
    res["complement"]={"size":len(residual), "nbFail":f, "nbSample":n}
    alone=cached.get(residual)
    fAlone,nAlone=(0,0) if alone==None else (alone.nbFail(), alone.nbSample())
    if len(contextSet)==0:
        missing=[(c, nbR, False) for c in (residual,)+splitHalves(residual) if not complete(c, False)]
    else:
        #the halves are only sampled if R with the context fails more than R alone
        missing=[(withContext(residual), nbSample, False)] if n<nbSample else []
        if nAlone<nbR:
            missing+=[(residual, nbR, False)]
    if len(missing)!=0:
        res["toCompute"]=missing
        return res
    if f==0:
        res["verdict"]="pass"
        return res
    if len(contextSet)!=0:
        res["complement"]["alone"]={"nbFail":fAlone, "nbSample":nAlone}
        if f/n<=fAlone/nAlone or proportionTest(f, n, fAlone, nAlone)<=threshold:
            res["verdict"]="pass"
            return res
        excess=min(max(clopperPearson(f, n)[0]-fAlone/nAlone, 0.05), 0.99)
        nbNode=min(nbR, max(4, int(math.ceil(math.log(0.01)/math.log(1.-excess)))))
    res["nbSampleNode"]=nbNode

    node=residual
    while True:
        if len(node)==1:
            res["verdict"]="localized"
            res["clause"]=withContext(node)
            res["node"]=node
            return res
        earlyExit=node is not residual
        halves=splitHalves(node)
        missing=[(withContext(c), nbNode, earlyExit) for c in halves if not complete(c, earlyExit)]
        if len(missing)!=0:
            res["toCompute"]=missing
            return res
        (fA,nA),(fB,nB)=counters(halves[0]), counters(halves[1])
        level={"size":len(node), "halves":[{"size":len(halves[0]), "nbFail":fA, "nbSample":nA},
                                           {"size":len(halves[1]), "nbFail":fB, "nbSample":nB}]}
        res["levels"].append(level)
        if not earlyExit:
            level["statistic"]=proportionTest(fA, nA, fB, nB)
            if level["statistic"]<=threshold or fA==fB:
                level["decision"]="same failure probability"
                res["verdict"]="diffuse"
                return res
            node=halves[0] if fA/nA>fB/nB else halves[1]
        elif (fA>0)!=(fB>0):
            node=halves[0] if fA>0 else halves[1]
        else:
            level["decision"]="both halves fail" if fA>0 else "both halves pass"
            res["verdict"]="localized"
            res["clause"]=withContext(node)
            res["node"]=node
            return res
        level["decision"]="first half" if node==halves[0] else "second half"


## Substitution : alternative clauses

def substitutionCandidates(clause, maxSingle=4):
    """Parts of a clause for which a substitute is searched : each delta of a small clause, the two
    contiguous halves of a large one"""
    clause=tuple(sorted(clause))
    if len(clause)<2:
        return []
    if len(clause)<=maxSingle:
        return [(i,) for i in clause]
    return list(splitHalves(clause))

def substitutionSearch(clause, removed, residual, observations, nbSample, residualFactor=4, threshold=3.84):
    """Search of a substitute of a part of a clause in the residual R : with the cause a and (b or c), the
    delta-debug finds the ddmin {a,b}, and c is a substitute of b in the residual.
    The core (the clause without the removed part) is the context of a dichotomy on R (residualSearch) :
    the localized cause J is a substitute when the alternative clause K=core u J fails more than the core
    alone and than J alone (likelihood ratio > threshold) : at least nbSample samples each.
    Return a dict : verdict ("none" : no substitute, "alternative", "incomplete" when samples are missing),
    clause (the alternative clause K), core, search (result of residualSearch) and toCompute"""
    cached={obs.conf:obs for obs in observations}
    nbR=residualFactor*nbSample
    def counters(conf):
        obs=cached.get(tuple(sorted(conf)))
        return (0,0) if obs==None else (obs.nbFail(), obs.nbSample())
    core=tuple(sorted(set(clause)-set(removed)))
    res={"verdict":"incomplete", "clause":None, "core":core, "search":None, "toCompute":[]}
    if len(core)==0 or len(residual)==0:
        res["verdict"]="none"
        return res
    search=residualSearch(None, None, observations, nbSample, residualFactor, threshold, context=core, residual=residual)
    res["search"]=search
    if search["verdict"]=="incomplete":
        res["toCompute"]=search["toCompute"]
        return res
    if search["verdict"]!="localized":
        res["verdict"]="none"
        return res
    alternative,node=search["clause"], search["node"]
    nbAccept=max(nbSample, search["nbSampleNode"])
    missing=[(conf, nbAccept, False) for conf in [alternative, core, node] if counters(conf)[1]<nbAccept]
    if len(missing)!=0:
        res["toCompute"]=missing
        return res
    (fK,nK),(fCore,nCore),(fNode,nNode)=counters(alternative), counters(core), counters(node)
    res["counters"]={"alternative":[fK,nK], "core":[fCore,nCore], "substitute":[fNode,nNode]}
    if all(fK/nK>f/n and proportionTest(fK, nK, f, n)>threshold for f,n in [(fCore,nCore), (fNode,nNode)]):
        res["verdict"]="alternative"
        res["clause"]=alternative
    else:
        res["verdict"]="none"
    return res


## Merge : unions of the clauses of the fitted model

def mergePlan(model, observations, nbSample):
    """Configurations (conf, nbSample) to sample to decide the merge of the clauses of an accumulation
    (fitAll) : the union of the graded clauses, then the unions of two clauses with at least one graded
    clause (the graded pairs first). Only the configurations with missing samples are returned"""
    cached={obs.conf:obs for obs in observations}
    gradedIndex=[k for k,gamma in enumerate(model.gammas) if gamma!=None]
    confs=[]
    if len(gradedIndex)>=3:
        confs+=[frozenset().union(*[model.clauses[k] for k in gradedIndex])]
    pairs=[(k,l) for k in range(len(model.clauses)) for l in range(k+1, len(model.clauses)) if k in gradedIndex or l in gradedIndex]
    pairs.sort(key=lambda pair: (pair[0] not in gradedIndex)+(pair[1] not in gradedIndex))
    confs+=[model.clauses[k]|model.clauses[l] for k,l in pairs]
    res=[]
    for conf in confs:
        conf=tuple(sorted(conf))
        if missingSamples(cached.get(conf), nbSample)!=0 and (conf, nbSample) not in res:
            res+=[(conf, nbSample)]
    return res
