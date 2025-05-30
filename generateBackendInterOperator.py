#!/usr/bin/env python3

# This file is part of Verrou, a FPU instrumentation tool.

# Copyright (C) 2014-2021 EDF
#   F. Févotte <francois.fevotte@edf.fr>
#   B. Lathuilière <bruno.lathuiliere@edf.fr>


# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.

# The GNU Lesser General Public License is contained in the file COPYING.

import sys
import re

FctNameRegExp=re.compile("(.*)FCTNAME\(([^,]*),([^)]*)\)(.*)")
FctConvNameRegExp=re.compile("(.*)FCTCONVNAME\(([^,]*),([^)]*)\)(.*)")
FctNameUnfusedRegExp=re.compile("(.*)FCTNAMEUNFUSED\(([^,]*),([^)]*)\)(.*)")
FctConvNameUnfusedRegExp=re.compile("(.*)FCTCONVNAMEUNFUSED\(([^,]*),([^)]*)\)(.*)")
BckNameRegExp=re.compile("(.*)BACKENDFUNC\(([^)]*)\)(.*)")
BckNameFirstRegExp=re.compile("(.*)BACKEND_FIRST_FUNC\(([^)]*)\)(.*)")
BckNameSecondRegExp=re.compile("(.*)BACKEND_SECOND_FUNC\(([^)]*)\)(.*)")
BckNameNearestRegExp=re.compile("(.*)BACKEND_NEAREST_FUNC\(([^)]*)\)(.*)")


def mergeFused(tab,tmpVar="res_temp"):
    fusedTab=[]
    for i in range(len(tab)):
        line=tab[i]
        if tmpVar+";" in line:
            tab[i]= "//"+line

        if "_FIRST_" in line:
            prefix=tab[0:max(0,i-1)]
            first=tab[i]
            second=tab[i+1]
            postfix=tab[i+2:]
            break
    if not "_SECOND_" in second:
            print("Generation failure")
    return [line for line in prefix] + [mergeTwoLines(first,second, tmpVar)] +[line for line in postfix]

def mergeTwoLines(first,second, varInter):
    first=first.replace("_FIRST_","")
    second=second.replace("_SECOND_","")
    res=(first.partition("&"+varInter))[0] + (second.partition(varInter+","))[2]
    return res


def treatBackend(tab, soft):
    if soft==False:
        return tab
    else:
        res=["if(vr.instrument_soft&&vr.instrument_soft_back){\n"]
        res+=tab
        res+=["}else{\n"]
        if any(["BACKEND_FIRST" in line for line in tab]):
            res+=mergeFused(tab)
        else:
            res+=[line.replace("BACKENDFUNC", "BACKEND_NEAREST_FUNC").replace("CONTEXT","BACKEND_NEAREST_CONTEXT") for line in tab]
        res+=["}\n"]
        return res


def transformTemplateForSoftStopStart(lineTab, soft=False, only64=False):
    func=[]
    dicOfFunc={}

    splitActive=False
    tab=[]
    for line in lineTab:
        resultRegExp=None
        for (FCTNAME, regExp) in [("FCTNAME", FctNameRegExp),
                                  ("FCTCONVNAME", FctConvNameRegExp),
                                  ("FCTNAMEUNFUSED", FctNameUnfusedRegExp),
                                  ("FCTCONVNAMEUNFUSED", FctConvNameUnfusedRegExp)]:
            resultRegExp=regExp.match(line)  #"(.*)FCTNAME\(([^,]*),([^)]*)\)(.*)")
            if resultRegExp!=None:
                break
        if resultRegExp!=None:
            typeVal=resultRegExp.group(2)
            opt=resultRegExp.group(3)
            newName=FCTNAME+"("+typeVal+","+opt+")"
            if splitActive:
                dicOfFunc[currentName]=tab
                currentName=newName
                tab=[line]
                continue
            else:
                splitActive=True
                currentName=newName

        if splitActive:
            tab+=[line]
    dicOfFunc[currentName]=tab
    #end of split
    res=[]

    for name in dicOfFunc:
        if "CONV" in name:
            if ("64F" in name) or (not only64):
                res+=transformTemplateForSoftStopStartOneConvFunction(dicOfFunc[name], dicOfFunc[name.replace("CONV","")],  soft)
        else:
            if ("64F" in name) or (not only64):
                res+=transformTemplateForSoftStopStartOneFunction(dicOfFunc[name], soft)
    return res

def transformTemplateForSoftStopStartOneFunction(lineTab, soft):
    res=[]
    back=[]
    status="around"
    for line in lineTab:
        if "PREBACKEND" in line:
            status="back"
            continue
        if "POSTBACKEND" in line:
            res+=treatBackend(back,soft)
            back=[]
            status="around"
            continue
        if status=="around":
            res+=[line]
        if status=="back":
            back+=[line]
    return res

def transformTemplateForSoftStopStartOneConvFunction(lineTabConv, lineTab, soft):
    if soft==False:
        return transformTemplateForSoftStopStartOneFunction(lineTabConv,soft)
    #utility function to parse function with linetab format
    def getFunctionName(lineFunctionTab):
        return lineFunctionTab[0]
    def getFunctionBloc(lineFunctionTab):
        remain=lineFunctionTab[1:]
        while remain[-1].strip()!="}":
            remain=remain[0:-1]
        return remain[0:-1]

    res=[getFunctionName(lineTabConv)]
    res+=["if(vr.instrument_soft && vr.instrument_soft_back){\n"]
    res+= [x for x in  getFunctionBloc(lineTabConv) if not (("PREBACKEND" in x) or ("POSTBACKEND" in x )) ]
    res+=["}else{\n"]

    elseInstrumentBock=[x for x in  getFunctionBloc(lineTab) if not (("PREBACKEND" in x) or ("POSTBACKEND" in x )) ]
    if any(["BACKEND_FIRST" in line for line in elseInstrumentBock]):
        elseInstrumentBock=mergeFused(elseInstrumentBock)
    res+=[x.replace("BACKENDFUNC", "BACKEND_NEAREST_FUNC").replace("CONTEXT","BACKEND_NEAREST_CONTEXT") for x in elseInstrumentBock]
    res+=["}\n}\n\n"]
    return res




def generateNargs(fileOut, fileNameTemplate, listOfBackend, listOfOp, nargs, post="", roundingTab=[None], soft=False, only64=False):
    commentConv=False
    templateStr=open(fileNameTemplate, "r").readlines()

    templateStr=transformTemplateForSoftStopStart(templateStr, soft, only64)
    if post in ["check_float_max"]:
        commentConv=True

    for backend in listOfBackend:
        if backend=="mcaquad":
            fileOut.write("#ifdef USE_VERROU_QUADMATH\n")

        for op in listOfOp:
            for rounding in roundingTab:
                if nargs in [1,2]:
                    applyTemplate(fileOut, templateStr, backend,op, post, sign=None, rounding=rounding, soft=soft, commentConv=commentConv)
                if nargs==3:
                    sign=""
                    if "msub" in op:
                        sign="-"
                    applyTemplate(fileOut, templateStr, backend, op, post, sign, rounding=rounding, soft=soft, commentConv=commentConv)
        if backend=="mcaquad":
            fileOut.write("#endif //USE_VERROU_QUADMATH\n")


def applyTemplate(fileOut, templateStr, backend, op, post, sign=None, rounding=None, soft=False, commentConv=False):
    fileOut.write("// generation of operation %s backend %s\n"%(op,backend))
    backendFunc=backend
    if rounding!=None:
        backendFunc=backend+"_"+rounding

    def fctName(unfused,conv,typeVal,opt):
        vrPrefix="vr_"
        if unfused:
            vrPrefix+="unfused_"
        if conv:
            vrPrefix+="conv_"
        if soft:
            return vrPrefix+backendFunc+post+"_soft"+op+typeVal+opt
        else:
            return vrPrefix+backendFunc+post+op+typeVal+opt

    def localOp(first):
        localOp=op
        if sign=="-":
            localOp=op.replace("sub","add")
        if op in ["madd","msub"] and first!=None:
            if first==True:
                localOp="mul"
            if first==False:
                if op=="madd":
                    localOp="add"
                else:
                    localOp="sub"
        return localOp

    def bckName(typeVal, first=None):
        if rounding!=None:
            return "interflop_"+backend+"_"+localOp(first)+"_"+typeVal+"_"+rounding
        return "interflop_"+backend+"_"+localOp(first)+"_"+typeVal

    def bckNearestName(typeVal):
        if rounding!=None:
            return (bckName(typeVal)).replace(rounding,"NEAREST")
        if sign!="-":
            return "interflop_verrou_"+op+"_"+typeVal+"_NEAREST"
        else:
            return "interflop_verrou_"+op.replace("sub","add")+"_"+typeVal+"_NEAREST"

    def bckNamePost(typeVal,first=None):
        bop=localOp(first)
        if bop in ["mul"] and post=="checkcancellation":
            return None
        return "interflop_"+post+"_"+bop+"_"+typeVal


    contextName="backend_"+backend+"_context"
    contextNamePost="backend_"+post+"_context"
    contextNearestName="backend_verrou_null_context"

    comment=None

    def outputRes(res,localComment):
        line=None
        if localComment:
            if res in ["","//"]:
                line=res+"\n"
            else:
                if "/*" in res:
                    line="/*"+ res.replace("/*", "%%").replace("*/","%%")+"*/"
                else:
                    line="/*"+ res+"*/\n"
        else:
            line=res+"\n"
        fileOut.write(line)

    for line in templateStr:
        if "BACKEND_NEAREST_CONTEXT" in line:
            line=line.replace("BACKEND_NEAREST_CONTEXT", contextNearestName)
        if "CONTEXT" in line:
            line=line.replace("CONTEXT", contextName)
        if "SIGN" in line:
            if sign!=None:
                line=line.replace("SIGN", sign)
            else:
                print("Generation failed")
                sys.exit()


        result=FctConvNameRegExp.match(line)
        fused=False
        if result==None:
            result=FctConvNameUnfusedRegExp.match(line)
            fused=True
        if result!=None:
            if commentConv:
                comment=True
            else:
                comment=False
            typeVal=result.group(2)
            opt=result.group(3)
            res=result.group(1) + fctName(fused,True, typeVal, opt) + result.group(4)
            outputRes(res,comment)
            continue

        result=FctNameRegExp.match(line)
        unfused=False
        if result==None:
            result=FctNameUnfusedRegExp.match(line)
            unfused=True
        if result!=None:
            comment=False
            typeVal=result.group(2)
            opt=result.group(3)
            if rounding=="NEAREST" and soft and (not unfused):
                comment=True
            res=result.group(1) + fctName(unfused,False,typeVal, opt) + result.group(4)
            outputRes(res,comment)
            continue

        result=None
        first=None
        for (rgExp,firstVar) in [(BckNameFirstRegExp, True), (BckNameSecondRegExp,False) , (BckNameRegExp,None)]:
            result=rgExp.match(line)
            if result!=None:
                first=firstVar
                break
        if result!=None:
            group3=result.group(3)
            if first==False:
                group3=group3.replace(" - "," ")
            res=result.group(1) + bckName(result.group(2),first) + group3
            outputRes(res,comment)
            if post!="":
                bnPost=bckNamePost(result.group(2),first)
                if bnPost!=None:
                    res=result.group(1) + bnPost+ group3
                    res=res.replace(contextName, contextNamePost)
                    outputRes(res,comment)
            continue

        result=BckNameNearestRegExp.match(line)
        if result!=None:
            res=result.group(1) + bckNearestName(result.group(2)) + result.group(3)
            outputRes(res,comment)
            continue
        if line.endswith("\n"):
            line=line[0:-1]
        outputRes(line,comment)




if __name__=="__main__":
    fileNameOutput="vr_generated_from_templates.h"
    fileOut=open(fileNameOutput,"w")
    fileOut.write("//Generated by %s\n"%(str(sys.argv)[1:-1]))

    roundingTab=["NEAREST", "UPWARD", "DOWNWARD", "FARTHEST", "ZERO", "AWAY_ZERO"]+[rnd + det for rnd in ["RANDOM", "AVERAGE", "PRANDOM"] for det in ["","_DET","_COMDET" ]]
    roundingTab+=[rnd + det for rnd in ["RANDOM", "AVERAGE"] for det in ["_SCOMDET" ]]
    roundingTab+=["SR_MONOTONIC","SR_SMONOTONIC"]

    template1Args="vr_interp_operator_template_cast.h"
    listOfOp1Args=["cast"]
    generateNargs(fileOut,template1Args, ["verrou","mcaquad","checkdenorm"], listOfOp1Args, 1)
    generateNargs(fileOut,template1Args, ["verrou","mcaquad","checkdenorm"], listOfOp1Args, 1,soft=True)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, post="check_float_max")
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, post="check_float_max",soft=True)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, roundingTab=roundingTab)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, roundingTab=roundingTab, soft=True)

    template1Args="vr_interp_operator_template_1args.h"
    listOfOp1Args=["sqrt"]
    fileOut.write("#ifdef USE_VERROU_SQRT\n")
    generateNargs(fileOut,template1Args, ["verrou","checkdenorm"], listOfOp1Args, 1)
    generateNargs(fileOut,template1Args, ["verrou","checkdenorm"], listOfOp1Args, 1,soft=True)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, post="check_float_max")
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, post="check_float_max",soft=True)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, roundingTab=roundingTab)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, roundingTab=roundingTab, soft=True)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, roundingTab=["FLOAT"], only64=True)
    generateNargs(fileOut,template1Args, ["verrou"], listOfOp1Args, 1, roundingTab=["FLOAT"], soft=True, only64=True)
    fileOut.write("#endif\n")

    template2Args="vr_interp_operator_template_2args.h"
    listOfOp2Args=["add","sub","mul","div"]
    generateNargs(fileOut,template2Args, ["verrou","mcaquad","checkdenorm"], listOfOp2Args, 2)
    generateNargs(fileOut,template2Args, ["verrou","mcaquad","checkdenorm"], listOfOp2Args, 2, soft=True)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, post="check_float_max")
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, post="check_float_max", soft=True)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, roundingTab=roundingTab)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, roundingTab=roundingTab, soft=True)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, roundingTab=["FLOAT"], only64=True)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, roundingTab=["FLOAT"], soft=True, only64=True)


    listOfOp2Args=["add","sub"]
    generateNargs(fileOut,template2Args, ["verrou","mcaquad","checkdenorm"], listOfOp2Args, 2, post="checkcancellation")
    generateNargs(fileOut,template2Args, ["verrou","mcaquad","checkdenorm"], listOfOp2Args, 2, post="checkcancellation",soft=True)

    template3Args="vr_interp_operator_template_3args.h"
    listOfOp3Args=["madd","msub"]
    generateNargs(fileOut,template3Args, ["verrou","mcaquad","checkdenorm"], listOfOp3Args, 3)
    generateNargs(fileOut,template3Args, ["verrou","mcaquad","checkdenorm"], listOfOp3Args, 3, soft=True)
    generateNargs(fileOut,template3Args, ["verrou","mcaquad","checkdenorm"], listOfOp3Args, 3, post="checkcancellation")
    generateNargs(fileOut,template3Args, ["verrou","mcaquad","checkdenorm"], listOfOp3Args, 3, post="checkcancellation",soft=True)
    generateNargs(fileOut,template3Args, ["verrou"], listOfOp3Args, 3, post="check_float_max")
    generateNargs(fileOut,template3Args, ["verrou"], listOfOp3Args, 3, post="check_float_max",soft=True)
    generateNargs(fileOut,template3Args, ["verrou"], listOfOp3Args, 3, roundingTab=roundingTab)
    generateNargs(fileOut,template3Args, ["verrou"], listOfOp3Args, 3, roundingTab=roundingTab,soft=True)
    generateNargs(fileOut,template3Args, ["verrou"], listOfOp3Args, 3, roundingTab=["FLOAT"], only64=True)
    generateNargs(fileOut,template3Args, ["verrou"], listOfOp3Args, 3, roundingTab=["FLOAT"],soft=True, only64=True)

    fileOut.close()
