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

def transformTemplateForSoftStopStart(lineTab, soft=False):
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


def generateNargs(fileOut, fileNameTemplate, listOfBackend, listOfOp, nargs, post="", roundingTab=[None], soft=False):

    templateStr=open(fileNameTemplate, "r").readlines()
    templateStr=transformTemplateForSoftStopStart(templateStr, soft)

    FctNameRegExp=re.compile("(.*)FCTNAME\(([^,]*),([^)]*)\)(.*)")
    BckNameRegExp=re.compile("(.*)BACKENDFUNC\(([^)]*)\)(.*)")
    BckNameNearestRegExp=re.compile("(.*)BACKEND_NEAREST_FUNC\(([^)]*)\)(.*)")


    for backend in listOfBackend:
        if backend=="mcaquad":
            fileOut.write("#ifdef USE_VERROU_QUADMATH\n")
        for op in listOfOp:
            for rounding in roundingTab:
                if nargs in [1,2]:
                    applyTemplate(fileOut, templateStr, FctNameRegExp, BckNameRegExp, BckNameNearestRegExp, backend,op, post, sign=None, rounding=rounding, soft=soft)
                if nargs==3:
                    sign=""
                    if "msub" in op:
                        sign="-"
                    applyTemplate(fileOut, templateStr, FctNameRegExp,BckNameRegExp,BckNameNearestRegExp, backend, op, post, sign, rounding=rounding, soft=soft)
        if backend=="mcaquad":
            fileOut.write("#endif //USE_VERROU_QUADMATH\n")


def applyTemplate(fileOut, templateStr, FctRegExp, BckRegExp, BckNearestRegExp, backend, op, post, sign=None, rounding=None, soft=False):
    if soft and rounding=="NEAREST":
        return;
    fileOut.write("// generation of operation %s backend %s\n"%(op,backend))
    backendFunc=backend
    if rounding!=None:
        backendFunc=backend+"_"+rounding

    def fctName(typeVal,opt):
        if soft:
            return "vr_"+backendFunc+post+"_soft"+op+typeVal+opt
        else:
            return "vr_"+backendFunc+post+op+typeVal+opt
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
            res=result.group(1) + fctName(result.group(2), result.group(3)) + result.group(4)
            fileOut.write(res+"\n")
            continue
        result=BckRegExp.match(line)
        if result!=None:
            res=result.group(1) + bckName(result.group(2)) + result.group(3)
            fileOut.write(res+"\n")
            if post!="":
                res=result.group(1) + bckNamePost(result.group(2)) + result.group(3)
                res=res.replace(contextName, contextNamePost)
                fileOut.write(res+"\n")
            continue

        result=BckNearestRegExp.match(line)
        if result!=None:
            res=result.group(1) + bckNearestName(result.group(2)) + result.group(3)
            fileOut.write(res+"\n")
            continue

        fileOut.write(line)





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
