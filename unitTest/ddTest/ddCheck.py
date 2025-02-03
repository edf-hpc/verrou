#!/usr/bin/env python3

import sys
import ddRun
import os
import pickle

def loadResult(rep):
    if "dd.line" in rep:
        pref="dd.line"
    if "dd.sym" in rep:
        pref="dd.sym"

    rddmincmp=None
    ddmaxcmp=None
    ddmax=None

    listOfrddim=[[line.strip()
                  for line in open(os.path.join(rep,x,pref+".include")).readlines()]
                 for x in os.listdir(rep) if x.startswith("ddmin")]
    full=[line.strip()
          for line in open(os.path.join(rep,"FullPerturbation" ,pref+".include")).readlines()]

    if os.path.exists(os.path.join(rep,"rddmin-cmp")):
        rddmincmp=[line.strip()
                   for line in open(os.path.join(rep,"rddmin-cmp" ,pref+".include")).readlines()]

    noPerturb=[line.strip()
               for line in open(os.path.join(rep,"NoPerturbation" ,pref+".include")).readlines()]

    if os.path.exists(os.path.join(rep,"ddmax-cmp")):
        ddmaxcmp=[line.strip()
                  for line in open(os.path.join(rep,"ddmax-cmp" ,pref+".include")).readlines()]
    if os.path.exists(os.path.join(rep,"ddmax")):
        ddmax=[line.strip()
              for line in open(os.path.join(rep,"ddmax" ,pref+".include")).readlines()]

    return {"ddmin": listOfrddim,
            "full":  full,
            "rddmincmp": rddmincmp,
            "noperturbation": noPerturb,
            "ddmax":ddmax,
            "ddmaxcmp":ddmaxcmp
            }


if __name__=="__main__":

    resRep=sys.argv[1]
    resOut=sys.argv[2]

    ddCase=ddRun.ddConfig()
    ref=os.path.join(resRep,"ref")
    ddCase.unpickle(os.path.join(ref,"dd.pickle"))

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
