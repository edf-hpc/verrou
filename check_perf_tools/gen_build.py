#!/usr/bin/python3

import os
import re
import sys
import subprocess

gitRepositoty="origin/"
gitRepositoty=""

branch="bl/sqrt"

verrouConfigList={
    "stable":           { "tag":"v2.4.0" ,"flags":"--enable-verrou-fma"},
    "current":           { "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":""},
    "current_fast":      { "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":"--enable-verrou-check-naninf=no --with-verrou-denorm-hack=none"},
    "dietzfelbinger":   { "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":"--with-verrou-det-hash=dietzfelbinger --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "multiply_shift":   { "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":"--with-verrou-det-hash=multiply_shift --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "double_tabulation":{ "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":"--with-verrou-det-hash=double_tabulation --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "mersenne_twister": { "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":"--with-verrou-det-hash=mersenne_twister --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "xxhash": { "valgrind":"valgrind-3.20.0", "branch_verrou":branch ,"flags":"--with-verrou-det-hash=xxhash --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
}

valgrindConfigList={
    "valgrind-3.17.0": {"file": "valgrind-3.17.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.17.0.tar.bz2"},
    "valgrind-3.19.0": {"file": "valgrind-3.19.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.19.0.tar.bz2"},
    "valgrind-3.20.0": {"file": "valgrind-3.20.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.20.0.tar.bz2"},
    "v2.3.1":          {"file": "v2.3.1.tar.gz","url":"https://github.com/edf-hpc/verrou/releases/download/v2.3.1/valgrind-3.17.0_verrou-2.3.1.tar.gz"},
    "v2.4.0":          {"file": "v2.4.0.tar.gz","url":"https://github.com/edf-hpc/verrou/releases/download/v2.4.0/valgrind-3.20.0_verrou-2.4.0.tar.gz"},
}



def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def buildConfig(name):
    buildRep="buildRep-"+name
    if name=="local":
        if not os.path.exists(buildRep):
            os.mkdir(buildRep)
        return
    verrouConfigParam=verrouConfigList[name]

    valgrindKey=None
    if "valgrind" in verrouConfigParam:
        valgrindKey=verrouConfigParam["valgrind"]
    if "tag" in verrouConfigParam:
        valgrindKey=verrouConfigParam["tag"]
    if valgrindKey==None:
        print("Error valgrind key needed")
        sys.exit(42)

    valgrindArchive=valgrindConfigList[valgrindKey]["file"]
    if not os.path.exists(valgrindArchive):
        valgrindUrl=valgrindConfigList[valgrindKey]["url"]
        runCmd("wget --output-document=%s %s"%(valgrindArchive,valgrindUrl))

    if not os.path.exists(buildRep):
        if "valgrind" in verrouConfigParam:
            branch=verrouConfigParam["branch_verrou"]
            if "gitRepositoty" in verrouConfigParam:
                branch=verrouConfigParam["gitRepositoty"]+branch
            else:
                branch=gitRepositoty+branch
            runCmd("./buildConfig.sh %s %s %s \"%s\""%(
                buildRep,
                valgrindConfigList[verrouConfigParam["valgrind"]]["file"],
                branch,
                verrouConfigParam["flags"])
            )
        if "tag" in verrouConfigParam:
            runCmd("./buildTag.sh %s %s \"%s\""%(
                buildRep,
                valgrindConfigList[verrouConfigParam["tag"]]["file"],
                verrouConfigParam["flags"])
            )


if __name__=="__main__":

    for name in verrouConfigList:
        buildConfig(name)
