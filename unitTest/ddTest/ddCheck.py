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

    listOfrddim=[[line.strip()
                  for line in open(os.path.join(rep,x,pref+".include")).readlines()]
                 for x in os.listdir(rep) if x.startswith("ddmin")]
    full=[line.strip()
          for line in open(os.path.join(rep,"FullPerturbation" ,pref+".include")).readlines()]
    rddmincmp=[line.strip()
               for line in open(os.path.join(rep,"rddmin-cmp" ,pref+".include")).readlines()]
    noPerturb=[line.strip()
               for line in open(os.path.join(rep,"NoPerturbation" ,pref+".include")).readlines()]

    return {"ddmin": listOfrddim,
            "full":  full,
            "rddmincmp": rddmincmp,
            "noperturbation": noPerturb}


if __name__=="__main__":

    resRep=sys.argv[1]
    resOut=sys.argv[2]

    ddCase=ddRun.ddConfig()
    ref=os.path.join(resRep,"ref")
    ddCase.unpickle(os.path.join(ref,"dd.pickle"))

    loadedRes=loadResult(resRep)

    res=False
    if "dd.sym" in resRep:
        res=ddCase.checkRddminSymResult(loadedRes)
    if "dd.line" in resRep:
        res=ddCase.checkRddminLineResult(loadedRes)

    if res:
        print("valid rddmin result")
    else:
        print("invalid rddmin result")
        sys.exit(42)
