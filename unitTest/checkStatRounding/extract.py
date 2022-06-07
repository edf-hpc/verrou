#!/usr/bin/python3

import os
import sys

defaultType="double"
defaultAlgo="Seq"

def extract(fileName,typeFloat, algo):
    if not typeFloat in ["double","float"]:
        print("invalid ALGO_TYPE")
        sys.exit(42)
    if not algo in ["Seq","Rec"]:
        print("invalid ALGO")
        sys.exit(42)
        
    for line in open(fileName):
        if line.startswith("<%s>"%(typeFloat)):
            resLine=line.split("\t")[1:]
            for algoRes in resLine:
                algoStr, res= algoRes.split(":")
                if algoStr=="res"+algo:
                    return res.strip()
    print("invalid file :", fileName) 
    return None

if __name__=="__main__":
    if "ALGO_TYPE" in os.environ:
        algo_type=os.environ["ALGO_TYPE"]
    else:
        algo_type=defaultType
    if "ALGO" in os.environ:
        algo=os.environ["ALGO"]
    else:
        algo=defaultAlgo

    print(extract(sys.argv[1]+"/res.out", algo_type, algo))
