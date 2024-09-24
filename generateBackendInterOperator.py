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

def treatBackend(tab, soft):
    if soft==False:
        return tab
    else:
        res=["if(vr.instrument_soft){\n"]
        res+=tab
        res+=["}else{\n"]
        res+=[line.replace("BACKENDFUNC", "BACKEND_NEAREST_FUNC").replace("CONTEXT","BACKEND_NEAREST_CONTEXT") for line in tab]
        res+=["}\n"]
        return res


def transformTemplateForSoftStopStart(nameRegExp,convNameRegExp,
                                      lineTab, soft=False):
    func=[]
    dicOfFunc={}

    splitActive=False
    tab=[]
    for line in lineTab:
        result=nameRegExp.match(line)  #"(.*)FCTNAME\(([^,]*),([^)]*)\)(.*)")
        if result!=None:
            typeVal=result.group(2)
            opt=result.group(3)
            newName="FCTNAME("+typeVal+","+opt+")"
            if splitActive:
                dicOfFunc[currentName]=tab
                currentName=newName
                tab=[line]
                continue
            else:
                splitActive=True
                currentName=newName

        result=convNameRegExp.match(line)  #"(.*)FCTCONVNAME\(([^,]*),([^)]*)\)(.*)")
        if result!=None:
            typeVal=result.group(2)
            opt=result.group(3)
            newName="FCTCONVNAME("+typeVal+","+opt+")"
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
            res+=transformTemplateForSoftStopStartOneConvFunction(dicOfFunc[name], dicOfFunc[name.replace("CONV","")],  soft)
        else:
            res+=transformTemplateForSoftStopStartOneFunction(dicOfFunc[name], soft)
    return res

def transformTemplateForSoftStopStartOneFunction(lineTab, soft):
    res=[]
    back=[]
    status="pre"
    for line in lineTab:
        if "PREBACKEND" in line:
            status="back"
            continue
        if "POSTBACKEND" in line:
            res+=treatBackend(back,soft)
            back=[]
            status="pre"
            continue
        if status=="pre":
            res+=[line]
        if status=="back":
            back+=[line]
    return res

def transformTemplateForSoftStopStartOneConvFunction(lineTabConv, lineTab, soft):
    if soft==False:
        return transformTemplateForSoftStopStartOneFunction(lineTabConv,soft)

    res=[lineTabConv[0]]
    remain=lineTabConv[1:]
    while remain[-1].strip()!="}":
        remain=remain[0:-1]

    res+=["if(vr.instrument_soft){\n"]
    for x in remain[0:-1]:
        if ("PREBACKEND" in x) or ("POSTBACKEND" in x):
            continue
        res+=[x]

    res+=["}else{\n"]

    remain=lineTab[1:]
    while remain[-1].strip()!="}":
        remain=remain[0:-1]

    for x in remain[0:-1]:
        if ("PREBACKEND" in x) or ("POSTBACKEND" in x):
            continue
        res+=[x.replace("BACKENDFUNC", "BACKEND_NEAREST_FUNC").replace("CONTEXT","BACKEND_NEAREST_CONTEXT")]

    res+=["}\n}\n\n"]
    return res




def generateNargs(fileOut, fileNameTemplate, listOfBackend, listOfOp, nargs, post="", roundingTab=[None], soft=False):
    commentConv=False
    templateStr=open(fileNameTemplate, "r").readlines()

    FctNameRegExp=re.compile("(.*)FCTNAME\(([^,]*),([^)]*)\)(.*)")
    FctConvNameRegExp=re.compile("(.*)FCTCONVNAME\(([^,]*),([^)]*)\)(.*)")
    templateStr=transformTemplateForSoftStopStart(FctNameRegExp, FctConvNameRegExp,  templateStr, soft)

    BckNameRegExp=re.compile("(.*)BACKENDFUNC\(([^)]*)\)(.*)")
    BckNameNearestRegExp=re.compile("(.*)BACKEND_NEAREST_FUNC\(([^)]*)\)(.*)")

    if post in ["check_float_max"]:
        commentConv=True

    for backend in listOfBackend:
        if backend=="mcaquad":
            fileOut.write("#ifdef USE_VERROU_QUADMATH\n")

        for op in listOfOp:
            for rounding in roundingTab:
                if nargs in [1,2]:
                    applyTemplate(fileOut, templateStr, FctNameRegExp, FctConvNameRegExp, BckNameRegExp, BckNameNearestRegExp, backend,op, post, sign=None, rounding=rounding, soft=soft, commentConv=commentConv)
                if nargs==3:
                    sign=""
                    if "msub" in op:
                        sign="-"
                    applyTemplate(fileOut, templateStr, FctNameRegExp, FctConvNameRegExp, BckNameRegExp,BckNameNearestRegExp, backend, op, post, sign, rounding=rounding, soft=soft, commentConv=commentConv)
        if backend=="mcaquad":
            fileOut.write("#endif //USE_VERROU_QUADMATH\n")


def applyTemplate(fileOut, templateStr, FctRegExp, FctConvRegExp, BckRegExp, BckNearestRegExp, backend, op, post, sign=None, rounding=None, soft=False, commentConv=False):
    fileOut.write("// generation of operation %s backend %s\n"%(op,backend))
    backendFunc=backend
    if rounding!=None:
        backendFunc=backend+"_"+rounding

    def fctName(conv,typeVal,opt):
        vrPrefix="vr_"
        if conv:
            vrPrefix="vr_conv_"
        if soft:
            return vrPrefix+backendFunc+post+"_soft"+op+typeVal+opt
        else:
            return vrPrefix+backendFunc+post+op+typeVal+opt

    def bckName(typeVal):
        if sign!="-":
            if rounding!=None:
                return "interflop_"+backend+"_"+op+"_"+typeVal+"_"+rounding
            return "interflop_"+backend+"_"+op+"_"+typeVal
        else:
            if rounding!=None:
                return "interflop_"+backend+"_"+op.replace("sub","add")+"_"+typeVal+"_"+rounding
            return "interflop_"+backend+"_"+op.replace("sub","add")+"_"+typeVal
    def bckNearestName(typeVal):
        if rounding!=None:
            return (bckName(typeVal)).replace(rounding,"NEAREST")
        if sign!="-":
            return "interflop_verrou_"+op+"_"+typeVal+"_NEAREST"
        else:
            return "interflop_verrou_"+op.replace("sub","add")+"_"+typeVal+"_NEAREST"

    def bckNamePost(typeVal):
        if sign!="-":
            return "interflop_"+post+"_"+op+"_"+typeVal
        else:
            return "interflop_"+post+"_"+op.replace("sub","add")+"_"+typeVal


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
        result=FctRegExp.match(line)
        if result!=None:
            comment=False

            typeVal=result.group(2)
            opt=result.group(3)
            if rounding=="NEAREST" and soft:
                comment=True
            res=result.group(1) + fctName(False,typeVal, opt) + result.group(4)
            outputRes(res,comment)
            continue
        result=FctConvRegExp.match(line)
        if result!=None:
            if commentConv:
                comment=True
            else:
                comment=False
            typeVal=result.group(2)
            opt=result.group(3)
            res=result.group(1) + fctName(True, typeVal, opt) + result.group(4)
            outputRes(res,comment)
            continue
        result=BckRegExp.match(line)
        if result!=None:
            res=result.group(1) + bckName(result.group(2)) + result.group(3)
            outputRes(res,comment)
            if post!="":
                res=result.group(1) + bckNamePost(result.group(2)) + result.group(3)
                res=res.replace(contextName, contextNamePost)
                outputRes(res,comment)
            continue

        result=BckNearestRegExp.match(line)
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

    roundingTab=["NEAREST", "UPWARD", "DOWNWARD", "FARTHEST", "ZERO", "AWAY_ZERO", "FLOAT"]+[rnd + det for rnd in ["RANDOM", "AVERAGE", "PRANDOM"] for det in ["","_DET","_COMDET" ]]
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
    fileOut.write("#endif\n")

    template2Args="vr_interp_operator_template_2args.h"
    listOfOp2Args=["add","sub","mul","div"]
    generateNargs(fileOut,template2Args, ["verrou","mcaquad","checkdenorm"], listOfOp2Args, 2)
    generateNargs(fileOut,template2Args, ["verrou","mcaquad","checkdenorm"], listOfOp2Args, 2, soft=True)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, post="check_float_max")
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, post="check_float_max", soft=True)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, roundingTab=roundingTab)
    generateNargs(fileOut,template2Args, ["verrou"], listOfOp2Args, 2, roundingTab=roundingTab, soft=True)


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
    fileOut.close()
