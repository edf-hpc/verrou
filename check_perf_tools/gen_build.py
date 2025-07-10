#!/usr/bin/python3

import os
import re
import sys
import subprocess
from pathlib import Path

workDirectory=Path().cwd() /".."/".."/"perfWorkDir"

gitRepository="origin/"
gitRepository=""

branch="master"
valgrind_version="valgrind-3.25.0"

verrouConfigListHash={
    #good idea to keep the same definition of "current" and "current_fast" (see bellow)
    "current":          { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":""},
    "current_fast":     { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--enable-verrou-check-naninf=no --with-verrou-denorm-hack=none"},
    "dietzfelbinger":   { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--with-verrou-det-hash=dietzfelbinger --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "multiply_shift":   { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--with-verrou-det-hash=multiply_shift --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "double_tabulation":{ "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--with-verrou-det-hash=double_tabulation --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "tabulation":       { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--with-verrou-det-hash=tabulation --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "mersenne_twister": { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--with-verrou-det-hash=mersenne_twister --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
    "xxhash":           { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--with-verrou-det-hash=xxhash --enable-verrou-check-naninf=no  --with-verrou-denorm-hack=none"},
}

verrouConfigListCmpToStable={
    #good idea to keep the same definition of "current" and "current_fast" (see above)
    "stable":       { "tag":"v2.5.0" ,"flags":"--enable-verrou-fma"},
    "current":      { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":""},
    "current_fast": { "valgrind":valgrind_version, "branch_verrou":branch ,"flags":"--enable-verrou-check-naninf=no --with-verrou-denorm-hack=none"},
}

verrouConfigListCmpBranch={
#    "stable":           { "tag":"v2.5.0" ,"flags":"--enable-verrou-fma" },
    "master_fast":{ "valgrind":valgrind_version, "branch_verrou":"master" ,"flags":"--enable-verrou-check-naninf=no --with-verrou-denorm-hack=none"},
    "master":     { "valgrind":valgrind_version, "branch_verrou":"master" ,   "flags":""},
    "back":       { "valgrind":valgrind_version, "branch_verrou":"bl/backtrace", "flags":""},
    "back_fast":  { "valgrind":valgrind_version, "branch_verrou":"bl/backtrace", "flags":"--enable-verrou-check-naninf=no --with-verrou-denorm-hack=none"},
}


verrouConfigListInst={
    "master":     { "valgrind":valgrind_version, "branch_verrou":"master" ,   "flags":""}
}


valgrindConfigList={
    "valgrind-3.17.0": {"file": "valgrind-3.17.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.17.0.tar.bz2"},
    "valgrind-3.19.0": {"file": "valgrind-3.19.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.19.0.tar.bz2"},
    "valgrind-3.20.0": {"file": "valgrind-3.20.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.20.0.tar.bz2"},
    "valgrind-3.21.0": {"file": "valgrind-3.21.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.21.0.tar.bz2"},
    "valgrind-3.22.0": {"file": "valgrind-3.22.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.22.0.tar.bz2"},
    "valgrind-3.23.0": {"file": "valgrind-3.23.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.23.0.tar.bz2"},
    "valgrind-3.24.0": {"file": "valgrind-3.24.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.24.0.tar.bz2"},
    "valgrind-3.25.0": {"file": "valgrind-3.25.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.25.0.tar.bz2"},
    "v2.3.1":          {"file": "v2.3.1.tar.gz","url":"https://github.com/edf-hpc/verrou/releases/download/v2.3.1/valgrind-3.17.0_verrou-2.3.1.tar.gz"},
    "v2.4.0":          {"file": "v2.4.0.tar.gz","url":"https://github.com/edf-hpc/verrou/releases/download/v2.4.0/valgrind-3.20.0_verrou-2.4.0.tar.gz"},
    "v2.5.0":          {"file": "v2.5.0.tar.gz","url":"https://github.com/edf-hpc/verrou/releases/download/v2.5.0/valgrind-3.21.0_verrou-2.5.0.tar.gz"},
}



def runCmd(cmd):
    subprocess.call(cmd, shell=True)

def buildConfig(name):
    workDirectory.mkdir(exist_ok=True)

    buildRep= workDirectory / ("buildRep-"+name)
    if name=="local":
        buildRep.mkdir(exist_ok=True)
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

    valgrindArchive=workDirectory / valgrindConfigList[valgrindKey]["file"]
    if not valgrindArchive.is_file():
        valgrindUrl=valgrindConfigList[valgrindKey]["url"]
        runCmd("wget --output-document=%s %s"%(valgrindArchive,valgrindUrl))

    if not buildRep.is_dir():
        if "valgrind" in verrouConfigParam:
            branch=verrouConfigParam["branch_verrou"]
            if "gitRepository" in verrouConfigParam:
                branch=verrouConfigParam["gitRepository"]+branch
            else:
                branch=gitRepository+branch
            runCmd("./buildConfig.sh %s %s %s \"%s\""%(
                buildRep.relative_to(Path.cwd()),
                valgrindArchive,
                branch,
                verrouConfigParam["flags"])
            )
        if "tag" in verrouConfigParam:
            runCmd("./buildTag.sh %s %s \"%s\""%(
                buildRep,
                valgrindArchive,
                verrouConfigParam["flags"])
            )


if __name__=="__main__":

    verrouConfigList=verrouConfigListCmpBranch

    if len(sys.argv)==2:
        if sys.argv[1]=="cmpBranch":
            verrouConfigList=verrouConfigListCmpBranch
        elif sys.argv[1]=="cmpHash":
            verrouConfigList=verrouConfigListHash
        elif sys.argv[1]=="cmpStable":
            verrouConfigList=verrouConfigListCmpToStable
        elif sys.argv[1]=="cmpInst":
            verrouConfigList=verrouConfigListInst
        else:
            print("invalid cmd")
            sys.exit(42)
    if not len(sys.argv) in [1,2]:
        print("invalid cmd")
        sys.exit(42)

    for name in verrouConfigList:
        buildConfig(name)
