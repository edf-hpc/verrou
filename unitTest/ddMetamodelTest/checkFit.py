#!/usr/bin/env python3
"""Check the metamodel fitted by verrou_dd_metamodel against the metamodel used to simulate the code :
  checkFit.py truth.json fitted.json
  checkFit.py --probe truth.json fitted.json
Without --probe the parameters are compared : failure probability of each clause of the truth, interactions,
diffuse background, and no other clause with a significant failure probability.
With --probe the failure probabilities of probe configurations are compared instead of the parameters :
the clauses of the fitted model are not the ones of the truth with graded clauses (the delta-debug splits
them in several ddmin). The alternative clauses of the truth (clauses sharing deltas) have to be clauses
of the fitted model.
The tolerances are about twice the largest errors observed on the scenarios of the Makefile (the delta-debug
and the enrichment are not deterministic with several threads). A graded clause and its parts are estimated
with fewer samples : they have a larger tolerance."""
import sys
import math
import ddMetamodel

tolClause=0.15     #failure probability of a clause
tolMu=0.6          #interaction term mu (the sign has to be the same)
maxSpurious=0.05   #failure probability of a fitted clause which is not a clause of the truth
maxBgFactor=2.5    #failure probability of the full background : ratio fitted/truth in [1/maxBgFactor, maxBgFactor]
tolProbe=0.1       #failure probability of a probe
tolProbeGraded=0.2 #failure probability of a graded clause of the truth and of its parts

def pFail(lam):
    return -math.expm1(-lam)

def status(ok):
    return "OK" if ok else "KO"

def checkProbe(truth, fitted):
    ok=True
    probes={} #probe -> tolerance
    def add(probe, tol):
        probes[probe]=max(tol, probes.get(probe, tol))
    for k,clause in enumerate(truth.clauses):
        tol=tolProbe if truth.gammas[k]==None else tolProbeGraded
        add(tuple(sorted(clause)), tol)
        for half in ddMetamodel.splitHalves(clause):
            if len(half)!=0:
                add(half, tol)
    add(tuple(sorted(truth.union)), tolProbe)
    add(tuple(i for i in range(truth.nbDelta) if i not in truth.union), tolProbe)
    add(tuple(range(truth.nbDelta)), tolProbe)
    for probe in sorted(probes, key=lambda p: (len(p), p)):
        qTruth=truth.pFail(frozenset(probe))
        qFit=fitted.pFail(frozenset(probe))
        res=abs(qTruth-qFit)<probes[probe]
        ok=ok and res
        print("%s probe %s : pFail %.3f (expected %.3f, tolerance %.2f)"%(status(res), ddMetamodel.toRanges(probe), qFit, qTruth, probes[probe]))
    for k,clause in enumerate(truth.clauses):
        if any(len(clause&other)!=0 for l,other in enumerate(truth.clauses) if l!=k):
            res=clause in fitted.clauses
            ok=ok and res
            print("%s alternative clause %s"%(status(res), ddMetamodel.toRanges(clause)))
    return ok

def checkParameters(truth, fitted):
    ok=True
    fittedIndex={clause:k for k,clause in enumerate(fitted.clauses)}
    for k,clause in enumerate(truth.clauses):
        if clause not in fittedIndex:
            print("KO ddmin not found : %s"%(sorted(clause)))
            ok=False
            continue
        qTruth=pFail(truth.lambdas[k])
        qFit=pFail(fitted.lambdas[fittedIndex[clause]])
        res=abs(qTruth-qFit)<tolClause
        ok=ok and res
        print("%s clause %s : pFail %.3f (expected %.3f, tolerance %.2f)"%(status(res), sorted(clause), qFit, qTruth, tolClause))
    for k,clause in enumerate(fitted.clauses):
        if clause not in truth.clauses:
            qFit=pFail(fitted.lambdas[k])
            res=qFit<maxSpurious
            ok=ok and res
            print("%s clause %s not in the truth : pFail %.3f (maximum %.2f)"%(status(res), sorted(clause), qFit, maxSpurious))
    for (k,l),mu in truth.interactions.items():
        if truth.clauses[k] not in fittedIndex or truth.clauses[l] not in fittedIndex:
            print("KO interaction %s %s : ddmin not found"%(sorted(truth.clauses[k]), sorted(truth.clauses[l])))
            ok=False
            continue
        pair=tuple(sorted((fittedIndex[truth.clauses[k]], fittedIndex[truth.clauses[l]])))
        muFit=fitted.interactions.get(pair, 0.)
        res=muFit*mu>0 and abs(muFit-mu)<tolMu
        ok=ok and res
        print("%s interaction %s : mu %.3f (expected %.3f, tolerance %.2f)"%(status(res), pair, muFit, mu, tolMu))
    if truth.eps>0:
        qTruth=pFail(truth.eps)
        qFit=pFail(fitted.eps)
        res=qTruth/maxBgFactor<=qFit<=qTruth*maxBgFactor
        ok=ok and res
        print("%s background : pFail(all) %.3f (expected %.3f, factor %.1f)"%(status(res), qFit, qTruth, maxBgFactor))
    return ok

if __name__=="__main__":
    probe="--probe" in sys.argv
    if probe:
        sys.argv.remove("--probe")
    truth=ddMetamodel.loadMetamodel(sys.argv[1])[0]
    fitted=ddMetamodel.loadMetamodel(sys.argv[2])[0]
    if fitted.nbDelta!=truth.nbDelta:
        print("KO invalid nbDelta : %i (expected %i)"%(fitted.nbDelta, truth.nbDelta))
        sys.exit(1)
    ok=checkProbe(truth, fitted) if probe else checkParameters(truth, fitted)
    sys.exit(0 if ok else 1)
