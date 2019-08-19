#!/usr/bin/env python3
import sys
import math

def readFile(fileName):
    data=(open(fileName).readlines()) 
    keyData=data[0].split()
    brutData=[line.split() for line in data[1:]]

    res={}
    for index in range(len(keyData)):
        fileNameKey=fileName.replace("res","")
        dataIndex=[line[index] for line in brutData]
        res[keyData[index]]=(min(dataIndex),max(dataIndex))

    return res

def computeEvalError(dataNative, data):
    res={}
    for key in dataNative.keys():
        resIEEE=float(dataNative[key][0])        
        evalError=  - math.log2(max(abs(float(data[key][1]) - resIEEE),
                                    abs(float(data[key][0]) - resIEEE)) / resIEEE)
        res[key]=evalError
    return res

def loadRef(fileName, num=2):
    res={}
    for line in open(fileName):
        spline=line.split(":")
        typeRealtype=spline[0].split()[0]
        correction=spline[0].split()[1]
        nbBitStr=spline[1].strip()
        if nbBitStr in ["24","53"]:
            res[(typeRealtype, correction)]=float(nbBitStr)
            continue
        [valueLow,valueUp]=nbBitStr[1:-1].split(",")
        if(float(valueUp)!=float(valueLow)):
            print("Please Increase the mpfi precision")
            sys.exit()
        value=float(valueUp)
        res[(typeRealtype, correction)]=value
    return res


def main(reference=None):
    

    output=open("tabAster.tex","w")
    outputReg=open("testReg","w")
    
    keys=["Native", "Randominterlibm", "Randomverrou", "Randomverrou+interlibm"]

    data={}
    strLatex=""
    for i in range(len(keys)):
        key=keys[i]
        data[key]=readFile("res"+key+".dat")

#        for key in sorted(keys[1:]):
    for i in range(1,len(keys)):
        key=keys[i]
        outputReg.write(key+"\n")
        evalError=computeEvalError(data["Native"], data[key])
        for keyCase in sorted(evalError.keys()):
            outputReg.write(keyCase +" "+str(evalError[keyCase])+"\n")
        
    output.write(r"\begin{table}" +" \n")
    output.write(r"\begin{center}" +" \n")
    output.write(r"\begin{tabular}{l@{~}lccccc}\toprule" +" \n")
    output.write(r"&  & \multicolumn{2}{c}{single precision}& \multicolumn{2}{c}{double precision}\\"+"\n"+
                 r"&& first & second & first & second \\ \midrule"+"\n")

    if reference!=None:
        output.write("&IEEE Error & %.2f & %.2f & %.2f & %.2f"%(
                     reference[("Float","Before")],reference[("Float","After")],
                     reference[("Double","Before")], reference[("Double","After")])
                     + r"\\\midrule"+"\n")
                
        
    
    for i in range(1,len(keys)):
        key=keys[i]            
        evalError=computeEvalError(data["Native"], data[key])
        keyConvert={"Randominterlibm": r"\textit{(i)}&interlibm",
                    "Randomverrou":    r"\textit{(ii)}&verrou",
                    "Randomverrou+interlibm":r"\textit{(iii)}&verrou+interlib"}

        lineStr=keyConvert[key]+ " "
        for typeFP in ["Float","Double"]:
            lineStr+=r"&%.2f &  %.2f  "%(evalError["BeforeCorrection_"+typeFP], evalError["AfterCorrection_"+typeFP]) 
        lineStr+=r"\\"+"\n"
        output.write(lineStr)
    output.write(r"\bottomrule"+"\n")    
    output.write(r"\end{tabular}"+"\n")
    output.write(r"\end{center}" +" \n")
    output.write(r"\caption{Number of significant bits for 4~implementations of function $f(a, a+6.ulp(a))$, as assessed by 3~techniques.}"+"\n")
    output.write(r"\label{sdAster}"+"\n")
    output.write(r"\end{table}"+"\n")

    
    


if __name__=="__main__":
    reference=loadRef("reference.dat")
    if len(reference)!=4:
        reference=None
    main(reference)
