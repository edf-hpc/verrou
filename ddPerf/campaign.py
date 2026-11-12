#!/usr/bin/env python3
"""Campaign of verrou_dd_fake : each option set is simulated on each metamodel with several seeds.
  campaign.py [--seeds=N] [--out=DIR] METAMODEL.json... -- "OPTIONS1" "OPTIONS2" ...
Example :
  ./campaign.py --seeds=20 meta1.json meta2.json -- "--nruns=5 --num-threads=4" "--nruns=5 --num-threads=4 --rddmin=s"
Print for each (metamodel, options) the mean of the simulated wall time, cpu time, number of samples,
number of ddmin found, fraction of the clauses of the metamodel found and number of background ddmin."""

import sys
import os
import json
import shutil
import subprocess
import statistics
from pathlib import Path

fakeCmd=str(Path(__file__).resolve().parent / "verrou_dd_fake")

def parseArgs(argv):
    nbSeed=10
    outDir=Path("campaign")
    metamodels=[]
    optionsTab=[]
    args=argv[1:]
    if "--" not in args:
        print(__doc__)
        sys.exit(1)
    sep=args.index("--")
    for arg in args[:sep]:
        if arg.startswith("--seeds="):
            nbSeed=int(arg[len("--seeds="):])
        elif arg.startswith("--out="):
            outDir=Path(arg[len("--out="):])
        else:
            metamodels+=[Path(arg).absolute()]
    optionsTab=args[sep+1:]
    return nbSeed, outDir, metamodels, optionsTab

def runOne(metamodel, options, seed, workDir):
    workDir.mkdir(parents=True, exist_ok=True)
    summaryFile=(workDir / "summary.json").absolute()
    cmd=[fakeCmd, "--cache=clean", "--quiet", "--fake-seed=%i"%(seed), "--summary="+str(summaryFile)]+options.split()+[str(metamodel)]
    with open(str(workDir / "out"),"w") as out:
        status=subprocess.call(cmd, cwd=str(workDir), stdout=out, stderr=subprocess.STDOUT)
    if status!=0:
        print("Error : %s (see %s)"%(" ".join(cmd), workDir / "out"), file=sys.stderr)
        return None
    with open(str(summaryFile)) as f:
        res=json.load(f)
    shutil.rmtree(str(workDir / "dd.fake"), ignore_errors=True)
    return res

if __name__=="__main__":
    nbSeed, outDir, metamodels, optionsTab=parseArgs(sys.argv)
    print("%-24s %-44s %10s %10s %8s %6s %7s %6s %8s"%("metamodel", "options", "wall", "cpu", "samples", "ddmin", "recall", "noise", "cmpPFail"))
    for metamodel in metamodels:
        for iOpt,options in enumerate(optionsTab):
            resTab=[runOne(metamodel, options, seed, outDir / metamodel.stem / ("opt%i"%iOpt) / ("seed%i"%seed)) for seed in range(nbSeed)]
            resTab=[res for res in resTab if res!=None]
            if len(resTab)==0:
                continue
            mean=lambda f: statistics.mean(f(res) for res in resTab)
            print("%-24s %-44s %10.4g %10.4g %8.1f %6.2f %7.3f %6.2f %8.3f"%(
                metamodel.stem[:24], options[:44],
                mean(lambda r: r["wallTime"]), mean(lambda r: r["cpuTime"]), mean(lambda r: r["nbSample"]),
                mean(lambda r: len(r["ddmin"])),
                mean(lambda r: len(r["clausesFound"])/r["nbClause"] if r["nbClause"]!=0 else 1.),
                mean(lambda r: len([d for d in r["ddmin"] if d["status"]=="background"])),
                mean(lambda r: 0. if r.get("rddminCmp")==None else r["rddminCmp"]["pFail"])))
