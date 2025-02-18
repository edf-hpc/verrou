#!/usr/bin/python3
import sys
import os.path
import re

def extract(rep):
    lines=(open(os.path.join(rep, "res.dat"))).readlines()
    return re.split(" ",lines[-1].strip())[1]
    
if __name__=="__main__":
    if len(sys.argv)==2: #extract for verrou_plot_stat
        print(extract(sys.argv[1]))
    if len(sys.argv)==3: #cmp for verrou_dd_*
        refValue=float(extract(sys.argv[1]))
        value=float(extract(sys.argv[2]))
        relDist= abs((value -refValue)/refValue)
        if relDist < 1e-6: sys.exit(0)
        else:sys.exit(1)
