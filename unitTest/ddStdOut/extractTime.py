#!/usr/bin/python3
import sys
import os
import re

pattern=re.compile("it:\s+(\d+)\s+x2:\s*([\w.]*)[\w\s]*")

def extractTime(rep):
    res=""
    lines=(open(os.path.join(rep,"res.dat")).readlines())
    for line in lines:
        regexp=pattern.match(line)
        if regexp!=None:
            res+=(" ".join(regexp.groups()))+"\n"
    return res[0:-1]

if __name__=="__main__":
    if len(sys.argv)==2:
        print(extractTime(sys.argv[1]))
