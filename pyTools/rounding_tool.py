#!/usr/bin/python3

import sys

roundingDetTab=["nearest","upward","downward", "toward_zero","farthest","float","ftz"]
roundingNonDetTab=[x+y for x in ["random", "average", "prandom"] for y in ["","_det","_comdet"]] +[x+y for x in ["random", "average"] for y in ["_scomdet"]]+["sr_monotonic"]

allRoundingTab=roundingDetTab+roundingNonDetTab



def filterNonDetTab(roundingTab):
    validTab=list(filter(isValidRounding, roundingTab ))
    return list(filter(lambda x: not (x in roundingDetTab), validTab ))

def filterDetRoundingTab(roundingTab):
    return list(filter(lambda x: x in roundingDetTab, roundingTab ))

def isValidRounding(rounding):
    return roundingToEnvVar(rounding,failure=False)!=None

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


def roundingToEnvVar(rounding, res={}, failure=True):
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
    if rounding in roundingDetTab + roundingNonDetTab:
        res.update({"VERROU_ROUNDING_MODE":rounding })
        return res
    if failure:
        print("No valid rounding : ",rounding)
        sys.exit(42)
    else:
        return None
