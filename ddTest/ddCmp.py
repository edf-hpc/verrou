#!/usr/bin/python


import sys
import ddRun
#DDConfig
import os
import pickle


def cmpNorm(ref, toCmp, ddCase):
    print "norm"
    if "dd.sym" in ref:
        return ddCase.statusOfSymConfig(open(toCmp+"/path_exclude").readline())
    if "dd.line" in ref:
        return ddCase.statusOfSourceConfig(ref+"/dd.exclude",open(toCmp+"/path_source").readline())
if __name__=="__main__":
    if sys.argv[1]== sys.argv[2]:
        sys.exit(0)
    else:
        ddCase=ddRun.ddConfig()
        ref=sys.argv[1]
        ddCase.unpickle(ref+"/dd.pickle")
        toCmp=sys.argv[2]
        sys.exit(cmpNorm(ref, toCmp,ddCase))
    




