#!/usr/bin/env python3


def computeDot(v1,v2, blockSize):
    totalSize=len(v1)
    if totalSize!=len(v2):
        print("incoherent size")
        sys.exit(42)
    if totalSize % blockSize !=0:
        print("block size should divide total size")
        sys.exit(42)

        
    res=0.
    for iExtern in range(int(totalSize / blockSize)):
        resLocal=0.
        for iIntern in range(blockSize):
            i=iExtern *blockSize + iIntern
            resLocal+=v1[i]*v2[i]
        res+=resLocal
    return res

if __name__=="__main__":
    v1=[0.1 for i in range(128)]
    v2=[0.1 for i in range(128)]

    for b in [1,2,4,8,16,32]:
        dot1=computeDot(v1,v2, b)
        dot2=computeDot(v1,v2, b)
        print("1-dot("+str(b)+") %.17g %.17g"%(dot1, dot1-1.28))
        print("2-dot("+str(b)+") %.17g %.17g"%(dot2, dot2-1.28))
