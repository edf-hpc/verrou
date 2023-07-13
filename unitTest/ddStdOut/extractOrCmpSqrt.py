#!/usr/bin/python3
import sys
import os


def extractValue(rep):
    lines=(open(os.path.join(rep,"res.dat")).readlines())
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
