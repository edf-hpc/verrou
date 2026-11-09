#!/usr/bin/env python3
"""Check the metamodel fitted by verrou_dd_metamodel against the metamodel used to simulate the code :
  checkFit.py truth.json fitted.json"""
import sys
import math
import ddMetamodel

def pFail(lam):
    return -math.expm1(-lam)

if __name__=="__main__":
    truth=ddMetamodel.loadMetamodel(sys.argv[1])[0]
    fitted=ddMetamodel.loadMetamodel(sys.argv[2])[0]
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
        pair=tuple(sorted((fittedIndex.get(truth.clauses[k]), fittedIndex.get(truth.clauses[l]))))
        muFit=fitted.interactions.get(pair, 0.)
        status="OK" if muFit*mu>0 and abs(muFit-mu)<1. else "KO"
        ok=ok and status=="OK"
        print("%s interaction %s : mu %.3f (expected %.3f)"%(status, pair, muFit, mu))
    if truth.eps>0:
        status="OK" if fitted.eps>0.25*truth.eps else "KO"
        ok=ok and status=="OK"
        print("%s background : pFail(all) %.3f (expected %.3f)"%(status, pFail(fitted.eps), pFail(truth.eps)))
    sys.exit(0 if ok else 1)
