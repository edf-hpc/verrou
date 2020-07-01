#!/usr/bin/python3
import sys
import os

def cmpRep(repRef,rep):
    fileNameRef=os.path.join(repRef,"res.out")
    fileName=os.path.join(rep,"res.out")
    
    valueRef=float(open(fileNameRef).readline().split(":")[1])
    value=float(open(fileName).readline().split(":")[1])

    print("valueRef: ",valueRef)
    print("value: ",value)
    
    return abs(value -valueRef) < 0.05*abs(valueRef)
    


if __name__=="__main__":
    ok=cmpRep(sys.argv[1],sys.argv[2])    
    if ok:
        sys.exit(0)
    else:
        sys.exit(1)
    

