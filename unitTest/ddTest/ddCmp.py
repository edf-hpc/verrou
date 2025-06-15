#!/usr/bin/env python3
import sys
import ddRun
import pickle
from pathlib import Path

def cmpNorm(ref, toCmp, ddCase):
    print("norm")
    if "dd.sym" in str(ref) and "dd.line" not in str(ref):
        return ddCase.statusOfSymConfig(open(toCmp / "path_exclude").readline())
    if "dd.line" in str(ref):
        return ddCase.statusOfSourceConfig(open(toCmp / "path_source").readline())
if __name__=="__main__":
    if sys.argv[1]== sys.argv[2]:
        sys.exit(0)
    else:
        ddCase=ddRun.ddConfig()
        ref=Path(sys.argv[1])
        ddCase.unpickle(ref / "dd.pickle")
        toCmp=Path(sys.argv[2])
        sys.exit(cmpNorm(ref, toCmp, ddCase))
    




