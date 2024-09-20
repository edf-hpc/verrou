#!/usr/bin/python3

import sys

listOfUnaryOp=["sqrt"]
listOfBinaryOp=["add","sub","mul","div"]
listOfTernaryOp=["madd"]

listOfType=["double","float"]

protoTypeUnary="""
IFV_INLINE void IFV_FCTNAME(/OP/_/TYPE/) (/TYPE/ a, /TYPE/* res,void* context) {
  typedef OpWithDynSelectedRoundingMode</OPCLASS/ </TYPE/> > Op;
  Op::apply(Op::PackArgs(a),res,context);
}
"""
protoTypeBinary="""
IFV_INLINE void IFV_FCTNAME(/OP/_/TYPE/) (/TYPE/ a, /TYPE/ b, /TYPE/* res,void* context) {
  typedef OpWithDynSelectedRoundingMode</OPCLASS/ </TYPE/> > Op;
  Op::apply(Op::PackArgs(a,b),res,context);
}
"""
protoTypeTernary="""
IFV_INLINE void IFV_FCTNAME(/OP/_/TYPE/) (/TYPE/ a, /TYPE/ b, /TYPE/ c, /TYPE/* res,void* context) {
  typedef OpWithDynSelectedRoundingMode</OPCLASS/ </TYPE/> > Op;
  Op::apply(Op::PackArgs(a,b,c),res,context);
}
"""
protoTypeCast="""
IFV_INLINE void IFV_FCTNAME(/OP/_double_to_float) (double a, float* res, void* context){
  typedef OpWithDynSelectedRoundingMode</OPCLASS/</TYPE/> > Op;
  Op::apply(Op::PackArgs(a),res,context);
}
"""

post_treatmement_code="""
#ifndef VERROU_IGNORE_NANINF_CHECK
    if (isNanInf(*res)) {
      if(isNan(*res)){
	vr_nanHandler();
      }
      if(isinf(*res)){
	vr_infHandler();
      }
    }
#endif
"""

protoTypeUnaryStatic="""
IFV_INLINE void IFV_FCTNAME(/OP/_/TYPE/_/ROUNDING_NAME/) (/TYPE/ a, /TYPE/* res,void* context) {
  typedef /ROUNDING_CLASS/</OPCLASS/ </TYPE/> /RANDCLASS/> Op;
  *res=Op::apply(Op::PackArgs(a));
  /POST_TREATMEMENT_CODE/;
}
"""
protoTypeBinaryStatic="""
IFV_INLINE void IFV_FCTNAME(/OP/_/TYPE/_/ROUNDING_NAME/) (/TYPE/ a, /TYPE/ b, /TYPE/* res,void* context) {
  typedef /ROUNDING_CLASS/</OPCLASS/ </TYPE/> /RANDCLASS/> Op;
  *res=Op::apply(Op::PackArgs(a,b));
  /POST_TREATMEMENT_CODE/;
}
"""
protoTypeTernaryStatic="""
IFV_INLINE void IFV_FCTNAME(/OP/_/TYPE/_/ROUNDING_NAME/) (/TYPE/ a, /TYPE/ b, /TYPE/ c, /TYPE/* res,void* context) {
  typedef /ROUNDING_CLASS/</OPCLASS/ </TYPE/> /RANDCLASS/> Op;
  *res=Op::apply(Op::PackArgs(a,b,c));
  /POST_TREATMEMENT_CODE/;
}
"""
protoTypeCastStatic="""
IFV_INLINE void IFV_FCTNAME(/OP/_double_to_float_/ROUNDING_NAME/) (double a, float* res, void* context){
  typedef /ROUNDING_CLASS/</OPCLASS/</TYPE/> /RANDCLASS/> Op;
  *res=Op::apply(Op::PackArgs(a));
 /POST_TREATMEMENT_CODE/;
}
"""

header_interflop="""
  void IFV_FCTNAME(add_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(add_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(sub_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(sub_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(mul_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(mul_float)  (float a,  float b,  float*  res, void* context);
  void IFV_FCTNAME(div_double) (double a, double b, double* res, void* context);
  void IFV_FCTNAME(div_float)  (float a,  float b,  float*  res, void* context);

  void IFV_FCTNAME(sqrt_double) (double a, double* res, void* context);
  void IFV_FCTNAME(sqrt_float)  (float a,  float*  res, void* context);

  void IFV_FCTNAME(cast_double_to_float) (double a, float* b, void* context);

  void IFV_FCTNAME(madd_double)(double a, double b, double c, double* res, void* context);
  void IFV_FCTNAME(madd_float) (float a,  float b,  float c,  float*  res, void* context);
"""


def getOpClass(op):
    if op=="add":
        return "AddOp"
    if op=="sub":
        return "SubOp"
    if op=="mul":
        return "MulOp"
    if op=="div":
        return "DivOp"
    if op=="sqrt":
        return "SqrtOp"
    if op=="madd":
        return "MAddOp"
    if op=="cast":
        return "CastOp"
    return None
def getRoundingClass(rounding):
    if rounding=="NEAREST":
        return "RoundingNearest"
    if rounding=="FLOAT":
        return "RoundingFloat"
    if rounding=="UPWARD":
        return "RoundingUpward"
    if rounding=="DOWNWARD":
        return "RoundingDownward"
    if rounding=="ZERO":
        return "RoundingZero"
    if rounding=="AWAY_ZERO":
        return "RoundingAwayZero"
    if rounding=="FARTHEST":
        return "RoundingFarthest"
    if rounding in ["RANDOM"+det for det in ["","_DET","_COMDET"] ]:
        return "RoundingRandom"
    if rounding in ["RANDOM"+det for det in ["_SCOMDET"] ]:
        return "RoundingPRandom"
    if rounding in ["AVERAGE"+det for det in ["_SCOMDET"] ]:
        return "RoundingSAverage"
    if rounding in ["AVERAGE"+det for det in ["","_DET","_COMDET"] ]:
        return "RoundingAverage"
    if rounding in ["PRANDOM"+det for det in ["","_DET","_COMDET"] ]:
        return "RoundingPRandom"
    if rounding in ["SR_MONOTONIC"]:
        return "RoundingSRMonotonic"
    if rounding in ["SR_SMONOTONIC"]:
        return "RoundingSRSMonotonic"
    return None

def getRandClass(rounding):
    if rounding in ["NEAREST","UPWARD","DOWNWARD","ZERO", "AWAY_ZERO", "FARTHEST"]:
        return None
    if rounding in ["RANDOM","AVERAGE"]:
        return "vr_rand_prng</OPCLASS/ </TYPE/> >"
    if rounding in ["RANDOM_DET","AVERAGE_DET","SR_MONOTONIC","SR_SMONOTONIC" ]:
        return "vr_rand_det</OPCLASS/ </TYPE/> >"
    if rounding in ["RANDOM_COMDET","AVERAGE_COMDET"]:
        return "vr_rand_comdet</OPCLASS/ </TYPE/> >"
    if rounding in ["RANDOM_SCOMDET","AVERAGE_SCOMDET"]:
        return "vr_rand_scomdet</OPCLASS/ </TYPE/> >"
    if rounding == "PRANDOM":
        return "vr_rand_p</OPCLASS/ </TYPE/>,vr_rand_prng>"
    if rounding == "PRANDOM_DET":
        return "vr_rand_p</OPCLASS/ </TYPE/>,vr_rand_det>"
    if rounding == "PRANDOM_COMDET":
        return "vr_rand_p</OPCLASS/ </TYPE/>,vr_rand_comdet>"
    return None


def specializePatternDyn(template, op, typeFp):
    res=template.replace("/OP/",op)
    res=res.replace("/TYPE/",typeFp)
    res=res.replace("/OPCLASS/", getOpClass(op))
    return res

def specializePatternStatic(template, op, typeFp, roundingName,roundingClass, randClass):
    if roundingClass==None:
        return "#error \"in code generation\""
    res=template
    if randClass==None:
        res=res.replace("/RANDCLASS/","")
    else:
        res=res.replace("/RANDCLASS/",","+randClass+" ")

    res=res.replace("/ROUNDING_CLASS/", roundingClass)
    res=res.replace("/OP/",op)
    res=res.replace("/TYPE/",typeFp)
    if op!="":
        res=res.replace("/OPCLASS/", getOpClass(op))

    res=res.replace("/ROUNDING_NAME/", roundingName)
    res=res.replace("/POST_TREATMEMENT_CODE/",post_treatmement_code)
    return res


def genFlopImpl(handler, roundingmode="dyn"):
    for op in listOfUnaryOp:
        for fpType in listOfType:
            if roundingmode=="dyn":
                handler.write(specializePatternDyn(protoTypeUnary, op, fpType)+"\n")
            else:
                handler.write(specializePatternStatic(protoTypeUnaryStatic, op, fpType, roundingmode, getRoundingClass(roundingmode), getRandClass(roundingmode))+"\n")


    for op in listOfBinaryOp:
        for fpType in listOfType:
            if roundingmode=="dyn":
                handler.write(specializePatternDyn(protoTypeBinary, op, fpType)+"\n")
            else:
                handler.write(specializePatternStatic(protoTypeBinaryStatic, op, fpType, roundingmode, getRoundingClass(roundingmode), getRandClass(roundingmode))+"\n")

    for op in listOfTernaryOp:
        for fpType in listOfType:
            if roundingmode=="dyn":
                handler.write(specializePatternDyn(protoTypeTernary, op, fpType)+"\n")
            else:
                handler.write(specializePatternStatic(protoTypeTernaryStatic, op, fpType, roundingmode, getRoundingClass(roundingmode),getRandClass(roundingmode))+"\n")

    if roundingmode=="dyn":
        handler.write(specializePatternDyn(protoTypeCast, "cast", "double,float") +"\n")
    else:
        handler.write(specializePatternStatic(protoTypeCastStatic, "cast", "double,float", roundingmode, getRoundingClass(roundingmode), getRandClass(roundingmode))+"\n")


if __name__=="__main__":
    roundingTab =["NEAREST", "UPWARD", "DOWNWARD", "FARTHEST", "ZERO", "AWAY_ZERO", "FLOAT"]
    roundingTab+=[rnd + det for rnd in ["RANDOM", "AVERAGE"] for det in ["","_DET","_COMDET","_SCOMDET" ]]
    roundingTab+=[rnd + det for rnd in ["PRANDOM"] for det in ["","_DET","_COMDET" ]]
    roundingTab+=["SR_MONOTONIC","SR_SMONOTONIC"]

    handler=open("interflop_verrou_flop_impl.hxx","w")
    handler.write("// generated by : "+str(sys.argv)+"\n")
    handler.write("#define IFV_INLINE\n")
    genFlopImpl(handler, roundingmode="dyn")
    for rounding in roundingTab:
        genFlopImpl(handler, roundingmode=rounding)
    handler.close()


    handler=open("interflop_verrou_rounding.h","w")

    handler.write("#undef IFV_FCTNAME\n")
    for rounding in roundingTab:
        handler.write("#define IFV_FCTNAME(FCT) interflop_verrou_##FCT##_"+rounding+"\n")
        handler.write(header_interflop)
        handler.write("#undef IFV_FCTNAME\n")

    handler.write("#define IFV_FCTNAME(FCT) interflop_verrou_##FCT\n")

