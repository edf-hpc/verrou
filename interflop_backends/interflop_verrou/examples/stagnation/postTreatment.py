#!/usr/bin/python3
from fractions import *

def loadFile(fileNameTab, outPut, st=False):
    outputhdl=open(outPut,"w")
    hdlTab=[open(fileName) for fileName in fileNameTab]
    size=len(fileNameTab)
    res=[]
    if st:
        indexPrev=0
        outputRate=open(outPut+"stagnation.rate","w")
        prevValue=100000.
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
        ref=None
        if st:
            refTab=[Fraction(line[4]) for line in lineTab]
            ref=refTab[0]
            numStagnationTab=[Fraction(line[5]) for line in lineTab]
            numStagnationAverage=sum(numStagnationTab)/size
            ratioIndex= numStagnationAverage / (index -indexPrev)
        else:
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

        if st:
            outputRate.write("%.17f\t%.17f\n"%(index-indexPrev , ratioIndex) )

        prevValue=average
        indexPrev=index


if __name__=="__main__":
    for rnd in [i+j for i in ["AVERAGE", "RANDOM"] for j in ["","_DET"]  ]+ ["SR_MONOTONIC","SR_SMONOTONIC"]:
        loadFile([rnd+"."+str(i)+".out"  for i in range(20)],rnd+".post.out")
        loadFile([rnd+"_st."+str(i)+".out"  for i in range(20)],rnd+"_st.post.out",st=True)
