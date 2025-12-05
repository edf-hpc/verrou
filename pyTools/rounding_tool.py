#!/usr/bin/python3

import sys

roundingDetTabWithoutFloatPrefix=["nearest", "upward", "downward", "toward_zero", "away_zero", "farthest", "float", "ftz", "daz","dazftz","native"]
roundingNonDetTabWithoutFloatPrefix=[x+y for x in ["random", "average", "prandom"] for y in ["","_det","_comdet"]] +[x+y for x in ["random", "average"] for y in ["_scomdet"]]+["sr_monotonic", "sr_smonotonic"]


roundingDetTabWithFloatPrefix=roundingDetTabWithoutFloatPrefix+["float_"+x for x in roundingDetTabWithoutFloatPrefix]
roundingNonDetTabWithFloatPrefix=roundingNonDetTabWithoutFloatPrefix+["float_"+x for x in roundingNonDetTabWithoutFloatPrefix]


def filterNonDetTab(roundingTab):
    validTab=list(filter(isValidRounding, roundingTab ))
    return list(filter(lambda x: not ((x in roundingDetTabWithFloatPrefix) or (x.startswith("vprec"))), validTab ))

def filterDetRoundingTab(roundingTab):
    return list(filter(lambda x: (x in roundingDetTabWithFloatPrefix) or (x.startswith("vprec")) , roundingTab ))

def isValidRounding(rounding):
    return roundingToEnvVar(rounding,failure=False)!=None

def isRoundingCompatibleWithDenorm(rounding):
    return roundingToEnvVar(rounding,failure=False)["VERROU_ROUNDING_MODE"] in ["nearest", "native", "ftz", "daz", "dazftz"]

def isStrFloat(strFloat):
    try:
        value=float(strFloat)
    except:
        return False
    return True

def isValidRef(strRounding):
    if isValidRounding(strRounding):
        return True
    return isStrFloat(strRounding)

def vprecSuggestion():
    aliasTab=["fp16", "bf16" ,"fp24","pxr24","fp32", "tf32"]
    res=["vprec" + mode +"-"+fp + db for mode in ["", "-ob","-ib","-full"]
         for fp in ["E8M23"]+aliasTab
         for db in [""]+["-"+x for x in  (["E11M52"]+aliasTab)]
         ]
    return res

def roundingToEnvVar(roundingCmd, res={}, failure=True):
    rounding=roundingCmd
    res.update({"VERROU_FLOAT":"no"})
    if roundingCmd.startswith("float_"):
        rounding=rounding.replace("float_","")
        res.update({"VERROU_FLOAT":"yes"})

    res.update({"VERROU_UNFUSED":"no"})
    if roundingCmd.startswith("unfused_"):
        rounding=rounding.replace("unfused_","")
        res.update({"VERROU_UNFUSED":"yes"})

    for prand in ["prandom_det", "prandom_comdet", "prandom"]:
        if rounding.startswith(prand):
            if rounding in ["prandom_det", "prandom_comdet", "prandom"]:
                res.update({"VERROU_ROUNDING_MODE":rounding,"VERROU_PRANDOM_UPDATE":"none"})
                return res

            rest=rounding.replace(prand,"")
            if rest.startswith("_"):
                rest=rest[1:]
                if rest=="func":
                    res.update({"VERROU_ROUNDING_MODE":prand, "VERROU_PRANDOM_UPDATE":"func"})
                    return res
                try:
                    value=float(rest)
                    res.update({"VERROU_ROUNDING_MODE":prand, "VERROU_PRANDOM_PVALUE":rest})
                    return res
                except:
                    if failure:
                        print("No valid prandom rounding specification : ", rounding )
                        sys.exit(42)
                    else:
                        return None
            else:
                if failure:
                    print("No valid prandom rounding : ",rounding)
                    sys.exit(42)
                else:
                    return None

    if rounding.startswith("mca"):
        mcaConfig=rounding.split("-")[1:]
        mode=mcaConfig[0]
        doublePrec=mcaConfig[1]
        floatPrec=mcaConfig[2]
        envvars={"VERROU_BACKEND":"mcaquad",
                 "VERROU_MCA_MODE":mode,
                 "VERROU_MCA_PRECISION_DOUBLE": doublePrec,
                 "VERROU_MCA_PRECISION_FLOAT": floatPrec}
        res.update(envvars)
        return res
    if rounding.startswith("vprec-"):
        def vprecParseFp(FP):
            """input E15M20 return 15,20
            accepts aliases: fp16,bf16,fp32,fp64,tf32
            """
            if FP.startswith("E"):
                FPTab=FP[1:].split("M")
                if len(FPTab)!=2:
                    return None
                try:
                    testInt=[int(localStr) for localStr in FPTab]
                except:
                    print("vprec badFP configuration:", FP)
                    return None
                return (testInt[0], testInt[1])
            if FP=="fp16":
                return (5,10)
            if FP=="fp32":
                return (8,23)
            if FP=="bf16":
                return (8,7)
            if FP=="fp64":
                return (11,52)
            if FP=="tf32":
                return (8,10)
            if FP=="fp24":
                return (7,16)
            if FP=="pxr24":
                return (8,15)

            print("vprec bad FP configuration:", FP)
            return None

        def vprecParseMode(param):
            mode="ob"
            if param.startswith("ob-"):
                param=param.replace("ob-","")
            elif param.startswith("ib-"):
                param=param.replace("ib-","")
                mode="ib"
            elif param.startswith("full-"):
                param=param.replace("full-","")
                mode="full"
            return (mode,param)

        try:
            mode,param=vprecParseMode(rounding.replace("vprec-",""))
            vprecConfig=param.split("-")
            if len(vprecConfig)==1:
                floatRange, floatPrec=vprecParseFp(vprecConfig[0])
                doubleRange, doublePrec= (floatRange, floatPrec)
            elif len(vprecConfig) ==2:
                floatRange, floatPrec  = vprecParseFp(vprecConfig[0])
                doubleRange, doublePrec= vprecParseFp(vprecConfig[1])
            else:
                print("vprec bad param:", param)

            if floatRange <1 or doubleRange <1 or floatPrec<1 or doublePrec<1 or floatRange>8 or doubleRange > 11 or floatPrec > 23 or doublePrec > 51:
                print("vprec bad param", param)
                raise("vprec bad config")

        except:
            if failure:
                print("No valid vprec",rounding)
                sys.exit(42)
            else:
                return None

        envvars={"VERROU_BACKEND":"vprec",
                 "VERROU_VPREC_MODE":mode,
                 "VERROU_VPREC_RANGE_BINARY64": str(doubleRange),
                 "VERROU_VPREC_PRECISION_BINARY64": str(doublePrec),
                 "VERROU_VPREC_RANGE_BINARY32": str(floatRange),
                 "VERROU_VPREC_PRECISION_BINARY32": str(floatPrec)
                 }
        res.update(envvars)
        return res

    if rounding in roundingDetTabWithoutFloatPrefix + roundingNonDetTabWithoutFloatPrefix:
        res.update({"VERROU_ROUNDING_MODE":rounding })
        return res
    if failure:
        print("No valid rounding : ",rounding)
        sys.exit(42)
    else:
        return None
