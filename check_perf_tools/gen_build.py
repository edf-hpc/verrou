#!/usr/bin/python3

import os
import re
import sys
import subprocess

gitRepositoty="origin/"

verrouConfigList={
    "stable":           { "tag":"v2.3.1" ,"flags":"--enable-verrou-fma"},
    "master":           { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma"},
    "master_fast":      { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma --enable-verrou-check-naninf=no"},
    "dietzfelbinger":   { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=dietzfelbinger --enable-verrou-check-naninf=no"},
    "multiply_shift":   { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=multiply_shift --enable-verrou-check-naninf=no"},
    "multiply_shift_fix":   { "valgrind":"valgrind-3.19.0", "branch_verrou":"bl/multiply-shift-fix" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=multiply_shift --enable-verrou-check-naninf=no"},
    "double_tabulation":{ "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=double_tabulation --enable-verrou-check-naninf=no"},
    "mersenne_twister": { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=mersenne_twister --enable-verrou-check-naninf=no"},
}

verrouConfigList={
    "stable":           { "tag":"v2.3.1" ,"flags":"--enable-verrou-fma"},
    "master":           { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma"},
    "master_fast":      { "valgrind":"valgrind-3.19.0", "branch_verrou":"master" ,"flags":"--enable-verrou-fma --enable-verrou-check-naninf=no"},
    "code_gen":           { "valgrind":"valgrind-3.19.0", "branch_verrou":"bl/perf_codegen" ,"flags":"--enable-verrou-fma"},
    "code_gen_fast":      { "valgrind":"valgrind-3.19.0", "branch_verrou":"bl/perf_codegen" ,"flags":"--enable-verrou-fma --enable-verrou-check-naninf=no"},
    "code_gen_merge":           { "valgrind":"valgrind-3.19.0", "branch_verrou":"perf_codegen_merge" ,"flags":"--enable-verrou-fma"},
    "code_gen_fast_merge":      { "valgrind":"valgrind-3.19.0", "branch_verrou":"perf_codegen_merge" ,"flags":"--enable-verrou-fma --enable-verrou-check-naninf=no"},
    "test_perf":      { "valgrind":"valgrind-3.19.0", "branch_verrou":"perf_codegen_merge" ,"gitRepositoty":"", "flags":"--enable-verrou-fma --enable-verrou-check-naninf=no"},
}


valgrindConfigList={
    "valgrind-3.17.0": {"file": "valgrind-3.17.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.17.0.tar.bz2"},
    "valgrind-3.19.0": {"file": "valgrind-3.19.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.19.0.tar.bz2"},
    "v2.3.1":          {"file": "v2.3.1.tar.gz","url":"https://github.com/edf-hpc/verrou/releases/download/v2.3.1/valgrind-3.17.0_verrou-2.3.1.tar.gz"}
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
