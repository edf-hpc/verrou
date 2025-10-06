#!/usr/bin/python3
import sys, platform
import ctypes, ctypes.util
import os
from pathlib import Path


def searchDefaultPath(fileName):
    #dirName=os.path.dirname(os.path.abspath(__file__))
    dirName=(Path(__file__).absolute()).parent
    pathPrefixTab=[Path.cwd(),
                   dirName,
                   dirName / ".." / "lib",
                   dirName / ".." / ".."/".."/"valgrind",
                   dirName / ".." / ".."/".."/".."/"libexec"/"valgrind",
                   Path.cwd()/ ".."
                   ]

    print(pathPrefixTab, file=sys.stderr)
    for pathPrefix in pathPrefixTab:
        absPath=pathPrefix / fileName
        if absPath.is_file():
#            print("absPath: ", absPath,file=sys.stderr)
            return absPath
    print("FileName %s not found"%(fileName),file=sys.stderr)
    sys.exit(42)

class bindingTaskLib:
    def __init__(self, pathLib=None):
        if(pathLib!=None):
            self.lib=ctypes.CDLL(pathLib)
        else:
            self.lib=ctypes.CDLL(searchDefaultPath("libverrouTask.so"), ctypes.RTLD_GLOBAL )
        self.lib.verrou_task.argtypes = [ ctypes.c_char_p, ctypes.c_int]
        self.lib.verrou_task_init()
    def __del__(self):
        self.lib.verrou_task_finalyze()

bindTask=bindingTaskLib()

def task(string, index):
    bindTask.lib.verrou_task(string.encode('utf-8'), index)

class bindingVerrouCLib:
    def __init__(self, pathLib=None):
        if(pathLib!=None):
            self.lib=ctypes.CDLL(pathLib)
        else:
            self.lib=ctypes.CDLL(searchDefaultPath("verrouCBinding.so") )

        self.lib.c_verrou_start_instrumentation.argtypes = []
        self.lib.c_verrou_stop_instrumentation.argtypes = []
        self.lib.c_verrou_start_soft_instrumentation.argtypes = []
        self.lib.c_verrou_stop_soft_instrumentation.argtypes = []
        self.lib.c_verrou_start_determinitic.argtypes = [ctypes.c_int]
        self.lib.c_verrou_stop_determinitic.argtypes  = [ctypes.c_int]

        self.lib.c_verrou_display_counters.argtypes = []

        self.lib.c_verrou_dump_cover.argtypes= []
        self.lib.c_verrou_dump_cover.restype=  ctypes.c_uint

        self.lib.c_verrou_count_fp_instrumented.argtypes= []
        self.lib.c_verrou_count_fp_not_instrumented.argtypes= []
        self.lib.c_verrou_count_fp_instrumented.restype=  ctypes.c_uint
        self.lib.c_verrou_count_fp_not_instrumented.restype= ctypes.c_uint

        self.lib.c_verrou_print_denorm_counter.argtypes=[]
        self.lib.c_verrou_reset_denorm_counter.argtypes=[]

        self.lib.c_verrou_set_rounding.argtypes = [ ctypes.c_char_p]

bindVerrou=bindingVerrouCLib()

def start_instrumentation():
    bindVerrou.lib.c_verrou_start_instrumentation()

def stop_instrumentation():
    bindVerrou.lib.c_verrou_stop_instrumentation()

def start_soft_instrumentation():
    bindVerrou.lib.c_verrou_start_soft_instrumentation()

def stop_soft_instrumentation():
    bindVerrou.lib.c_verrou_stop_soft_instrumentation()

def start_determinitic (level):
    bindVerrou.lib.c_verrou_start_determinitic(level)

def stop_determinitic (level):
    bindVerrou.lib.c_verrou_stop_determinitic(level)

def dump_cover():
    return bindVerrou.lib.c_verrou_dump_cover();

def display_counters():
    bindVerrou.lib.c_verrou_display_counters()

def count_fp_instrumented():
    return bindVerrou.lib.c_verrou_count_fp_instrumented()

def count_fp_not_instrumented():
    return bindVerrou.lib.c_verrou_count_fp_not_instrumented()

def print_denorm_counter():
    bindVerrou.lib.c_verrou_print_denorm_counter()

def reset_denorm_counter():
    bindVerrou.lib.c_verrou_reset_denorm_counter()

def set_rounding(roundingStr):
    bindVerrou.lib.c_verrou_set_rounding(roundingStr.upper().encode('utf-8'))

def compute_verrou_tolerance(functor,
                             tabDet=["upward", "downward", "toward_zero", "farthest", "away_zero"],
                             tabSto=["random","average","sr_smonotonic","sr_monotonic"],
                             nbSamples=1000):
    set_rounding("nearest")
    res_ref=functor()
    tab=[]
    for rnd in tabDet:
        set_rounding(rnd)
        tab+=[functor()]
    for rnd in tabSto:
        set_rounding(rnd)
        tab+=[functor() for i in range(nbSamples) ]
    set_rounding("nearest")
    errAbs=max([abs((x - res_ref))  for x in tab])
    errRel=errAbs / abs(res_ref)
    errRelUlp=errRel / sys.float_info.epsilon
    return (errAbs, errRel, errRelUlp)

if __name__=="__main__":

    print("avt a binding fp: ", count_fp_instrumented())
    print("avt a binding not fp: ", count_fp_not_instrumented())
    a=3.*4.
    print(a)
    print("apres a binding fp: ", count_fp_instrumented())
    print("apres a binding not fp: ", count_fp_not_instrumented())


    task("toto",10)

    display_counters()
    start_instrumentation()
    a=3.*4
    stop_instrumentation()

    task("toto",10)
    start_instrumentation()

    a*=5
    stop_instrumentation()

    print(a)
    task("toto",10)

