#!/usr/bin/python3

listOfOpInstrumented=[
    {"Iop":"Iop_AddF64", "op":"VR_OP_ADD", "prec":"VR_PREC_DBL", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_SubF64", "op":"VR_OP_SUB", "prec":"VR_PREC_DBL", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_AddF32", "op":"VR_OP_ADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_SubF32", "op":"VR_OP_SUB", "prec":"VR_PREC_FLT", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_MulF64", "op":"VR_OP_MUL", "prec":"VR_PREC_DBL", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_DivF64", "op":"VR_OP_DIV", "prec":"VR_PREC_DBL", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_MulF32", "op":"VR_OP_MUL", "prec":"VR_PREC_FLT", "vec":"VR_VEC_SCAL" },
    {"Iop":"Iop_DivF32", "op":"VR_OP_DIV", "prec":"VR_PREC_FLT", "vec":"VR_VEC_SCAL" },

    {"Iop":"Iop_Add64F0x2", "op":"VR_OP_ADD", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Sub64F0x2", "op":"VR_OP_SUB", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Add32F0x4", "op":"VR_OP_ADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Sub32F0x4", "op":"VR_OP_SUB", "prec":"VR_PREC_FLT", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Mul64F0x2", "op":"VR_OP_MUL", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Div64F0x2", "op":"VR_OP_DIV", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Mul32F0x4", "op":"VR_OP_MUL", "prec":"VR_PREC_FLT", "vec":"VR_VEC_LLO" },
    {"Iop":"Iop_Div32F0x4", "op":"VR_OP_DIV", "prec":"VR_PREC_FLT", "vec":"VR_VEC_LLO" },

    {"Iop":"Iop_Add64Fx2", "op":"VR_OP_ADD", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2" },
    {"Iop":"Iop_Sub64Fx2", "op":"VR_OP_SUB", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2" },
    {"Iop":"Iop_Add64Fx4", "op":"VR_OP_ADD", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Sub64Fx4", "op":"VR_OP_SUB", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Add32Fx4", "op":"VR_OP_ADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Sub32Fx4", "op":"VR_OP_SUB", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Add32Fx8", "op":"VR_OP_ADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL8" },
    {"Iop":"Iop_Sub32Fx8", "op":"VR_OP_SUB", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL8" },

    {"Iop":"Iop_Mul64Fx2", "op":"VR_OP_MUL", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2" },
    {"Iop":"Iop_Div64Fx2", "op":"VR_OP_DIV", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2" },
    {"Iop":"Iop_Mul64Fx4", "op":"VR_OP_MUL", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Div64Fx4", "op":"VR_OP_DIV", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Mul32Fx4", "op":"VR_OP_MUL", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Div32Fx4", "op":"VR_OP_DIV", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4" },
    {"Iop":"Iop_Mul32Fx8", "op":"VR_OP_MUL", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL8" },
    {"Iop":"Iop_Div32Fx8", "op":"VR_OP_DIV", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL8" },

    {"Iop":"Iop_MAddF32", "op":"VR_OP_MADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_UNK" },
    {"Iop":"Iop_MAddF64", "op":"VR_OP_MADD", "prec":"VR_PREC_DBL", "vec":"VR_VEC_UNK" },
    {"Iop":"Iop_MSubF32", "op":"VR_OP_MSUB", "prec":"VR_PREC_FLT", "vec":"VR_VEC_UNK" },
    {"Iop":"Iop_MSubF64", "op":"VR_OP_MSUB", "prec":"VR_PREC_DBL", "vec":"VR_VEC_UNK" },

    {"Iop":  "Iop_F64toF32", "op":"VR_OP_CONV", "prec": "VR_PREC_DBL_TO_FLT", "vec":"VR_VEC_UNK"},

    {"Iop":"Iop_SqrtF64", "op":"VR_OP_SQRT", "prec":"VR_PREC_DBL", "vec": "VR_VEC_SCAL"},
    {"Iop":"Iop_Sqrt64F0x2", "op":"VR_OP_SQRT", "prec":"VR_PREC_DBL", "vec": "VR_VEC_LLO"},
    {"Iop":"Iop_Sqrt64Fx2", "op":"VR_OP_SQRT", "prec":"VR_PREC_DBL", "vec": "VR_VEC_FULL2"},
    {"Iop":"Iop_Sqrt64Fx4", "op":"VR_OP_SQRT", "prec":"VR_PREC_DBL", "vec": "VR_VEC_FULL4"},

    {"Iop":"Iop_SqrtF32", "op":"VR_OP_SQRT", "prec":"VR_PREC_FLT", "vec": "VR_VEC_SCAL"},
    {"Iop":"Iop_Sqrt32F0x4", "op":"VR_OP_SQRT", "prec":"VR_PREC_FLT", "vec": "VR_VEC_LLO"},
    {"Iop":"Iop_Sqrt32Fx4", "op":"VR_OP_SQRT", "prec":"VR_PREC_FLT", "vec": "VR_VEC_FULL4"},
    {"Iop":"Iop_Sqrt32Fx8", "op":"VR_OP_SQRT", "prec":"VR_PREC_FLT", "vec": "VR_VEC_FULL8"}
]

listOfOpCounted=[
    {"Iop":  "Iop_F64toI64S", "op":"VR_OP_CONV",  "prec": "VR_PREC_DBL_TO_INT", "vec":"VR_VEC_SCAL"},
    {"Iop":  "Iop_F64toI64U", "op":"VR_OP_CONV",  "prec": "VR_PREC_DBL_TO_INT", "vec":"VR_VEC_SCAL"},
    {"Iop":  "Iop_F64toI32S", "op":"VR_OP_CONV",  "prec": "VR_PREC_DBL_TO_SHT", "vec":"VR_VEC_SCAL"},
    {"Iop":  "Iop_F64toI32U", "op":"VR_OP_CONV",  "prec": "VR_PREC_DBL_TO_SHT", "vec":"VR_VEC_SCAL"},

    {"Iop":"Iop_Max32Fx4", "op":"VR_OP_MAX", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_Max32F0x4","op":"VR_OP_MAX", "prec":"VR_PREC_FLT", "vec":"VR_VEC_LLO"}, 
    {"Iop":"Iop_Max64Fx2", "op":"VR_OP_MAX", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2"},
    {"Iop":"Iop_Max64F0x2", "op":"VR_OP_MAX", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO"}, 

    {"Iop":"Iop_Min32Fx4", "op":"VR_OP_MIN", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_Min32F0x4","op":"VR_OP_MIN", "prec":"VR_PREC_FLT", "vec":"VR_VEC_LLO"}, 
    {"Iop":"Iop_Min64Fx2", "op":"VR_OP_MIN", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2"},
    {"Iop":"Iop_Min64F0x2","op":"VR_OP_MIN", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO"}, 

    {"Iop":"Iop_Add32Fx2", "op":"VR_OP_ADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL2"},
    {"Iop":"Iop_Sub32Fx2", "op":"VR_OP_ADD", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL2"},

    {"Iop": "Iop_CmpF64" , "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec": "VR_VEC_SCAL"},
    {"Iop": "Iop_CmpF32" , "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec": "VR_VEC_SCAL"},

    {"Iop":  "Iop_F32toF64", "op":"VR_OP_CONV", "prec": "VR_PREC_FLT_TO_DBL", "vec":"VR_VEC_UNK"},
    {"Iop":"Iop_CmpEQ64Fx2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2"},
    {"Iop":"Iop_CmpLT64Fx2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2"},
    {"Iop":"Iop_CmpLE64Fx2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2"},
    {"Iop":"Iop_CmpUN64Fx2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_FULL2"},

    {"Iop":"Iop_CmpEQ64F0x2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO"}, 
    {"Iop":"Iop_CmpLT64F0x2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO"}, 
    {"Iop":"Iop_CmpLE64F0x2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO"}, 
    {"Iop":"Iop_CmpUN64F0x2",  "op":"VR_OP_CMP", "prec":"VR_PREC_DBL", "vec":"VR_VEC_LLO"}, 

    {"Iop":"Iop_CmpEQ32Fx4",  "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_CmpLT32Fx4",  "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_CmpLE32Fx4",  "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_CmpUN32Fx4",  "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_CmpGT32Fx4",  "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"},
    {"Iop":"Iop_CmpGE32Fx4",  "op":"VR_OP_CMP", "prec":"VR_PREC_FLT", "vec":"VR_VEC_FULL4"} 
]




norCountNorInstrumented=[
    "Iop_ReinterpF64asI64","Iop_ReinterpI64asF64",
    "Iop_ReinterpF32asI32","Iop_ReinterpI32asF32",
    "Iop_NegF64", "Iop_NegF32",
    "Iop_AbsF64", "Iop_AbsF32",
    "Iop_Abs64Fx2","Iop_Neg64Fx2"
]


shouldBeInstrumented=[
    "Iop_SqrtF128","Iop_Sqrt16Fx8",
    "Iop_AddF64r32","Iop_SubF64r32", "Iop_MulF64r32","Iop_DivF64r32",
    "Iop_MAddF64r32","Iop_MSubF64r32",
    "Iop_AddF128","Iop_SubF128","Iop_MulF128","Iop_DivF128",
    "Iop_AtanF64","Iop_Yl2xF64", "Iop_Yl2xp1F64",
    "Iop_PRemF64","Iop_PRemC3210F64",
    "Iop_PRem1F64","Iop_PRem1C3210F64",
    "Iop_ScaleF64",
    "Iop_SinF64","Iop_CosF64","Iop_TanF64",
    "Iop_2xm1F64",
    "Iop_RSqrtEst5GoodF64",
    "Iop_Log2_64Fx2","Iop_Scale2_64Fx2","Iop_RecipEst64Fx2","Iop_RecipStep64Fx2","Iop_RSqrtEst64Fx2","Iop_RSqrtStep64Fx2",
    "Iop_RecipStep32Fx4","Iop_RSqrtEst32Fx4","Iop_RSqrtStep32Fx4","Iop_RecipEst32F0x4","Iop_RSqrtEst32F0x4",
    "Iop_Scale2_32Fx4","Iop_Log2_32Fx4","Iop_Exp2_32Fx4",
    "Iop_RSqrtEst32Fx8","Iop_RecipEst32Fx8",
    "Iop_RoundF64toF64_NEAREST","Iop_RoundF64toF64_NegINF","Iop_RoundF64toF64_PosINF", "Iop_RoundF64toF64_ZERO",  
    "Iop_F32toF16x4", "Iop_F16toF32x4","Iop_F16toF64x2",
    "Iop_F128toF64", "Iop_F128toF32", "Iop_F64toI16S",
    "Iop_CmpF128",
    "Iop_PwMax32Fx4", "Iop_PwMin32Fx4"
]

def instrumentReplaceFunction(iopDic):
    print("instrumentReplaceFunction ",iopDic)
    def defaultInstrumentReplaceFunctionForBinary(iopDic):
        if iopDic["vec"] in "VR_VEC_SCAL":
            return "vr_replaceBinFpOpScal"
        if iopDic["vec"]=="VR_VEC_LLO":
            return "vr_replaceBinFpOpLLO"
        if iopDic["vec"]=="VR_VEC_FULL2" and iopDic["prec"]=="VR_PREC_DBL":
            return "vr_replaceBinFullSSE"
        if iopDic["vec"]=="VR_VEC_FULL4" and iopDic["prec"]=="VR_PREC_FLT":
            return "vr_replaceBinFullSSE"
        if iopDic["vec"]=="VR_VEC_FULL4" and iopDic["prec"]=="VR_PREC_DBL":
            return "vr_replaceBinFullAVX"
        if iopDic["vec"]=="VR_VEC_FULL8" and iopDic["prec"]=="VR_PREC_FLT":
            return "vr_replaceBinFullAVX"

    if iopDic["op"] in ["VR_OP_ADD", "VR_OP_SUB","VR_OP_MUL","VR_OP_DIV" ]:
        return defaultInstrumentReplaceFunctionForBinary(iopDic)
    if iopDic["op"] in ["VR_OP_SQRT"]:
        return defaultInstrumentReplaceFunctionForBinary(iopDic)+"_unary"
    if iopDic["op"] in ["VR_OP_MADD","VR_OP_MSUB"]:
        return "vr_replaceFMA"
    if iopDic["op"] == "VR_OP_CONV" and iopDic["prec"]=="VR_PREC_DBL_TO_FLT":
        return "vr_replaceCast"

def isCC(iopDic):
    if iopDic["op"] in ["VR_OP_ADD", "VR_OP_SUB","VR_OP_MADD","VR_OP_MSUB" ]:
        return True
    return False
def isFused(iopDic):
    if iopDic["op"] in ["VR_OP_MADD","VR_OP_MSUB"]:
        return True
    return False
def isConv(iopDic): #hypothesis iopDic come from instrumentation list
    if iopDic["prec"] in ["VR_PREC_DBL"]:
        return True
    return False

def wrapperFunction(iopDic):
    print(iopDic)
    if iopDic["op"] == "VR_OP_CONV" and iopDic["prec"]=="VR_PREC_DBL_TO_FLT":
        return "bcName(cast64FTo32F)"

    minOp=(iopDic["op"].replace("VR_OP_","")).lower()
    nbBit=None
    if iopDic["prec"]=="VR_PREC_DBL": nbBit="64"
    if iopDic["prec"]=="VR_PREC_FLT": nbBit="32"
    vec=None
    if iopDic["vec"] in ["VR_VEC_SCAL", "VR_VEC_UNK"]: vec="F"
    if iopDic["vec"]=="VR_VEC_LLO": vec="FLLO"
    if iopDic["vec"]=="VR_VEC_FULL2": vec="Fx2"
    if iopDic["vec"]=="VR_VEC_FULL4": vec="Fx4"
    if iopDic["vec"]=="VR_VEC_FULL8": vec="Fx8"
    if iopDic["vec"]=="VR_VEC_FULL8": vec="Fx8"

    argWrap=minOp+nbBit+vec

    funWrap="bcName"
    if isConv(iopDic):
        funWrap +="Conv"
    if isCC(iopDic):
        funWrap += "WithCC"
    return funWrap+"("+ argWrap + ")"


def loopOverIop(handler, prefix,  postFix, default,
                transformLambdaInstrumented,
                transformLambdaCounted,
                applyNorCountNorInstrumented,
                applyShouldBeInstrumented):
    handler.write(prefix);
    handler.write(" "*4 + "switch(op){\n")
    for iopDic in listOfOpInstrumented:
        handler.write(" "*4+"case "+iopDic["Iop"]+":\n")
        handler.write(transformLambdaInstrumented(iopDic))
        handler.write(" "*6+"break;\n")
        pass
    for iopDic in listOfOpCounted:
        handler.write(" "*4+"case "+iopDic["Iop"]+":\n")
        handler.write(transformLambdaCounted(iopDic))
        handler.write(" "*6+"break;\n")

    for iop in norCountNorInstrumented:
        handler.write(" "*4+"case "+iop+":\n")
    handler.write(applyNorCountNorInstrumented)
    handler.write(" "*6+"break;\n")

    for iop in shouldBeInstrumented:
        handler.write(" "*4+"case "+iop+":\n")
    handler.write(applyShouldBeInstrumented)
    handler.write(" "*6+"break;\n")

    handler.write(" "*4+ "default:\n")
    handler.write(default)
    handler.write(" "*6 + "break;\n")
    handler.write(" "*4 + "}\n")
    handler.write(postFix)


prefixCode="""
    Vr_instr_kind res;
    res.containFloatCmp=False;
    res.containFloatModOp=False;
"""
postFixCode="""
    return res;
"""

shouldBeCode="""      if(!countOnly){
          vr_maybe_record_ErrorOp (VR_ERROR_UNCOUNTED, op);
      }
      addStmtToIRSB (sb, stmt);
"""
norCountNorInstrumentedCode="""      addStmtToIRSB (sb, stmt);
"""

defaultCode="""      if(operation_with_float_args(expr) && !countOnly){
         vr_maybe_record_ErrorOp (VR_ERROR_UNKNOWN_FLT_IOP, op);
      };
      addStmtToIRSB (sb, stmt);
"""
countPattern="""      vr_countOp (sb, %%OP%%, %%PREC%%, %%VEC%%, False);
      addStmtToIRSB (sb, stmt);
"""
def transformCount(iopDic):
    res=countPattern
    if iopDic["op"]=="VR_OP_CMP":
        res+=" "*6+"res.containFloatCmp=True;\n"

    res=res.replace("%%OP%%", iopDic["op"])
    res=res.replace("%%PREC%%", iopDic["prec"])
    res=res.replace("%%VEC%%", iopDic["vec"])

    return res


instPattern="""          res.containFloatModOp= %%REPLACE_INST%% (sb, stmt, expr, %%WRAPFUNCNAME%%, %%OP%%, %%PREC%%, %%VEC%%, countOnly);
"""

def transformInstPattern(iopDic, wrapFunction=None):
    res=instPattern
    res=res.replace("%%OP%%", iopDic["op"])
    res=res.replace("%%PREC%%", iopDic["prec"])
    res=res.replace("%%VEC%%", iopDic["vec"])
    res=res.replace("%%REPLACE_INST%%", instrumentReplaceFunction(iopDic))
    if wrapFunction==None:
        res=res.replace("%%WRAPFUNCNAME%%", wrapperFunction(iopDic))
    else:
        res=res.replace("%%WRAPFUNCNAME%%", wrapFunction)

    return res

def transformInst(iopDic):
    if iopDic["op"] in ["VR_OP_MADD","VR_OP_MSUB"]:
        wrapperName=wrapperFunction(iopDic)
        wrapperNameUnfused=wrapperName.replace("(","Unfused(")
        resFused=transformInstPattern(iopDic,wrapperName)
        resUnfused=transformInstPattern(iopDic,wrapperNameUnfused)
        res="#ifndef IGNOREFMA\n"
        res+=" "*6+"if(vr.unfused){\n"
        res+=resUnfused
        res+=" "*6+"}else{\n"
        res+=resFused
        res+=" "*6+"}\n"
        res+="#else //IGNOREFMA\n"
        res+=transformCount(iopDic)
        res+="#endif //IGNOREFMA\n"
        return res

    res=transformInstPattern(iopDic)
    if iopDic["op"]=="VR_OP_SQRT":
        res="#ifndef IGNORESQRT\n"+res
        res+="#else //IGNORESQRT\n"
        res+=transformCount(iopDic)
        res+="#endif //IGNORESQRT\n"

    return res


if __name__=="__main__":
    handler=open("vr_instrumentOp_impl.h","w")

    loopOverIop(handler, prefixCode, postFixCode, defaultCode,
                transformInst,
                transformCount,
                norCountNorInstrumentedCode,
                shouldBeCode
                )

    handler.close()
