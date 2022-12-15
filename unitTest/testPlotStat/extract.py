#!/usr/bin/env python3
import sys
blockSize=32

for line in open(sys.argv[1]+"/out"):
    if line.startswith("1-dot("+str(blockSize)+")"):
        print(line.split()[2])
        sys.exit(0)
sys.exit(42)
    
