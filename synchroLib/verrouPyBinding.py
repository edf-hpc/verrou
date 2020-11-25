#!/usr/bin/python3
import sys, platform
import ctypes, ctypes.util


import os
import os.path




class bindingSynchroLib:

    def __init__(self, pathLib=None):
        if(pathLib!=None):
            self.lib=ctypes.CDLL(pathLib)
        else:
            self.lib=ctypes.CDLL(searchDefaultPath("verrouSynchroLib.so") )
        self.lib.verrou_synchro.argtypes = [ ctypes.c_char_p, ctypes.c_int]
        
    def verrou_synchro_init(self):
        print("bindingSynchroLib : verrou_synchro_init")
        self.lib.verrou_synchro_init()

    def verrou_synchro_finalyze(self):
        print("bindingSynchroLib : verrou_synchro_finalyze")
        self.lib.verrou_synchro_finalyze()

    def verrou_synchro(self,string, index):
        print("bindingSynchroLib : verrou_synchro", string, index)
        self.lib.verrou_synchro(string.encode('utf-8'), index)

class bindingVerrouCLib:
    def __init__(self, pathLib):
        if(pathLib!=None):
            self.lib=ctypes.CDLL(pathLib)
        else:
            self.lib=ctypes.CDLL(searchDefaultPath("verrouCBindingLib.so") )

        self.lib=ctypes.CDLL(pathLib)
        self.lib.c_verrou_start_instrumentation.argtypes = []
        self.lib.c_verrou_stop_instrumentation.argtypes = []
        self.lib.c_verrou_start_determinitic.argtypes = [ctypes.c_int]
        self.lib.c_verrou_stop_determinitic.argtypes  = [ctypes.c_int]

        self.lib.c_verrou_display_counters.argtypes = []

        self.lib.c_verrou_dump_cover.argtypes= []
        self.lib.c_verrou_dump_cover.restype=  ctypes.c_uint

        self.lib.c_verrou_count_fp_instrumented.argtypes= []
        self.lib.c_verrou_count_fp_not_instrumented.argtypes= []
        self.lib.c_verrou_count_fp_instrumented.restype=  ctypes.c_uint
        self.lib.c_verrou_count_fp_not_instrumented.restype= ctypes.c_uint


    def verrou_start_instrumentation(self):
        self.lib.c_verrou_start_instrumentation()

    def verrou_stop_instrumentation(self):
        self.lib.c_verrou_stop_instrumentation()
        
    def verrou_start_determinitic (self, level):
        self.lib.c_verrou_start_determinitic(level)

    def verrou_stop_determinitic (self, level):
        self.lib.c_verrou_stop_determinitic(level)

    def verrou_dump_cover(self):
        return self.lib.c_verrou_dump_cover();
        
    def verrou_display_counters(self):
        self.lib.c_verrou_display_counters()

    def verrou_count_fp_instrumented(self):
        return self.lib.c_verrou_count_fp_instrumented()
    
    def verrou_count_fp_not_instrumented(self):
        return self.lib.c_verrou_count_fp_not_instrumented()


def searchDefaultPath(fileName):
    dirName=os.path.dirname(os.path.abspath(__file__))
    pathPrefixTab=["./", dirName, os.path.join(dirName, "..", "lib")]

    print(pathPrefixTab, file=sys.stderr)
    for pathPrefix in pathPrefixTab:
        absPath=os.path.join(pathPrefix, fileName)
        if os.path.exists(absPath):
            print("absPath: ", absPath,file=sys.stderr)
            return absPath
    print("FileName %s not found"%(fileName),file=sys.stderr)
    sys.exit(42)

    
    

    
if __name__=="__main__":
    verrouSynchroLib="./verrouSynchroLib.so"
    verrouCBindingLib="./verrouCBindingLib.so"

    bindVerrou=bindingVerrouCLib(verrouCBindingLib)
    bindVerrou.verrou_stop_instrumentation()
    bindVerrou.verrou_display_counters()


    
#    print("dumpCover : ",bindVerrou.verrou_dump_cover())
#    print("dumpCover : ",bindVerrou.verrou_dump_cover())
#    print("dumpCover : ",bindVerrou.verrou_dump_cover())
        
    
    
    
    print("avt a binding fp: ", bindVerrou.verrou_count_fp_instrumented())
    print("avt a binding not fp: ", bindVerrou.verrou_count_fp_not_instrumented())
    a=3.*4.
    print(a)
    print("apres a binding fp: ", bindVerrou.verrou_count_fp_instrumented())
    print("apres a binding not fp: ", bindVerrou.verrou_count_fp_not_instrumented())


    bindSynchro=bindingSynchroLib(verrouSynchroLib)
    

    
    bindSynchro.verrou_synchro_init()
    bindSynchro.verrou_synchro("toto",10)    

    bindVerrou.verrou_display_counters()
    bindVerrou.verrou_start_instrumentation()
    a=3.*4
    bindVerrou.verrou_stop_instrumentation()

    bindSynchro.verrou_synchro("toto",10)
    bindVerrou.verrou_start_instrumentation()

    a*=5
    bindVerrou.verrou_stop_instrumentation()

    
    print(a)
    bindSynchro.verrou_synchro("toto",10)    

    bindSynchro.verrou_synchro_finalyze()

