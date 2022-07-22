#!/usr/bin/python3

import os
import re
import sys
import subprocess


detRounding=["random_det","average_det", "random_comdet","average_comdet"]
verrouConfigList={
    "master":     { "valgrind":"valgrind-3.17.0", "branch_verrou":"master"      ,"flags":"--enable-verrou-fma", "special_rounding_tab":[]},
    "dietzfelbinger": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=dietzfelbinger", "special_rounding_tab":detRounding},
    "multiply_shift": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=multiply_shift", "special_rounding_tab":detRounding},
    "double_tabulation": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=double_tabulation", "special_rounding_tab":detRounding},
    "mersenne_twister": { "valgrind":"valgrind-3.17.0", "branch_verrou":"bl/merge_det" ,"flags":"--enable-verrou-fma --with-verrou-det-hash=mersenne_twister", "special_rounding_tab":detRounding},
}

valgrindConfigList={
    "valgrind-3.17.0": {"file": "valgrind-3.17.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.17.0.tar.bz2"},
    "valgrind-3.19.0": {"file": "valgrind-3.19.0.tar.bz2", "url":"https://sourceware.org/pub/valgrind/valgrind-3.19.0.tar.bz2"}
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

    valgrindArchive=valgrindConfigList[verrouConfigParam["valgrind"]]["file"]
    if not os.path.exists(valgrindArchive):
        valgrindUrl=valgrindConfigList[verrouConfigParam["valgrind"]]["url"]
        runCmd("wget --output-document=%s %s"%(valgrindArchive,valgrindUrl))

    if not os.path.exists(buildRep):
        runCmd("./buildConfig.sh %s %s %s \"%s\""%(
            buildRep,
            valgrindConfigList[verrouConfigParam["valgrind"]]["file"],
            verrouConfigParam["branch_verrou"],
            verrouConfigParam["flags"])
        )


if __name__=="__main__":

    for name in verrouConfigList:
        buildConfig(name)
