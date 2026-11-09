#!/usr/bin/env python3
"""Run script for verrou_dd_sym and verrou_dd_line simulating a code with a metamodel (json file written by
verrou_dd_metamodel, DD_METAMODEL=file, truth.json by default) : the deltas are fake symbols or lines
numbered with their index in the metamodel. The failure of the sample is drawn with the failure
probability of the metamodel (reproducible with VERROU_SEED) and written for ddMetamodelCmp.py."""
import sys
import os
import random
from pathlib import Path
import ddMetamodel

def loadModel():
    fileName=os.environ.get("DD_METAMODEL", str(Path(__file__).absolute().parent / "truth.json"))
    return ddMetamodel.loadMetamodel(fileName)[0]

def symName(i):
    return "delta-%i\tmetamodel.so"%(i)

def lineName(i):
    return "metamodel.c\t%i\tdelta"%(i)

def runRef(model):
    if "VERROU_GEN_EXCLUDE" in os.environ: #verrou_dd_sym
        with open(os.environ["VERROU_GEN_EXCLUDE"].replace("%p", "4242"),"w") as f:
            f.write("".join(symName(i)+"\n" for i in range(model.nbDelta)))
    if "VERROU_GEN_SOURCE" in os.environ: #verrou_dd_line
        with open(os.environ["VERROU_GEN_SOURCE"].replace("%p", "4242"),"w") as f:
            f.write("".join(lineName(i)+"\n" for i in range(model.nbDelta)))

def readLines(fileName):
    with open(fileName) as f:
        return [line.rstrip("\n") for line in f]

def configuration(model):
    if "VERROU_SOURCE" in os.environ:
        included=set(readLines(os.environ["VERROU_SOURCE"]))
        return frozenset(i for i in range(model.nbDelta) if lineName(i) in included)
    excluded=set(readLines(os.environ["VERROU_EXCLUDE"]))
    return frozenset(i for i in range(model.nbDelta) if symName(i) not in excluded)

if __name__=="__main__":
    model=loadModel()
    rundir=Path(sys.argv[1])
    if rundir.name=="ref":
        runRef(model)
        sys.exit(0)
    S=configuration(model)
    rng=random.Random(os.environ["VERROU_SEED"]+"/"+",".join(map(str,sorted(S)))) if "VERROU_SEED" in os.environ else random.Random()
    fail,timeValue=model.sample(S, rng)
    with open(str(rundir / "dd.metamodel.fail"),"w") as f:
        f.write(str(int(fail))+"\n")
