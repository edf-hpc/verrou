#!/usr/bin/env python3

import sys
import ddRun
import os
import pickle
from pathlib import Path

def loadResult(rep):
    if "dd.line" in rep:
        pref="dd.line"
    if "dd.sym" in rep:
        pref="dd.sym"

    rddmincmp=None
    ddmaxcmp=None
    ddmax=None

    listOfrddim=[[line.strip() for line in open(x / (pref+".include")).readlines()]
                 for x in rep.glob("ddmin*")]
    full=[line.strip()
          for line in open(rep / "FullPerturbation" / (pref+".include")).readlines()]

    if (rep / "rddmin-cmp").is_dir():
        rddmincmp=[line.strip()
                   for line in open(rep / "rddmin-cmp" / (pref+".include")).readlines()]

    noPerturb=[line.strip()
               for line in open(rep / "NoPerturbation" / (pref+".include")).readlines()]

    if (rep / "ddmax-cmp").is_dir():
        ddmaxcmp=[line.strip()
                  for line in open(rep / "ddmax-cmp" / (pref+".include")).readlines()]
    if (rep / "ddmax").is_dir():
        ddmax=[line.strip()
              for line in open(rep / "ddmax" / (pref+".include")).readlines()]

    return {"ddmin": listOfrddim,
            "full":  full,
            "rddmincmp": rddmincmp,
            "noperturbation": noPerturb,
            "ddmax":ddmax,
            "ddmaxcmp":ddmaxcmp
            }


if __name__=="__main__":

    resRep=Path(sys.argv[1])
    resOut=Path(sys.argv[2])

    ddCase=ddRun.ddConfig()
    ref=resRep / "ref"
    ddCase.unpickle(ref / "dd.pickle")

    loadedRes=loadResult(resRep)
    ddmaxAlgo=False
    if loadedRes["ddmax"]!=None:
        ddmaxAlgo=True

    rddminAlgo=False
    if loadedRes["rddmincmp"]!=None:
        rddminAlgo=True

    if rddminAlgo==False and ddmaxAlgo==False:
        print("one algo need to be detected")
        sys.exit(42)

    res=False
    if "dd.sym" in resRep:
        if ddmaxAlgo:
            res=ddCase.checkDDmaxSymResult(loadedRes)
        else:
            res=ddCase.checkRddminSymResult(loadedRes)
    if "dd.line" in resRep:
        if rddminAlgo:
            res=ddCase.checkRddminLineResult(loadedRes)
        else:
            #not yet implemented
            res=ddCase.checkDDmaxLineResult(loadedRes)

    if res:
        print("valid rddmin//ddmax result")
    else:
        print("invalid rddmin//ddmax result")
        sys.exit(42)
