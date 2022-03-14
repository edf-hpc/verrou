#!/usr/bin/python3
import sys
import os

def extract(rep):
    fileName=os.path.join(rep,"res.out")
    value=float(open(fileName).readline().split(":")[1])
    return value
    
def cmpRep(repRef,rep):
    valueRef=extract(repRef)
    value=extract(rep)
    print("valueRef: ",valueRef)
    print("value: ",value)
    
    return abs(value -valueRef) < 0.05*abs(valueRef)
    


if __name__=="__main__":

    if len(sys.argv)==3:
        ok=cmpRep(sys.argv[1],sys.argv[2])    
        if ok:
            sys.exit(0)
        else:
            sys.exit(1)
    
    if len(sys.argv)==2:
        print(extract(sys.argv[1]))
        sys.exit(0)
    print("Use one or two args")
    sys.exit(1)
