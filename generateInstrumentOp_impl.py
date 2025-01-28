#!/usr/bin/python3

import sys

includeAndUndef="""#include "vr_instrumentOp_impl.h"
#undef bcName
#undef bcNameWithCC
#undef bcNameConv
#undef bcNameConvWithCC
#undef bcNameWithCCUnfused
#undef bcNameConvWithCCUnfused
"""

includePatternHardWithoutConv="""
#define bcName(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCC(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING"#OP, vr_unfused_verrou_ROUNDING##OP
#define bcNameConv(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameConvWithCC(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameConvWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING"#OP, vr_unfused_verrou_ROUNDING##OP
"""+includeAndUndef

includePatternHardWithConv="""
#define bcName(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCC(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING"#OP, vr_unfused_verrou_ROUNDING##OP
#define bcNameConv(OP) "vr_conv_verrou_ROUNDING"#OP, vr_conv_verrou_ROUNDING##OP
#define bcNameConvWithCC(OP) "vr_conv_verrou_ROUNDING"#OP, vr_conv_verrou_ROUNDING##OP
#define bcNameConvWithCCUnfused(OP) "vr_unfused_conv_verrou_ROUNDING"#OP, vr_unfused_conv_verrou_ROUNDING##OP
"""+includeAndUndef

floatConvPattern="""\tif(!vr.float_conv){%s\n\t}else{//vr.float_conv%s\n\t}//end float_conv\n"""

includePatternHard=floatConvPattern%(includePatternHardWithoutConv,includePatternHardWithConv)

includePatternSoftWithoutConv="""
#define bcName(OP) "vr_verrou_ROUNDING_soft"#OP, vr_verrou_ROUNDING_soft##OP
#define bcNameWithCC(OP) "vr_verrou_ROUNDING_soft"#OP, vr_verrou_ROUNDING_soft##OP
#define bcNameWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING_soft"#OP, vr_unfused_verrou_ROUNDING_soft##OP
#define bcNameConv(OP) "vr_verrou_ROUNDING_soft"#OP, vr_verrou_ROUNDING_soft##OP
#define bcNameConvWithCC(OP) "vr_verrou_ROUNDING_soft"#OP, vr_verrou_ROUNDING_soft##OP
#define bcNameConvWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING_soft"#OP, vr_unfused_verrou_ROUNDING_soft##OP
"""+includeAndUndef

includePatternSoftWithConv="""
#define bcName(OP) "vr_verrou_ROUNDING_soft"#OP, vr_verrou_ROUNDING_soft##OP
#define bcNameWithCC(OP) "vr_verrou_ROUNDING_soft"#OP, vr_verrou_ROUNDING_soft##OP
#define bcNameWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING_soft"#OP, vr_unfused_verrou_ROUNDING_soft##OP
#define bcNameConv(OP) "vr_conv_verrou_ROUNDING_soft"#OP, vr_conv_verrou_ROUNDING_soft##OP
#define bcNameConvWithCC(OP) "vr_conv_verrou_ROUNDING_soft"#OP, vr_conv_verrou_ROUNDING_soft##OP
#define bcNameConvWithCCUnfused(OP) "vr_unfused_conv_verrou_ROUNDING_soft"#OP, vr_unfused_conv_verrou_ROUNDING_soft##OP
"""+includeAndUndef

includePatternSoft=floatConvPattern%(includePatternSoftWithoutConv, includePatternSoftWithConv)

includePatternSoftNearest="""\tif(!vr.float_conv){
#define bcName(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCC(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING_soft"#OP, vr_unfused_verrou_ROUNDING_soft##OP
#define bcNameConv(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameConvWithCC(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameConvWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING_soft"#OP, vr_unfused_verrou_ROUNDING_soft##OP
"""+includeAndUndef+"""
\t}else{//vr.float_conv
#define bcName(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCC(OP) "vr_verrou_ROUNDING"#OP, vr_verrou_ROUNDING##OP
#define bcNameWithCCUnfused(OP) "vr_unfused_verrou_ROUNDING_soft"#OP, vr_unfused_verrou_ROUNDING_soft##OP
#define bcNameConv(OP) "vr_conv_verrou_ROUNDING_soft"#OP, vr_conv_verrou_ROUNDING_soft##OP
#define bcNameConvWithCC(OP) "vr_conv_verrou_ROUNDING_soft"#OP, vr_conv_verrou_ROUNDING_soft##OP
#define bcNameConvWithCCUnfused(OP) "vr_unfused_conv_verrou_ROUNDING_soft"#OP, vr_unfused_conv_verrou_ROUNDING_soft##OP
"""+includeAndUndef+"""
\t}//end float_conv
"""


roundingList=[["NEAREST","NATIVE"],
              ["RANDOM"], ["AVERAGE"],
              ["RANDOM_DET"],["AVERAGE_DET"],
              ["RANDOM_COMDET"],["AVERAGE_COMDET"],
              ["RANDOM_SCOMDET"],["AVERAGE_SCOMDET"],
              ["SR_MONOTONIC"],["SR_SMONOTONIC"],
              ["UPWARD"],["DOWNWARD"],["ZERO"],
              ["AWAY_ZERO"],["FARTHEST"],["FLOAT"],
              ["PRANDOM"], ["PRANDOM_DET"], ["PRANDOM_COMDET"]
              ]

def replaceCC(lines, patternWithCC, replaceWithCC, patternWithoutCC, replaceWithoutCC):
    res=""
    for line in lines.split("\n"):
        if "WithCC" in line:
            res+=line.replace(patternWithCC, replaceWithCC)+"\n"
        else:
            res+=line.replace(patternWithoutCC, replaceWithoutCC)+"\n"
    return res+"\n"

def replaceConv(lines, patternWithConv, replaceWithConv, patternWithoutConv, replaceWithoutConv):
    res=""
    for line in lines.split("\n"):
        if "Conv" in line:
            res+=line.replace(patternWithConv, replaceWithConv)+"\n"
        else:
            res+=line.replace(patternWithoutConv, replaceWithoutConv)+"\n"
    return res+"\n"


def generateVerrouRounding(handler,roundingList):
    for rounding in roundingList:
        roundingMode=rounding[0]
        checkStr="if(vr.roundingMode==VR_"+roundingMode
        for checkRounding in rounding[1:]:
            checkStr+= " ||  vr.roundingMode==VR_" +checkRounding
        checkStr+="){\n"

        includeMacroHard=includePatternHard.replace("ROUNDING",roundingMode)
        includeMacroSoft=includePatternSoft.replace("ROUNDING",roundingMode)
        if roundingMode=="NEAREST":
            includeMacroSoft=includePatternSoftNearest.replace("ROUNDING",roundingMode)
        if roundingMode=="FLOAT":
            includeMacroHard=replaceConv(includePatternHard, "ROUNDING", roundingMode, "ROUNDING", "NEAREST")
            includeMacroSoft=replaceConv(includePatternSoft, "ROUNDING", roundingMode, "ROUNDING_soft", "NEAREST")

        includeMacro="""\tif(vr.instrument_soft_used){\n""" + includeMacroSoft+"\t}else{//instrument hard\n"+includeMacroHard+"\t}\n"
        strWrite=checkStr + includeMacro+ "}\n"
        handler.write(strWrite)

def checkPostbackend(backendName, postBackendList, postBackendName):
    res="vr.backend=="+backendName+" "
    for postBackend in postBackendList:
        if postBackend["postBackendName"]=="":
            continue
        res+= "&& "
        neg=""
        if postBackend["postBackendName"]!=postBackendName:
            neg="! "
        if postBackend["boolCheck"]!=None:
            res+= neg+postBackend["boolCheck"]
    return res




def generateVerrouGeneric(handler, backendEnum="vr_verrou", backendName="verrou", postBackendList=None):
    if postBackendList==None:
        includeMacroHard=includePatternHard.replace("_ROUNDING","")
        includeMacroSoft=includePatternSoft.replace("_ROUNDING","")

        includeMacro="\tif(vr.instrument_soft_used){\n" + includeMacroSoft+"\n\t}else{//hard\n"+includeMacroHard+"\n\t}//end vr.instrument_soft_used)\n"
        handler.write(includeMacro)
    else:
        for postBackend in postBackendList:
            res="\nif(" +checkPostbackend(backendEnum, postBackendList,postBackend["postBackendName"])+"){\n"
            if postBackend["convFloat"]:
                includeMacroHard=includePatternHard.replace("_ROUNDING",postBackend["postBackendName"])
                includeMacroSoft=includePatternSoft.replace("_ROUNDING",postBackend["postBackendName"])
                if postBackend["ccOnly"]:
                    includeMacroHard=replaceCC(includePatternHard,"_ROUNDING",postBackend["postBackendName"], "_ROUNDING", "" )
                    includeMacroSoft=replaceCC(includePatternSoft,"_ROUNDING",postBackend["postBackendName"], "_ROUNDING", "" )

                includeMacro="\tif(vr.instrument_soft_used){\n" + includeMacroSoft+"\n\t}else{//hard\n"+includeMacroHard+"\n\t}//end vr.instrument_soft_used)\n"
                res+=includeMacro
            else:
                includeMacroHard=includePatternHardWithoutConv.replace("_ROUNDING",postBackend["postBackendName"])
                includeMacroSoft=includePatternSoftWithoutConv.replace("_ROUNDING",postBackend["postBackendName"])
                if postBackend["ccOnly"]:
                    includeMacroHard=replaceCC(includePatternHardWithoutConv,"_ROUNDING",postBackend["postBackendName"], "_ROUNDING", "" )
                    includeMacroSoft=replaceCC(includePatternSoftWithoutConv,"_ROUNDING",postBackend["postBackendName"], "_ROUNDING", "" )
                includeMacro="\tif(vr.instrument_soft_used){\n" + includeMacroSoft+"\n\t}else{//hard\n"+includeMacroHard+"\n\t}//end vr.instrument_soft_used)\n"
                res+=includeMacro
            res+="}\n"
            res=res.replace("\n\n","\n")
            res=res.replace("\n\n","\n")
            if backendName!="verrou":
                handler.write(res.replace("verrou",backendName))
            else:
                handler.write(res)


if __name__=="__main__":

    fileName="vr_instrumentOp_impl_generated_verrou_specific.h"
    handler=open(fileName,"w")
    handler.write("// Generated by %s\n"%( str(sys.argv)[1:-1] ))
    handler.write("#ifndef DEBUG_PRINT_OP //the switch case with rounding mode during instrumentation is incompatible with DEBUG_PRINT_OP (to make it compatible python code generation need to be adapted)\n")
    generateVerrouRounding(handler,roundingList)
    handler.write("#endif\n")
    generateVerrouGeneric(handler)

    fileName="vr_instrumentOp_impl_generated_all_backends.h"
    handler=open(fileName,"w")
    handler.write("// Generated by %s\n"%( str(sys.argv)[1:-1] ))
    generateVerrouGeneric(handler, backendEnum="vr_verrou", backendName="verrou",
                          postBackendList=[{"postBackendName":"checkcancellation","boolCheck":"checkCancellation","convFloat":True ,"ccOnly":True },
                                           {"postBackendName":"check_float_max"  ,"boolCheck":"vr.checkFloatMax", "convFloat":False,"ccOnly":False}
                                           ])

    generateVerrouGeneric(handler, backendEnum="vr_checkdenorm", backendName="checkdenorm",
                          postBackendList= [{"postBackendName":"","boolCheck":None, "convFloat":True ,"ccOnly":False },
                                            {"postBackendName":"checkcancellation","boolCheck":"checkCancellation","convFloat":True ,"ccOnly":True }
                           ])

    handler.write("#ifdef USE_VERROU_QUADMATH\n")
    handler.write("#define IGNORESQRT\n")

    generateVerrouGeneric(handler, backendEnum="vr_mcaquad", backendName="mcaquad",
                          postBackendList= [ {"postBackendName":"","boolCheck":None, "convFloat":True ,"ccOnly":False },
                                             {"postBackendName":"checkcancellation","boolCheck":"checkCancellation","convFloat":True ,"ccOnly":True }
                           ])


    handler.write("#undef IGNORESQRT\n")
    handler.write("#else //USE_VERROU_QUADMATH\n")
    handler.write("""
    if(vr.backend==vr_mcaquad){
            VG_(tool_panic) ( "Verrou compiled without quad support...  \\n");
    }
""")
    handler.write("#endif //USE_VERROU_QUADMATH\n")

