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
import itertools
from generateOperatorNames import operatorPrefix

FctNameRegExp=re.compile(r"(.*)FCTNAME\(([^,]*),([^)]*)\)(.*)")
FctConvNameRegExp=re.compile(r"(.*)FCTCONVNAME\(([^,]*),([^)]*)\)(.*)")
FctNameUnfusedRegExp=re.compile(r"(.*)FCTNAMEUNFUSED\(([^,]*),([^)]*)\)(.*)")
FctConvNameUnfusedRegExp=re.compile(r"(.*)FCTCONVNAMEUNFUSED\(([^,]*),([^)]*)\)(.*)")
# BACKEND(type, out, args) : the call of the operator of the backend
# BACKEND_UNFUSED(type, out, args) : the FMA with one call for the mul and one for the add or sub
# BACKEND_NEAREST(type, out, args) : the nearest operator, used when the instrumentation is stopped
BackendRegExp=re.compile(r"(\s*)(BACKEND_NEAREST|BACKEND_UNFUSED|BACKEND)\((.*)\);")
# (regExp, unfused, conv)
FunctionRegExps=[(FctConvNameRegExp, False, True), (FctConvNameUnfusedRegExp, True, True),
                 (FctNameRegExp, False, False), (FctNameUnfusedRegExp, True, False)]


def treatBackend(tab, soft):
    if soft==False:
        return tab
    else:
        res=["if(vr.instrument_soft&&vr.instrument_soft_back){\n"]
        res+=tab
        res+=["}else{\n"]
        res+=[toNearest(line) for line in tab]
        res+=["}\n"]
        return res

def toNearest(line):
    return line.replace("BACKEND_UNFUSED(","BACKEND(").replace("BACKEND(","BACKEND_NEAREST(")


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
    res+=[toNearest(x) for x in elseInstrumentBock]
    res+=["}\n}\n\n"]
    return res




# operatorTables[prefix][opName]=name of the wrapper (VR_OP_ENTRY adds its _naninf variant)
operatorTables={}
# tableGuards[prefix]=preprocessor guards of a table (the sqrt operators have their own guard)
tableGuards={}

def backendGuards(backend):
    return {"mcaquad":["USE_VERROU_QUADMATH"], "vprec":["USE_VERROU_VPREC"]}.get(backend,[])

def roundingGuards(rounding):
    return [] if rounding in ["NEAREST",None] else ["USE_VERROU_OPTIM"]

def writeIfdef(fileOut, guards):
    fileOut.write("".join("#ifdef %s\n"%guard for guard in guards))

def writeEndif(fileOut, guards):
    fileOut.write("".join("#endif //%s\n"%guard for guard in reversed(guards)))

def writeOperatorTables(fileName):
    fileOut=open(fileName,"w")
    fileOut.write("//Generated by %s\n"%(str(sys.argv)[1:-1]))
    allOpNames=sorted({opName for table in operatorTables.values() for opName in table})
    fileOut.write("enum { "+", ".join("VR_OPID_"+opName for opName in allOpNames)+", VR_OPID_NB };\n")
    for prefix in sorted(operatorTables):
        writeIfdef(fileOut, tableGuards[prefix])
        fileOut.write("static const Vr_opEntry vr_table_%s[VR_OPID_NB][VR_NANINF_NB]={\n"%(prefix))
        table=operatorTables[prefix]
        for (sqrtGuard,opNames) in itertools.groupby(sorted(table), lambda opName:opName.startswith("sqrt")):
            guards=["USE_VERROU_SQRT"] if sqrtGuard else []
            writeIfdef(fileOut, guards)
            for opName in opNames:
                fileOut.write("  [VR_OPID_%s]=VR_OP_ENTRY(%s),\n"%(opName,table[opName]))
            writeEndif(fileOut, guards)
        fileOut.write("};\n")
        writeEndif(fileOut, tableGuards[prefix])
    fileOut.close()


def generateOperators(fileOut, fileNameTemplate, listOfBackend, listOfOp, post="", roundingTab=[None], only64=False):
    templateStr=open(fileNameTemplate, "r").readlines()
    skipConv=post in ["check_float_max"]

    for soft in [False, True]:
        softTemplateStr=transformTemplateForSoftStopStart(templateStr, soft, only64)
        for backend in listOfBackend:
            writeIfdef(fileOut, backendGuards(backend))
            for op in listOfOp:
                for rounding in roundingTab:
                    writeIfdef(fileOut, roundingGuards(rounding))
                    guards=backendGuards(backend)+roundingGuards(rounding)
                    for naninf in [False, True]:
                        if naninf:
                            fileOut.write("#ifndef VERROU_IGNORE_NANINF_CHECK\n")
                        applyTemplate(fileOut, softTemplateStr, backend, op, post, rounding=rounding, soft=soft, skipConv=skipConv, naninf=naninf, guards=guards)
                        if naninf:
                            fileOut.write("#endif //VERROU_IGNORE_NANINF_CHECK\n")
                    writeEndif(fileOut, roundingGuards(rounding))
            writeEndif(fileOut, backendGuards(backend))


def applyTemplate(fileOut, templateStr, backend, op, post, rounding=None, soft=False, skipConv=False, naninf=False, guards=[]):
    fileOut.write("// generation of operation %s backend %s%s\n"%(op,backend, " with NaN/Inf check" if naninf else ""))
    nanInfSuffix="_naninf" if naninf else ""

    def fctName(unfused,conv,typeVal,opt):
        prefix=operatorPrefix(backend, rounding, post, soft, unfused, conv)
        name=prefix+op+typeVal+opt
        operatorTables.setdefault(prefix,{})[op+typeVal+opt]=name
        tableGuards[prefix]=guards
        return name+nanInfSuffix

    contextName="backend_"+backend+"_context"
    contextNamePost="backend_"+post+"_context"
    contextNearestName="backend_verrou_null_context"

    def call(name, args, out, context):
        return name+"("+", ".join(args+[out,context])+");"

    def nearestCall(bop, typeVal, args, out, context):
        # the NEAREST operators are inlined in the wrappers (vr_nearest_inline.h)
        return call("vr_nearest_"+bop+"_"+typeVal, args, out, context)

    def backendCalls(bop, typeVal, args, out):
        if rounding=="NEAREST":
            res=[nearestCall(bop, typeVal, args, out, contextName)]
        elif rounding!=None:
            res=[call("interflop_"+backend+"_"+bop+"_"+typeVal+"_"+rounding, args, out, contextName)]
        else:
            res=[call("interflop_"+backend+"_"+bop+"_"+typeVal, args, out, contextName)]
        if post!="" and not (bop=="mul" and post=="checkcancellation"):
            res+=[call("interflop_"+post+"_"+bop+"_"+typeVal, args, out, contextNamePost)]
        return res

    def backendLines(marker, typeVal, out, args):
        if marker=="BACKEND_UNFUSED":
            res=[typeVal+" res_temp;"]
            res+=backendCalls("mul", typeVal, args[:2], "&res_temp")
            res+=backendCalls("add" if op=="madd" else "sub", typeVal, ["res_temp",args[2]], out)
        else:
            bop=op
            if op=="msub": # a msub is a madd with the opposite of the last argument
                bop="madd"
                args=args[:-1]+["-"+args[-1]]
            if marker=="BACKEND_NEAREST":
                res=[nearestCall(bop, typeVal, args, out, contextNearestName)]
            else:
                res=backendCalls(bop, typeVal, args, out)
        if naninf:
            res+=["vr_nanInfCheck_"+("float" if typeVal.endswith("float") else "double")+"(*("+out+"));"]
        return res

    skip=False #the lines of the current function are not generated

    def outputRes(res):
        fileOut.write(res+"\n")

    for line in templateStr:
        result=None
        for (regExp,unfused,conv) in FunctionRegExps:
            result=regExp.match(line)
            if result!=None:
                break
        if result!=None:
            if conv:
                skip=skipConv
            else:
                skip=(rounding=="NEAREST" and soft and (not unfused))
            if not skip:
                outputRes(result.group(1) + fctName(unfused, conv, result.group(2), result.group(3)) + result.group(4))
            continue

        if skip:
            continue

        result=BackendRegExp.match(line)
        if result!=None:
            (typeVal,out,*args)=[field.strip() for field in result.group(3).split(",")]
            for res in backendLines(result.group(2), typeVal, out, args):
                outputRes(result.group(1)+res)
            continue

        if line.endswith("\n"):
            line=line[0:-1]
        outputRes(line)




if __name__=="__main__":
    fileNameOutput="vr_generated_from_templates.h"
    fileOut=open(fileNameOutput,"w")
    fileOut.write("//Generated by %s\n"%(str(sys.argv)[1:-1]))

    roundingTab=["NEAREST", "UPWARD", "DOWNWARD", "FARTHEST", "ZERO", "AWAY_ZERO"]+[rnd + det for rnd in ["RANDOM", "NEARNESS", "PRANDOM"] for det in ["","_DET","_COMDET" ]]
    roundingTab+=[rnd + det for rnd in ["RANDOM", "NEARNESS"] for det in ["_SCOMDET" ]]
    roundingTab+=["SR_MONOTONIC","SR_SMONOTONIC"]

    allBackends=["verrou","mcaquad","vprec","checkdenorm"]

    def generateVerrouOnly(template, listOfOp, floatRounding=True):
        generateOperators(fileOut, template, ["verrou"], listOfOp, post="check_float_max")
        generateOperators(fileOut, template, ["verrou"], listOfOp, roundingTab=roundingTab)
        if floatRounding:
            generateOperators(fileOut, template, ["verrou"], listOfOp, roundingTab=["FLOAT"], only64=True)

    template="vr_interp_operator_template_cast.h"
    generateOperators(fileOut, template, allBackends, ["cast"])
    generateVerrouOnly(template, ["cast"], floatRounding=False)

    template="vr_interp_operator_template_1args.h"
    writeIfdef(fileOut, ["USE_VERROU_SQRT"])
    generateOperators(fileOut, template, ["verrou","checkdenorm","vprec"], ["sqrt"])
    generateVerrouOnly(template, ["sqrt"])
    writeEndif(fileOut, ["USE_VERROU_SQRT"])

    template="vr_interp_operator_template_2args.h"
    listOfOp=["add","sub","mul","div"]
    generateOperators(fileOut, template, allBackends, listOfOp)
    generateVerrouOnly(template, listOfOp)
    generateOperators(fileOut, template, allBackends, ["add","sub"], post="checkcancellation")

    template="vr_interp_operator_template_3args.h"
    listOfOp=["madd","msub"]
    generateOperators(fileOut, template, allBackends, listOfOp)
    generateOperators(fileOut, template, allBackends, listOfOp, post="checkcancellation")
    generateVerrouOnly(template, listOfOp)

    fileOut.close()

    writeOperatorTables("vr_instrumentOp_tables.h")
