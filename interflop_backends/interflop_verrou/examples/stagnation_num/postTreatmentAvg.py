#!/usr/bin/python3
from fractions import *

def loadFile(fileNameTab, outPut):
    outputhdl=open(outPut,"w")
    hdlTab=[open(fileName) for fileName in fileNameTab]
    size=len(fileNameTab)
    res=[]

    while True:
        lineTab=[((hdl.readline()).strip()).split("\t") for hdl in hdlTab]
        if [""] in lineTab:
            return res
#        print(lineTab)

        index=int(lineTab[0][0])
        for i in range(size):
            if index!= int(lineTab[i][0]):
                print("incoherentFile")
                sys.exit()
        valueTab=[Fraction(line[1]) for line in lineTab]

        average=sum(valueTab)/Fraction(size)
        ref=Fraction(index) * Fraction(0.1) + Fraction(100000)

        averageError=abs(average-ref)
        averageErrorRelative=averageError/ ref

        maxValue=max([value for value in valueTab])
        minValue=min([value for value in valueTab])
        errorMax=max([abs(value - ref) for value in valueTab])
        errorMaxRelative=errorMax /ref
        errorMin=min([abs(value - ref) for value in valueTab])
        errorMinRelative=errorMin /ref


        data=[average, averageErrorRelative,
              minValue, errorMinRelative,
              maxValue, errorMaxRelative]
        outputhdl.write(("%d"+"\t%.17f"*6+"\n")%(index, *[float(v) for v in data]))

        prevValue=average
        indexPrev=index


if __name__=="__main__":
    for rnd in ["RANDOM"]+["AVERAGE_"+str(i) for i in [1,2,3,4,8,16,32,64]]:
        loadFile(["avg_study_"+rnd+"."+str(i)+".out"  for i in range(20)],rnd+".post.out")

