#!/usr/bin/env python3
"""Histograms of the cost and of the quality of a delta-debug algorithm simulated by verrou_dd_fake on several seeds :
  histograms.py [--seeds=N] [--first-seed=S] [--jobs=J] [--bins=B] [--max-samples=M] [--out=DIR] [--recompute] METAMODEL.json... -- "OPTIONS1" ["OPTIONS2" ...]
Example :
  ./histograms.py --seeds=100 meta.json -- "--nruns=5 --num-threads=4" "--nruns=5 --num-threads=4 --rddmin=s"
For each metamodel, the number of samples and the simulated wall time are measured at four steps :
  complete     : end of the delta-debug
  first ddmin  : the first ddmin found (whatever its status)
  half clauses : half of the clauses of the metamodel (rounded up) are found (the reference is the metamodel,
                 not the ddmin found by the simulated algorithm) : the seeds which do not reach it are counted
  all clauses  : all the clauses of the metamodel are found
The quality of the result is measured by :
  clauses found  : percentage of the clauses of the metamodel found
  weighted       : percentage of the clauses found weighted by their failure probability (missing a clause
                   which fails often is worse)
  residual pFail : failure probability of the complement of the ddmin found (rddmin-cmp for the rddmin
                   algorithms) : probability that the final check passes although the code still fails
  efficiency     : cpu time / (wall time * number of threads)
  deltas outside : number of deltas of the ddmin outside the clauses of the metamodel (they should be in the
                   complement rddmin-cmp) : with a metamodel without background (verrou_dd_fake --no-background
                   or verrou_dd_metamodel --no-background) these deltas are useless for the user
  non minimal    : number of ddmin which contain a clause and other deltas
--max-samples=M stops each simulation after M samples (verrou_dd_fake --max-samples=) : an algorithm out of
control does not block the histograms. A stopped simulation is not counted in the complete step, only
counted in the other steps if they are reached before the stop, and its quality is the one at the stop.
The option sets are superposed (at most 3 by figure, the next ones go to the next figures with the same bins).
Written in DIR (default histograms) : <metamodel>.png (or <metamodel>-<k>.png when there are more than 3 option
sets), <metamodel>.values.json (raw values) and work/ (output and summary of each simulation).
Cache : the summary of each simulation is kept in DIR/work/<metamodel name>-<md5 of the metamodel>/<md5 of the
options>/seed<S> (the options are written in options.txt) : only the missing simulations are computed when an
option set is added or when the number of seeds is increased. The cache does not know the version of
verrou_dd_fake and DD_stoch : --recompute computes again all the simulations (after a change of the algorithms).
A summary without a value used by the histograms (written by an older verrou_dd_fake) is computed again.
The quantiles and a compact histogram are printed for each step."""

import sys
import os
import json
import math
import hashlib
import time
import statistics
import concurrent.futures
from pathlib import Path

import campaign

colors=["#2a78d6", "#eb6834", "#1baf7a"] #categorical slots 1-3 (validated for overlapping marks)
maxOptionsByFigure=len(colors)
textPrimary="#0b0b0b"
textSecondary="#52514e"
gridColor="#e4e3df"
surface="#fcfcfb"

steps=[("complete", "complete"), ("firstDdmin", "first ddmin"), ("halfClauses", "half clauses"), ("allClauses", "all clauses")]
metrics=[("nbSample", "samples"), ("wallTime", "wall time")]
#rows of quality panels : key, title, x label, step of the discrete values (None : continuous), fixed range or None
qualities=[[("clausesFound", "clauses found", "clauses of the metamodel found (%)", "clause", (0., 100.)),
            ("weightedRecall", "clauses found, weighted", "clauses found weighted by their failure probability (%)", None, (0., 100.)),
            ("residualPFail", "residual failure probability", "failure probability of the complement of the ddmin", None, None),
            ("efficiency", "parallel efficiency", "cpu time / (wall time * threads)", None, None)],
           [("deltasOutside", "deltas outside the clauses", "deltas of the ddmin outside the clauses", 1, None),
            ("nbNonMinimal", "non minimal ddmin", "ddmin containing a clause and other deltas", 1, None)]]

def panels(timeUnit, nbClause):
    """Rows of the figure (list of panels : key, title, x label, label of the missing values, step of the
    discrete values or None)"""
    rows=[[((step,metric), stepName, metricName if metric!="wallTime" else "%s (%s)"%(metricName, timeUnit),
            "stopped" if step=="complete" else "not reached", 1 if metric=="nbSample" else None)
           for step,stepName in steps] for metric,metricName in metrics]
    for row in qualities:
        rows+=[[(("quality",quality), title, xlabel, None,
                 ((100./nbClause if nbClause!=0 else None) if step=="clause" else step, fixedRange))
                for quality,title,xlabel,step,fixedRange in row]]
    return rows

def getValue(v, key):
    return v[key[0]][key[1]]


def parseArgs(argv):
    res={"nbSeed":50, "firstSeed":0, "maxSamples":None, "recompute":False, "jobs":os.cpu_count() or 1, "bins":20, "outDir":Path("histograms"),
         "metamodels":[], "optionsTab":[]}
    args=argv[1:]
    if "--" not in args:
        print(__doc__)
        sys.exit(1)
    sep=args.index("--")
    for arg in args[:sep]:
        for key,option,conv in [("nbSeed","--seeds=",int), ("firstSeed","--first-seed=",int), ("jobs","--jobs=",int),
                                ("bins","--bins=",int), ("outDir","--out=",Path),
                                ("maxSamples","--max-samples=",int)]:
            if arg.startswith(option):
                res[key]=conv(arg[len(option):])
                break
        else:
            if arg=="--recompute":
                res["recompute"]=True
                continue
            if arg.startswith("--"):
                print("Error : unknown option %s"%(arg), file=sys.stderr)
                sys.exit(1)
            res["metamodels"]+=[Path(arg).absolute()]
    res["optionsTab"]=args[sep+1:]
    if len(res["metamodels"])==0 or len(res["optionsTab"])==0:
        print(__doc__)
        sys.exit(1)
    return res

def values(res):
    """Values of one simulation : dict step -> dict metric -> value (None : step not reached)"""
    stopped=res.get("stopped", False)
    out={"complete":{metric:(None if stopped else res[metric]) for metric,label in metrics}, "stopped":stopped}
    for step,name in steps[1:]:
        reached=res["progress"][step]
        out[step]={metric:(None if reached==None else reached[metric]) for metric,label in metrics}
    out["quality"]={"clausesFound":100.*len(res["clausesFound"])/res["nbClause"] if res["nbClause"]!=0 else None,
                    "weightedRecall":None if res["weightedRecall"]==None else 100.*res["weightedRecall"],
                    "residualPFail":res["residualPFail"], "efficiency":res["efficiency"],
                    "deltasOutside":res["nbDeltaOutsideClauses"], "nbNonMinimal":res["nbNonMinimal"]}
    out["nbClause"]=res["nbClause"]
    return out

def md5Short(data):
    return hashlib.md5(data).hexdigest()[:12]

def cachedSummary(workDir):
    """Summary of a simulation in cache, or None if it is missing or written by an older verrou_dd_fake"""
    try:
        with open(str(workDir / "summary.json")) as f:
            res=json.load(f)
        values(res)
        return res
    except (OSError, ValueError, KeyError, TypeError):
        return None

def simulate(metamodel, optionsTab, conf):
    """Raw values : list (by option set) of lists (by seed) of values(). The simulations in cache are reused"""
    seeds=range(conf["firstSeed"], conf["firstSeed"]+conf["nbSeed"])
    with open(str(metamodel), "rb") as f:
        workRoot=conf["outDir"] / "work" / ("%s-%s"%(metamodel.stem, md5Short(f.read())))
    if conf["maxSamples"]!=None:
        optionsTab=[options+" --max-samples=%i"%(conf["maxSamples"]) for options in optionsTab]
    optionsTab=[" ".join(options.split()) for options in optionsTab]
    tasks={} #workDir -> future : the same option set given twice is computed once
    futures=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=conf["jobs"]) as executor:
        for options in optionsTab:
            optionsDir=workRoot / md5Short(options.encode("utf-8"))
            optionsDir.mkdir(parents=True, exist_ok=True)
            with open(str(optionsDir / "options.txt"), "w") as f:
                f.write(options+"\n")
            futureTab=[]
            for seed in seeds:
                workDir=optionsDir / ("seed%i"%seed)
                if workDir not in tasks:
                    res=None if conf["recompute"] else cachedSummary(workDir)
                    if res!=None:
                        future=concurrent.futures.Future()
                        future.set_result(res)
                        future.cached=True
                    else:
                        future=executor.submit(campaign.runOne, metamodel, options, seed, workDir)
                        future.cached=False
                    tasks[workDir]=future
                futureTab+=[tasks[workDir]]
            futures+=[futureTab]
        progress(metamodel, futures)
        return [[values(f.result()) for f in futureTab if f.result()!=None] for futureTab in futures]

def formatDuration(seconds):
    seconds=int(round(seconds))
    return "%dh%02dm%02ds"%(seconds//3600, (seconds%3600)//60, seconds%60) if seconds>=3600 else "%dm%02ds"%(seconds//60, seconds%60)

def progress(metamodel, futures):
    """Print the progress of the simulations on stderr until they are all done : on one line updated in
    place on a terminal, otherwise one line every 10% of the simulations. The simulations in cache are not
    counted in the percentage and in the remaining time"""
    allFutures=list(set(f for futureTab in futures for f in futureTab if not f.cached))
    nbCached=len(set(f for futureTab in futures for f in futureTab if f.cached))
    total=len(allFutures)
    if total==0:
        print("%s : %i simulations in cache, nothing to compute"%(metamodel.name, nbCached), file=sys.stderr)
        return
    start=time.time()
    tty=sys.stderr.isatty()
    lastDecile=-1
    nbDone=0
    for future in concurrent.futures.as_completed(allFutures):
        nbDone+=1
        nbFailed=len([f for f in allFutures if f.done() and f.result()==None])
        nbStopped=len([f for f in allFutures if f.done() and f.result()!=None and f.result().get("stopped")])
        done=["%i/%i"%(len([f for f in futureTab if f.done()]), len(futureTab)) for futureTab in futures]
        elapsed=time.time()-start
        line="%s : %i/%i simulations (%3i%%) by option set %s  elapsed %s  remaining %s%s"%(
            metamodel.name, nbDone, total, 100*nbDone//total, " ".join(done), formatDuration(elapsed),
            formatDuration(elapsed*(total-nbDone)/nbDone), "" if nbFailed==0 else "  %i failed"%(nbFailed))
        if nbStopped!=0:
            line+="  %i stopped (--max-samples=)"%(nbStopped)
        if nbCached!=0:
            line+="  (%i in cache)"%(nbCached)
        if tty:
            print("\r\033[K"+line, end="", file=sys.stderr, flush=True)
        elif 10*nbDone//total>lastDecile:
            lastDecile=10*nbDone//total
            print(line, file=sys.stderr, flush=True)
    if tty:
        print("", file=sys.stderr)


def quantile(sortedTab, q):
    pos=q*(len(sortedTab)-1)
    low=int(math.floor(pos))
    high=min(low+1, len(sortedTab)-1)
    return sortedTab[low]+(sortedTab[high]-sortedTab[low])*(pos-low)

def binEdges(tabs, nbBin, step=None):
    """Edges of nbBin bins, or of one bin centered on each value for discrete values (multiples of step) if
    they are not too many. step=(step, (low, high)) : the range is fixed (if (low, high) is not None) and step
    is None for continuous values"""
    allValues=[v for tab in tabs for v in tab]
    if len(allValues)==0:
        return None
    low,high=min(allValues), max(allValues)
    if isinstance(step, tuple):
        step,fixedRange=step
        if fixedRange!=None:
            low,high=fixedRange
    if step!=None and (high-low)/step<=2*nbBin:
        nb=int(round((high-low)/step))+1
        return [low-step/2+step*i for i in range(nb+1)]
    if high==low:
        low,high=low-0.5, high+0.5
    return [low+(high-low)*i/nbBin for i in range(nbBin+1)]

def counts(tab, edges):
    res=[0]*(len(edges)-1)
    for v in tab:
        res[min(int((v-edges[0])/(edges[-1]-edges[0])*(len(edges)-1)), len(edges)-2)]+=1
    return res

def sparkline(tab, edges):
    blocks=" ▁▂▃▄▅▆▇█"
    c=counts(tab, edges)
    top=max(c) if max(c)>0 else 1
    return "".join(blocks[int(math.ceil(x/top*(len(blocks)-1)))] for x in c)

def printText(metamodel, optionsTab, raw, nbBin, rows):
    print("\n%s : %s"%(metamodel.name, ", ".join("%i seeds"%(len(r)) for r in raw)))
    for key,title,xlabel,missingLabel,step in [panel for row in rows for panel in row]:
        tabs=[sorted(getValue(v, key) for v in r if getValue(v, key)!=None) for r in raw]
        edges=binEdges(tabs, nbBin, step)
        print("  %s%s"%(title+", "+xlabel if key[0]!="quality" else xlabel, "" if edges==None else "  (histogram from %.4g to %.4g)"%(edges[0], edges[-1])))
        print("    %-3s %8s %10s %10s %10s %10s %10s %10s  %s"%("opt", "reached", "min", "q10", "median", "mean", "q90", "max", "histogram"))
        for iOpt,tab in enumerate(tabs):
            if len(tab)==0:
                print("    %-3i %4i/%-3i"%(iOpt, 0, len(raw[iOpt])))
                continue
            print("    %-3i %4i/%-3i %10.4g %10.4g %10.4g %10.4g %10.4g %10.4g  |%s|"%(
                iOpt, len(tab), len(raw[iOpt]), tab[0], quantile(tab, 0.1), quantile(tab, 0.5),
                statistics.mean(tab), quantile(tab, 0.9), tab[-1], sparkline(tab, edges)))
    for iOpt,options in enumerate(optionsTab):
        print("  opt %i : %s"%(iOpt, options))


def plot(metamodel, optionsTab, raw, nbBin, outDir, rows):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.ticker

    groups=[list(range(i, min(i+maxOptionsByFigure, len(optionsTab)))) for i in range(0, len(optionsTab), maxOptionsByFigure)]
    edgesTab={key:binEdges([[getValue(v, key) for v in r if getValue(v, key)!=None] for r in raw], nbBin, step)
              for row in rows for key,title,xlabel,missingLabel,step in row}
    nbCol=max(len(row) for row in rows)
    files=[]
    for iGroup,group in enumerate(groups):
        fig,axes=plt.subplots(len(rows), nbCol, figsize=(5.2*nbCol, 3.6*len(rows)+0.3*len(group)+0.6),
                              facecolor=surface, squeeze=False, layout="constrained")
        for iRow,row in enumerate(rows):
            for col in range(len(row), nbCol):
                axes[iRow][col].set_visible(False)
            for col,(key,title,xlabel,missingLabel,step) in enumerate(row):
                ax=axes[iRow][col]
                ax.set_facecolor(surface)
                edges=edgesTab[key]
                for slot,iOpt in enumerate(group):
                    tab=[getValue(v, key) for v in raw[iOpt] if getValue(v, key)!=None]
                    if edges==None or len(tab)==0:
                        continue
                    ax.hist(tab, bins=edges, histtype="stepfilled", color=colors[slot], alpha=0.10, linewidth=0)
                    ax.hist(tab, bins=edges, histtype="step", color=colors[slot], linewidth=2)
                    ax.axvline(statistics.median(tab), color=colors[slot], linewidth=1.5, linestyle=(0,(4,3)))
                missing=["opt %i: %i/%i %s"%(iOpt, len([v for v in raw[iOpt] if getValue(v, key)==None]), len(raw[iOpt]), missingLabel)
                         for iOpt in group if any(getValue(v, key)==None for v in raw[iOpt])]
                if missingLabel==None:
                    #the quality of the stopped simulations is the one at the stop
                    missing=["opt %i: %i/%i stopped"%(iOpt, len([v for v in raw[iOpt] if v["stopped"]]), len(raw[iOpt]))
                             for iOpt in group if any(v["stopped"] for v in raw[iOpt])]
                ax.set_title(title+"".join("\n"+", ".join(missing) for i in [0] if len(missing)!=0), color=textPrimary, fontsize=11, loc="left")
                ax.set_xlabel(xlabel, color=textSecondary, fontsize=9)
                ax.set_ylabel("seeds", color=textSecondary, fontsize=9)
                ax.tick_params(colors=textSecondary, labelsize=8, length=0)
                ax.yaxis.set_major_locator(matplotlib.ticker.MaxNLocator(integer=True))
                if step==1: #counts
                    ax.xaxis.set_major_locator(matplotlib.ticker.MaxNLocator(integer=True))
                ax.grid(axis="y", color=gridColor, linewidth=0.8)
                ax.set_axisbelow(True)
                for side in ["top", "right", "left"]:
                    ax.spines[side].set_visible(False)
                ax.spines["bottom"].set_color(textSecondary)
        handles=[matplotlib.lines.Line2D([], [], color=colors[slot], linewidth=2, label="opt %i : %s"%(iOpt, optionsTab[iOpt]))
                 for slot,iOpt in enumerate(group)]
        handles+=[matplotlib.lines.Line2D([], [], color=textSecondary, linewidth=1.5, linestyle=(0,(4,3)), label="median")]
        legend=fig.legend(handles=handles, loc="outside upper left", ncol=1, frameon=False, fontsize=9, labelcolor=textPrimary,
                          title="%s : %s seeds"%(metamodel.name, "/".join(str(len(raw[iOpt])) for iOpt in group)),
                          title_fontsize=12, alignment="left")
        legend.get_title().set_color(textPrimary)
        fileName=outDir / ("%s.png"%(metamodel.stem) if len(groups)==1 else "%s-%i.png"%(metamodel.stem, iGroup))
        fig.savefig(str(fileName), dpi=110, facecolor=surface)
        plt.close(fig)
        files+=[fileName]
    return files


if __name__=="__main__":
    conf=parseArgs(sys.argv)
    conf["outDir"].mkdir(parents=True, exist_ok=True)
    for metamodel in conf["metamodels"]:
        with open(str(metamodel)) as f:
            timeUnit="simulated seconds" if json.load(f).get("time")!=None else "simulated, 1 per sample"
        raw=simulate(metamodel, conf["optionsTab"], conf)
        nbClause=max([v["nbClause"] for r in raw for v in r]+[0])
        rows=panels(timeUnit, nbClause)
        with open(str(conf["outDir"] / ("%s.values.json"%(metamodel.stem))),"w") as f:
            json.dump({"metamodel":str(metamodel), "options":conf["optionsTab"], "firstSeed":conf["firstSeed"],
                       "values":raw}, f, indent=1)
        printText(metamodel, conf["optionsTab"], raw, conf["bins"], rows)
        for fileName in plot(metamodel, conf["optionsTab"], raw, conf["bins"], conf["outDir"], rows):
            print("  figure written in %s"%(fileName))
