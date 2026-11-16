#!/usr/bin/env python3
"""Check the metamodel fitted by verrou_dd_metamodel against the metamodel used to simulate the code :
  checkFit.py truth.json fitted.json
  checkFit.py --probe truth.json fitted.json
With --probe the failure probabilities of probe configurations are compared instead of the parameters :
the clauses of the fitted model are not the ones of the truth with graded clauses (the delta-debug splits
them in several ddmin). The alternative clauses of the truth (clauses sharing deltas) have to be clauses
of the fitted model."""
import sys
import math
import ddMetamodel

def pFail(lam):
    return -math.expm1(-lam)

def checkProbe(truth, fitted, tol=0.2):
    ok=True
    probes=[]
    for clause in truth.clauses:
        probes+=[tuple(sorted(clause))]+[half for half in ddMetamodel.splitHalves(clause) if len(half)!=0]
    probes+=[tuple(sorted(truth.union)), tuple(i for i in range(truth.nbDelta) if i not in truth.union), tuple(range(truth.nbDelta))]
    for probe in sorted(set(probes), key=lambda p: (len(p), p)):
        qTruth=truth.pFail(frozenset(probe))
        qFit=fitted.pFail(frozenset(probe))
        status="OK" if abs(qTruth-qFit)<tol else "KO"
        ok=ok and status=="OK"
        print("%s probe %s : pFail %.3f (expected %.3f)"%(status, ddMetamodel.toRanges(probe), qFit, qTruth))
    for k,clause in enumerate(truth.clauses):
        if any(len(clause&other)!=0 for l,other in enumerate(truth.clauses) if l!=k):
            status="OK" if clause in fitted.clauses else "KO"
            ok=ok and status=="OK"
            print("%s alternative clause %s"%(status, ddMetamodel.toRanges(clause)))
    return ok

if __name__=="__main__":
    probe="--probe" in sys.argv
    if probe:
        sys.argv.remove("--probe")
    truth=ddMetamodel.loadMetamodel(sys.argv[1])[0]
    fitted=ddMetamodel.loadMetamodel(sys.argv[2])[0]
    if probe:
        sys.exit(0 if checkProbe(truth, fitted) else 1)
    ok=True
    if fitted.nbDelta!=truth.nbDelta:
        print("invalid nbDelta : %i (expected %i)"%(fitted.nbDelta, truth.nbDelta))
        sys.exit(1)
    fittedIndex={clause:k for k,clause in enumerate(fitted.clauses)}
    for k,clause in enumerate(truth.clauses):
        if clause not in fittedIndex:
            print("ddmin not found : %s"%(sorted(clause)))
            ok=False
            continue
        qTruth=pFail(truth.lambdas[k])
        qFit=pFail(fitted.lambdas[fittedIndex[clause]])
        status="OK" if abs(qTruth-qFit)<0.15 else "KO"
        ok=ok and status=="OK"
        print("%s clause %s : pFail %.3f (expected %.3f)"%(status, sorted(clause), qFit, qTruth))
    for (k,l),mu in truth.interactions.items():
        if truth.clauses[k] not in fittedIndex or truth.clauses[l] not in fittedIndex:
            print("KO interaction %s %s : ddmin not found"%(sorted(truth.clauses[k]), sorted(truth.clauses[l])))
            ok=False
            continue
        pair=tuple(sorted((fittedIndex[truth.clauses[k]], fittedIndex[truth.clauses[l]])))
        muFit=fitted.interactions.get(pair, 0.)
        status="OK" if muFit*mu>0 and abs(muFit-mu)<1. else "KO"
        ok=ok and status=="OK"
        print("%s interaction %s : mu %.3f (expected %.3f)"%(status, pair, muFit, mu))
    if truth.eps>0:
        status="OK" if fitted.eps>0.25*truth.eps else "KO"
        ok=ok and status=="OK"
        print("%s background : pFail(all) %.3f (expected %.3f)"%(status, pFail(fitted.eps), pFail(truth.eps)))
    sys.exit(0 if ok else 1)
