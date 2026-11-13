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
  h(S) = sum_k     lambda_k [C_k in S]               clauses C_k : the ddmin found by the delta-debug
       + sum_{k<l} mu_kl    [C_k u C_l in S]         interactions between two ddmin (mu<0 masking, mu>0 synergy)
       + eps * (|S\\U| / |{0..N-1}\\U|)^beta          diffuse background (U : union of the clauses)
The ddmin are independent when mu=0 and eps=0 : pFail(S)=1-prod_{C_k in S}(1-q_k) with q_k=1-exp(-lambda_k).

Time model of one sample : t(S)=(a + b |S|/N + c fail) * exp(sigma z) with z a standard normal variable.

The samples stopped by earlyExit do not bias the likelihood (the stopping rule depends only on the
observed results) : the fit uses the binomial likelihood of the samples in cache.
"""

import json
import math
import random
import sys
import os
from pathlib import Path

import DD_stoch

formatName="verrou_dd_metamodel"
formatVersion=1
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
    with open(str(fileName)) as f:
        for line in f:
            line=line.rstrip("\n")
            if line.startswith("ENV ") and res["tool"]==None:
                res["tool"]=line[4:].rstrip(":")
            elif line.startswith("\t") and res["argv"]==None and "=" in line:
                key,value=line.strip().split("=",1)
                if key=="PWD":
                    res["pwd"]=value
                else:
                    res["env"][key]=value
            elif line.startswith("Cmd: "):
                res["argv"]=line[len("Cmd: "):].split(" ")
                break
    return res

def argvOption(argv, option, default=None):
    """Value of the last --option=value of argv"""
    value=default
    for arg in argv:
        if arg.startswith(option):
            value=arg[len(option):]
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
    """Samples in cache of one configuration : samples is a dict sampleIndex -> (fail, time or None)"""
    def __init__(self, conf, md5=None, samples=None, origin="dd"):
        self.conf=tuple(sorted(conf))
        self.md5=md5
        self.samples={} if samples==None else samples
        self.origin=origin

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

        self.planOrigin={}
        if (self.rep / self.planFileName).is_file():
            with open(str(self.rep / self.planFileName)) as f:
                self.planOrigin=json.load(f)

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
        traceFile=argvOption(self.cmd["argv"], "--trace-file=")
        if traceFile==None:
            return None
        tracePath=Path(traceFile)
        if not tracePath.is_absolute() and self.cmd["pwd"]!=None:
            tracePath=Path(self.cmd["pwd"]) / tracePath
        return tracePath if tracePath.is_file() else None

    def _loadObservation(self, confDir):
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
        return observation(conf, confDir.name, samples, self.planOrigin.get(confDir.name, "dd"))

    def nbDelta(self):
        return len(self.deltas)

    def ddminClauses(self):
        """The ddmin found by the delta-debug, sorted by index of the link ddmin<k>"""
        ddminLinks=[name for name in self.links if name.startswith("ddmin") and name[5:].isdigit()]
        ddminLinks.sort(key=lambda name: int(name[5:]))
        return [self.observations[self.links[name]].conf for name in ddminLinks]

    def nbRun(self):
        if self.cmd==None or self.cmd["argv"]==None:
            return 5
        return int(argvOption(self.cmd["argv"], "--nruns=", 5))

    def sourceInfo(self):
        """Non-identifying description of the delta-debug which generated the cache"""
        info={"deltaKind": self.deltaFileName}
        if self.cmd!=None and self.cmd["argv"]!=None:
            info["tool"]=Path(self.cmd["argv"][0]).name
            info["options"]=[arg for arg in self.cmd["argv"][1:]
                             if any(arg.startswith(opt) for opt in ["--nruns=", "--algo=", "--rddmin=", "--num-threads=",
                                                                    "--seed=", "--dicho-", "--rddmin-tab=", "--rddmin-adaptive"])]
        return info


class metamodel:
    def __init__(self, nbDelta, clauses, lambdas, interactions=None, eps=0., beta=1., timeModel=None):
        self.nbDelta=nbDelta
        self.clauses=[frozenset(c) for c in clauses]
        self.lambdas=list(lambdas)
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

    def hazard(self, S):
        S=frozenset(S)
        inside=self.clausesIn(S)
        h=sum(self.lambdas[k] for k in inside)
        insideSet=set(inside)
        h+=sum(mu for (k,l),mu in self.interactions.items() if k in insideSet and l in insideSet)
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

    def sample(self, S, rng=random):
        """Simulate one sample : return (fail, time)"""
        fail=rng.random()<self.pFail(S)
        timeValue=self.meanTime(S, fail)
        if timeValue!=None and self.timeModel["sigma"]>0:
            timeValue*=math.exp(rng.gauss(0., self.timeModel["sigma"]))
        return fail, timeValue

    def failureJson(self):
        return {"clauses":[toRanges(c) for c in self.clauses],
                "lambda":self.lambdas,
                "pFail":[-math.expm1(-lam) for lam in self.lambdas],
                "interactions":[[k,l,mu] for (k,l),mu in sorted(self.interactions.items())],
                "background":{"eps":self.eps, "beta":self.beta, "pFailFull":-math.expm1(-self.eps)}}

    @staticmethod
    def fromJson(data):
        failure=data["failure"]
        return metamodel(data["nbDelta"], [fromRanges(c) for c in failure["clauses"]], failure["lambda"],
                         {(k,l):mu for k,l,mu in failure["interactions"]},
                         failure["background"]["eps"], failure["background"]["beta"], data.get("time"))


def loadMetamodel(fileName):
    """Return (metamodel, observations, data) from a json file written by verrou_dd_metamodel"""
    with open(str(fileName)) as f:
        data=json.load(f)
    if data.get("format")!=formatName:
        raise ValueError("%s is not a %s file"%(fileName, formatName))
    return metamodel.fromJson(data), [observation.fromJson(obs) for obs in data.get("observations",[])], data


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

def fitFailure(nbDelta, clauses, observations, interactions=True, background=True, penalty=0.1):
    """Maximum likelihood fit of the failure model. Return (metamodel, logLikelihood, nbParam).
    penalty : L2 penalty on the interaction terms mu (they are often defined by few samples)"""
    import numpy as np
    from scipy.optimize import minimize

    data=_aggregate(observations)
    template=metamodel(nbDelta, clauses, [0.]*len(clauses))
    pairs=informativePairs(observations, clauses) if interactions else []
    nbClause=len(clauses)
    nbPair=len(pairs)

    A=np.zeros((len(data), nbClause))
    P=np.zeros((len(data), nbPair))
    m=np.zeros(len(data))
    nbFail=np.array([d[1] for d in data], dtype=float)
    nbPass=np.array([d[2]-d[1] for d in data], dtype=float)
    for i,(S,_,_) in enumerate(data):
        inside=template.clausesIn(S)
        A[i,inside]=1.
        for j,(k,l) in enumerate(pairs):
            if k in inside and l in inside:
                P[i,j]=1.
        m[i]=template.backgroundFraction(S)

    def unpack(x):
        lam=x[:nbClause]
        mu=x[nbClause:nbClause+nbPair]
        eps,beta=(x[-2],x[-1]) if background else (0.,1.)
        return lam,mu,eps,beta

    def negLogLik(x):
        lam,mu,eps,beta=unpack(x)
        h=A@lam + P@mu + eps*np.power(m, beta)
        h=np.maximum(h, 1e-9)
        ll=np.sum(nbFail*np.log(-np.expm1(-h))) - np.sum(nbPass*h)
        return -ll + penalty*np.sum(mu**2)

    x0=[1.]*nbClause+[0.]*nbPair
    bounds=[(0.,30.)]*nbClause+[(-30.,30.)]*nbPair
    if background:
        x0+=[0.1, 1.]
        bounds+=[(0.,30.), (0.25, 8.)]
    if len(x0)==0:
        model=metamodel(nbDelta, clauses, [])
        return model, logLikelihood(model, observations), 0
    res=minimize(negLogLik, np.array(x0), method="L-BFGS-B", bounds=bounds)
    lam,mu,eps,beta=unpack(res.x)
    model=metamodel(nbDelta, clauses, [float(v) for v in lam],
                    {pair:float(v) for pair,v in zip(pairs,mu)}, float(eps), float(beta))
    nbParam=nbClause+nbPair+(2 if background else 0)
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


def binomialPValue(nbFail, nbSample, p):
    """Two-sided p-value of nbFail failures among nbSample samples with the failure probability p"""
    from scipy.stats import binomtest
    p=min(max(p, 1e-12), 1.-1e-12)
    return float(binomtest(nbFail, nbSample, p).pvalue)

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

def fitAll(nbDelta, clauses, observations, penalty=0.1):
    """Fit the nested models (independent ddmin, + background, + interactions), then remove greedily the
    ddmin better explained by the diffuse background (AIC decrease) : with a diffuse error, the delta-debug
    finds large ddmin with a small failure probability.
    Return the selected model, the comparison table and the indexes of the removed ddmin"""
    variants=[("independent", False, False), ("background", False, True), ("background+interactions", True, True)]
    table=[]
    model=None
    for name, interactions, background in variants:
        model, ll, nbParam=fitFailure(nbDelta, clauses, observations, interactions, background, penalty)
        table+=[{"model":name, "logLikelihood":ll, "nbParam":nbParam, "aic":2*nbParam-2*ll}]

    kept=list(range(len(clauses)))
    removed=[]
    while len(kept)!=0:
        candidates=[]
        for k in kept:
            subModel, ll, nbParam=fitFailure(nbDelta, [clauses[j] for j in kept if j!=k], observations, True, True, penalty)
            candidates+=[(2*nbParam-2*ll, k, subModel, ll, nbParam)]
        aic, k, subModel, ll, nbParam=min(candidates, key=lambda c: c[0])
        if aic>=table[-1]["aic"]:
            break
        kept.remove(k)
        removed+=[k]
        model=subModel
        table+=[{"model":"background+interactions without C"+",C".join(str(j) for j in removed),
                 "logLikelihood":ll, "nbParam":nbParam, "aic":aic}]
    return model, table, removed


## Enrichment

def enrichmentPlan(nbDelta, clauses, observations, budget, nbSample, rng):
    """Configurations to sample (each one up to nbSample samples) within budget new samples.
    Return a list of (conf, kind, cost) by decreasing priority :
      clause      : each ddmin alone (failure probability without the rest of the code)
      full, background : all the deltas, all the deltas except the ddmin
      clause+background : one ddmin and the background (independence between a ddmin and the background)
      pair        : union of two ddmin (interaction)
      random      : random subsets (shape of the background and validation)"""
    cached={obs.conf:set(obs.samples) for obs in observations}
    def cost(conf):
        done=cached.get(tuple(sorted(conf)), set())
        return len([i for i in range(nbSample) if i not in done])

    union=set().union(*[set(c) for c in clauses])
    full=tuple(range(nbDelta))
    background=tuple(i for i in full if i not in union)

    candidates=[(tuple(c), "clause") for c in clauses]
    candidates+=[(full, "full")]
    if len(background)!=0:
        candidates+=[(background, "background")]
        candidates+=[(tuple(sorted(set(background)|set(c))), "clause+background") for c in clauses]
    candidates+=[(tuple(sorted(set(clauses[k])|set(clauses[l]))), "pair")
                 for k in range(len(clauses)) for l in range(k+1, len(clauses))]

    plan=[]
    planned=set()
    def add(conf, kind):
        nonlocal budget
        conf=tuple(sorted(conf))
        if len(conf)==0 or conf in planned:
            return
        c=cost(conf)
        if c==0 or c>budget:
            return
        planned.add(conf)
        plan.append((conf, kind, c))
        budget-=c

    for conf,kind in candidates:
        add(conf, kind)

    #random subsets with a log-uniform size, alternatively in the full space and in the background
    for trial in range(10000):
        if budget<1:
            break
        space=full if (trial%2==0 or len(background)==0) else background
        size=max(1, int(round(math.exp(rng.uniform(0., math.log(len(space)))))))
        add(rng.sample(space, size), "random")
    return plan
