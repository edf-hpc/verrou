import sys


def isIntegerEqualWithTol(value, ref, tol ):
    if value >= ref+tol[0] and value <= ref+tol[1]:
        return True
    return False

def countForEstimator(statusTab, counterTab, refIndex, tol=[0,0]):
    assert(statusTab[refIndex]==True)
    assert(len(statusTab) == len(counterTab))
    nbSuccess=0
    nbFail=0
    nbFailDiff, nbFailEqual, nbSuccessDiff, nbSuccessEqual=(0,0,0,0)

    for i in range(len(statusTab)):
        assert(statusTab[i] in [True,False])
        if statusTab[i]==True:
            nbSuccess+=1
            if  isIntegerEqualWithTol(counterTab[i], counterTab[refIndex], tol):
                nbSuccessEqual+=1
            else:
                nbSuccessDiff+=1
        else:
            nbFail+=1
            if  isIntegerEqualWithTol(counterTab[i], counterTab[refIndex], tol):
                nbFailEqual+=1
            else:
                nbFailDiff+=1
    dicRes= {"nbSuccess": nbSuccess, "nbFail":nbFail,
            "nbFailDiff":nbFailDiff,
            "nbFailEqual":nbFailEqual,
            "nbSuccessDiff":nbSuccessDiff,
            "nbSuccessEqual":nbSuccessEqual}
    return dicRes


def computeEstimator(statusTab, counterTab, estimatorTab, refIndex=0):
    assert(statusTab[refIndex]==True)
    assert(len(statusTab) == len(counterTab))

    countData=countForEstimator(statusTab, counterTab, refIndex)
    nbSuccess=countData["nbSuccess"]
    nbFail=countData["nbFail"]
    nbFailDiff=countData["nbFailDiff"]
    nbFailEqual=countData["nbFailEqual"]
    nbSuccessDiff=countData["nbSuccessDiff"]
    nbSuccessEqual=countData["nbSuccessEqual"]

    res=[]
    for estimator in estimatorTab:
        if estimator=="dc":
            if (nbFail+nbSuccess)==0:
                res+=[float("Nan")]
            else:
                res+=[float(nbFailDiff + nbSuccessEqual)/ float(nbSuccess+nbFail)]
        elif estimator=="wdc":
            if nbFail==0 or nbSuccess==0:
                res+=[float("Nan")]
            else:
                res+=[0.5* (float(nbFailDiff)/ float(nbFail) + float(nbSuccessEqual)/float(nbSuccess))]
        elif "-stol" in estimator:
            tolTab=[counterTab[i] -counterTab[refIndex] for i in range(len(statusTab)) if statusTab[i]]
            tolMin, tolMax=min(tolTab), max(tolTab)

            countDataTol=countForEstimator(statusTab, counterTab, refIndex, tol=[tolMin, tolMax])
            nbFailDiffTol=countDataTol["nbFailDiff"]
            #nbFailEqualTol=countDataTol["nbFailEqual"]
            nbSuccessDiffTol=countDataTol["nbSuccessDiff"]
            if nbSuccessDiffTol!=0:
                print("Bug nbSuccessDiff")
                print ("counterTab", counterTab)
                print ("countDataTol", countDataTol)
                sys.exit(42)

            nbSuccessEqualTol=countDataTol["nbSuccessEqual"]
            if estimator=="wdc-stol":
                if nbFail==0 or nbSuccess==0:
                    res+=[float("Nan")]
                else:
                    res+=[0.5* (float(nbFailDiffTol)/ float(nbFail) + float(nbSuccessEqualTol)/float(nbSuccess))]
            elif estimator=="fdr-stol": #fail diff ratio
                if nbFail==0 or nbSuccess==0:
                    res+=[float("Nan")]
                else:
                    res+=[(float(nbFailDiffTol)/ float(nbFail))]
            else:
                print("unknown estimator", estimator)
                sys.exit(42)

        else:
            print("unknown estimator", estimator)
    return res

