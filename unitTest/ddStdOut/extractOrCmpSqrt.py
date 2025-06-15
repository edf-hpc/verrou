#!/usr/bin/python3
import sys
import os
from pathlib import Path


def extractValue(rep):
    fileName=Path(rep) / "res.dat"
    if "PARAM" in os.environ:
        fileName=Path(rep) / os.environ["PARAM"]
    lines=(open(fileName).readlines())
    for line in lines:
        if line.startswith("res="):
            return float(line.partition("=")[2])
    return None
               

if __name__=="__main__":
    if len(sys.argv)==2:
        print(extractValue(sys.argv[1]))
    if len(sys.argv)==3:
        valueRef=extractValue(sys.argv[1])
        value=extractValue(sys.argv[2])

        relDiff=abs((value-valueRef)/valueRef)
        if relDiff ==0:
            sys.exit(0)
        else:
            sys.exit(1)
