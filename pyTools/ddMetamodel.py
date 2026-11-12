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
        if (self.rep / self.planFileName).is_file():
            with open(str(self.rep / self.planFileName)) as f:
                plan=json.load(f)
            self.residualMd5=plan.pop(self.residualKey, [])
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
                target=Path(link.readlink()).name
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

    def ddminClauses(self):
        """The ddmin found by the delta-debug, sorted by index of the link ddmin<k>"""
        ddminLinks=[name for name in self.links if name.startswith("ddmin") and name[5:].isdigit()]
        ddminLinks.sort(key=lambda name: int(name[5:]))
        return [self.observations[self.links[name]].conf for name in ddminLinks]

    def residualClauses(self):
        """Localized causes of the residual found by the enrichment (not found by the delta-debug)"""
        ddmin=set(self.ddminClauses())
        return [self.observations[md5].conf for md5 in self.residualMd5
                if md5 in self.observations and self.observations[md5].conf not in ddmin]

    def clauses(self):
        """Clauses of the metamodel : ddmin of the delta-debug then residual clauses"""
        return self.ddminClauses()+self.residualClauses()

    def clauseNames(self):
        return ["ddmin%i"%(k) for k in range(len(self.ddminClauses()))]+["residual%i"%(k) for k in range(len(self.residualClauses()))]

    def savePlan(self):
        plan=dict(self.planOrigin)
        if len(self.residualMd5)!=0:
            plan[self.residualKey]=self.residualMd5
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

def fitFailure(nbDelta, clauses, observations, interactions=True, background=True, penalty=0.1, init=None):
    """Maximum likelihood fit of the failure model. Return (metamodel, logLikelihood, nbParam).
    penalty : L2 penalty on the interaction terms mu (they are often defined by few samples)
    init : metamodel used as warm start (parameters of the same clauses and pairs, the others at 0) :
    when init is a nested model, the fit can not be worse than init. The default start is also tried."""
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
    logM=np.log(np.where(m>0, m, 1.))

    def unpack(x):
        lam=x[:nbClause]
        mu=x[nbClause:nbClause+nbPair]
        eps,beta=(x[-2],x[-1]) if background else (0.,1.)
        return lam,mu,eps,beta

    def negLogLik(x):
        """Penalized negative log likelihood and its gradient"""
        lam,mu,eps,beta=unpack(x)
        mBeta=np.where(m>0, np.power(m, beta), 0.)
        hRaw=A@lam + P@mu + eps*mBeta
        h=np.maximum(hRaw, 1e-9)
        ll=np.sum(nbFail*np.log(-np.expm1(-h))) - np.sum(nbPass*h)
        dh=np.where(hRaw>1e-9, nbFail*np.exp(-h)/(-np.expm1(-h)) - nbPass, 0.) #d ll / d h
        grad=np.concatenate([-(dh@A), -(dh@P)+2.*penalty*mu])
        if background:
            grad=np.concatenate([grad, [-(dh@mBeta), -eps*(dh@(mBeta*logM))]])
        return -ll + penalty*np.sum(mu**2), grad

    x0=[1.]*nbClause+[0.]*nbPair
    bounds=[(0.,30.)]*nbClause+[(-30.,30.)]*nbPair
    if background:
        x0+=[0.1, 1.]
        bounds+=[(0.,30.), (0.25, 8.)]
    if len(x0)==0:
        model=metamodel(nbDelta, clauses, [])
        return model, logLikelihood(model, observations), 0
    starts=[np.array(x0)]
    if init!=None:
        initLambda={c:lam for c,lam in zip(init.clauses, init.lambdas)}
        initMu={(init.clauses[k],init.clauses[l]):mu for (k,l),mu in init.interactions.items()}
        xInit=[initLambda.get(c, 0.) for c in template.clauses]
        xInit+=[initMu.get((template.clauses[k],template.clauses[l]), initMu.get((template.clauses[l],template.clauses[k]), 0.)) for k,l in pairs]
        if background:
            xInit+=[init.eps, init.beta]
        starts+=[np.clip(np.array(xInit), [lo for lo,hi in bounds], [hi for lo,hi in bounds])]
    res=min([minimize(negLogLik, start, method="L-BFGS-B", jac=True, bounds=bounds) for start in starts], key=lambda r: r.fun)
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

def fitAll(nbDelta, clauses, observations, penalty=0.1, minBetaRemoval=1.):
    """Fit the nested models (independent ddmin, + background, + interactions), each one started from the
    previous one, and select the smallest AIC. Then remove greedily the ddmin better explained by the
    diffuse background (AIC decrease relatively to the best model) : with a diffuse error, the delta-debug
    finds large ddmin with a small failure probability. A removal is only accepted if the background stays
    convex (beta>=minBetaRemoval) : with beta<1 the small subsets fail more than proportionally, so the
    background would absorb a localized cause and spread it on all the deltas.
    Return the selected model, the comparison table and the indexes of the removed ddmin"""
    variants=[("independent", False, False), ("background", False, True), ("background+interactions", True, True)]
    table=[]
    best=None #(aic, model, interactions)
    model=None
    for name, interactions, background in variants:
        model, ll, nbParam=fitFailure(nbDelta, clauses, observations, interactions, background, penalty, init=model)
        aic=2*nbParam-2*ll
        table+=[{"model":name, "logLikelihood":ll, "nbParam":nbParam, "aic":aic}]
        if best==None or aic<best[0]:
            best=(aic, model, interactions)

    bestAic, model, interactions=best
    name="background+interactions" if interactions else "background"
    kept=list(range(len(clauses)))
    removed=[]
    while len(kept)!=0:
        candidates=[]
        for k in kept:
            subModel, ll, nbParam=fitFailure(nbDelta, [clauses[j] for j in kept if j!=k], observations, interactions, True, penalty, init=model)
            if subModel.beta>=minBetaRemoval:
                candidates+=[(2*nbParam-2*ll, k, subModel, ll, nbParam)]
        if len(candidates)==0:
            break
        aic, k, subModel, ll, nbParam=min(candidates, key=lambda c: c[0])
        if aic>=bestAic:
            break
        kept.remove(k)
        removed+=[k]
        model=subModel
        bestAic=aic
        table+=[{"model":name+" without C"+",C".join(str(j) for j in removed),
                 "logLikelihood":ll, "nbParam":nbParam, "aic":aic}]
    for line in table:
        line["selected"]=(line["aic"]==bestAic)
    return model, table, removed


## Enrichment

def enrichmentPlan(nbDelta, clauses, observations, budget, nbSample, rng, residualFactor=4, pairsRatio=0.5, withRandom=True):
    """Configurations to sample within budget new samples.
    Return a list of (conf, kind, cost, nbSampleConf) by decreasing priority :
      clause      : each clause alone (failure probability without the rest of the code)
      full        : all the deltas
      residual    : complement of the clauses (residualFactor*nbSample samples : the failure probability of the
                    residual decides if the final check of rddmin passes) and its two contiguous halves
                    (localized or diffuse residual, see residualSearch)
      clause+background : one clause and the background (independence between a clause and the background)
      pair        : union of two clauses (interaction), at most pairsRatio*budget samples, the most
                    informative pairs first (failure probability under independence close to 1/2)
      random      : random subsets (shape of the background and validation), only with withRandom"""
    cached={obs.conf:obs for obs in observations}
    def cost(conf, nb):
        obs=cached.get(conf)
        done=set() if obs==None else set(obs.samples)
        return len([i for i in range(nb) if i not in done])

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

def clopperPearson(nbFail, nbSample, alpha=0.05):
    from scipy.stats import beta
    low=0. if nbFail==0 else float(beta.ppf(alpha/2, nbFail, nbSample-nbFail+1))
    high=1. if nbFail==nbSample else float(beta.ppf(1-alpha/2, nbFail+1, nbSample-nbFail))
    return low, high

def residualSearch(nbDelta, clauses, observations, nbSample, residualFactor=4, threshold=3.84):
    """Dichotomy on the residual R (complement of the clauses) with the samples in cache.
    Level 0 : R and its two contiguous halves are sampled (nbR=residualFactor*nbSample samples each) :
      R passes -> "pass" ; the halves fail with different probabilities (likelihood ratio > threshold)
      -> "localized" and the dichotomy continues in the failing half, otherwise -> "diffuse".
    Next levels (the node is known to fail) : the halves are sampled with earlyExit (as the delta-debug) :
      only one half fails -> continue in this half ; both or none (cause across the split) or a node of
      size 1 -> the node is a localized cause (clause).
    Return a dict : verdict ("pass", "diffuse", "localized", "incomplete" when samples are missing),
    complement (R and its counters), levels, clause (localized cause or None) and toCompute (list of
    (conf, nbSample, earlyExit) required to continue)"""
    cached={obs.conf:obs for obs in observations}
    nbR=residualFactor*nbSample
    def counters(conf):
        obs=cached.get(tuple(conf))
        return (0,0) if obs==None else (obs.nbFail(), obs.nbSample())
    def complete(conf, earlyExit):
        f,n=counters(conf)
        return n>=nbR or (earlyExit and f>0)

    union=set().union(*[set(c) for c in clauses])
    residual=tuple(i for i in range(nbDelta) if i not in union)
    res={"verdict":"incomplete", "complement":None, "levels":[], "clause":None, "toCompute":[]}
    if len(residual)==0:
        res["verdict"]="pass"
        return res
    f,n=counters(residual)
    res["complement"]={"size":len(residual), "nbFail":f, "nbSample":n}
    halves=splitHalves(residual)
    missing=[(c, nbR, False) for c in (residual,)+halves if not complete(c, False)]
    if len(missing)!=0:
        res["toCompute"]=missing
        return res
    if f==0:
        res["verdict"]="pass"
        return res

    node=residual
    while True:
        if len(node)==1:
            res["verdict"]="localized"
            res["clause"]=node
            return res
        earlyExit=node is not residual
        halves=splitHalves(node)
        missing=[(c, nbR, earlyExit) for c in halves if not complete(c, earlyExit)]
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
            res["clause"]=node
            return res
        level["decision"]="first half" if node==halves[0] else "second half"
