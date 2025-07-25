#!/usr/bin/python3

import os
import sys
from pathlib import Path

#Script use to generate all xoshiro function with a specific name to be able to use different one in the same binary. If you accept modern C++ (>17) you should consider
# https://github.com/Reputeless/Xoshiro-cpp instead.


repOrgImpl=Path("org")

def extractFun(fileName, name):
    res=[]
    numberOfAcc=None
    for line in open(fileName,"r").readlines():
        if name in line: #should implement regexp
            numberOfAcc=0
        if numberOfAcc!=None:
            res+=[line]
            numberOfAcc+=line.count("{")
            numberOfAcc-=line.count("}")
        if numberOfAcc==0:
            return res

def replaceFunName(extractFun, oldName, newName):
    return [x.replace(oldName, newName) for x in extractFun]


def addParamState(extractedFun, paramState, paramStateName="s"):
    if "(void)" in extractedFun[0]:
        return [extractedFun[0].replace("(void)", "("+paramState +"& "+paramStateName+")")] +extractedFun[1:]
    if "()" in extractedFun[0]:
        return [extractedFun[0].replace("()", "("+paramState +"& "+paramStateName+")")] +extractedFun[1:]
    print("error : impossible to add param")
    sys.exit(42)

def addHeader(extractedFun):
    return [extractedFun[0].replace("{",";")] +extractedFun

def addInline(extractedFun):
    return ["inline "+extractedFun[0]] +extractedFun[1:]

def writeFun(handler,extractedFun):
    if handler==None:
        for line in extractedFun:
            print(line[0:-1])
    else:
        for line in extractedFun:
            handler.write(line)

def genNextAndRotl(impl, size, r=False):
    assert(size in [128,256])
    assert(str(size) in impl)

    state="xoshiro"+str(size)+"_state_t"
    if r:
        state="xoroshiro"+str(size)+"_state_t"
        assert(size in [128])

    pathName= repOrgImpl / (impl+".c")

    fun_next=extractFun(pathName, 'next')
    fun_next=addParamState(fun_next , state)
    fun_next=replaceFunName(fun_next, "next", impl+"_next")
    fun_next=replaceFunName(fun_next, "rotl", impl+"_rotl")
    fun_next=addInline(fun_next);

    fun_rotl=extractFun(pathName, 'rotl')
    fun_rotl=replaceFunName(fun_rotl, "rotl", impl+"_rotl")

    return fun_rotl +["\n"] +fun_next


if __name__=="__main__":

    implemList128 =["xoshiro128plus","xoshiro128plusplus","xoshiro128starstar"]
    implemList128r=["xoroshiro128plus","xoroshiro128plusplus","xoroshiro128starstar"]
    implemList256 =["xoshiro256plus", "xoshiro256plusplus", "xoshiro256starstar"]

    implMix="splitmix64"
    res=["//generated by %s\n"%(str(sys.argv))]
    res+=["//generated from %s\n"%(str([str(repOrgImpl / (impl+".c")) for impl in implemList128 + implemList128r + implemList256 ]+[implMix]))]
    res+=["// cf. copyright\n"]
    res+=["\n"]
    res+=["#include<stdint.h>\n"]
    res+=["#include<cfloat>\n"]
    res+=["\n"]

    res+=["typedef uint32_t xoshiro128_state_t[4];\n"]
    res+=["typedef uint64_t xoshiro256_state_t[4];\n"]
    res+=["typedef uint64_t xoroshiro128_state_t[2];\n"]

    res+=["\n"]

    for impl in implemList128:
        res+=genNextAndRotl(impl,128)
    for impl in implemList128r:
        res+=genNextAndRotl(impl,128,r=True)
    for impl in implemList256:
        res+=genNextAndRotl(impl,256)

    res+=["\n"]
    pathName=repOrgImpl / (implMix+".c")
    fun_next=extractFun(pathName, "next");
    fun_next=addParamState(fun_next , "uint64_t ", "x" );
    fun_next=replaceFunName(fun_next, "next", implMix+"_next")
    fun_next=addInline(fun_next);
    res+=fun_next

    init_code="""
inline void init_xoshiro256_state(xoshiro256_state_t& state, uint64_t seed){
  uint64_t splitMixState=seed;
  state[0]= splitmix64_next(splitMixState);
  state[1]= splitmix64_next(splitMixState);
  state[2]= splitmix64_next(splitMixState);
  state[3]= splitmix64_next(splitMixState);
}

inline void init_xoroshiro128_state(xoroshiro128_state_t& state,uint64_t seed){
  uint64_t splitMixState=seed;
  state[0]= splitmix64_next(splitMixState);
  state[1]= splitmix64_next(splitMixState);
}

inline void init_xoshiro128_state(xoshiro128_state_t& state,uint64_t seed){
  uint64_t splitMixState=seed;
  uint64_t state01=splitmix64_next(splitMixState);
  uint64_t state23=splitmix64_next(splitMixState);
  state[0]= (uint32_t)state01;
  state[1]= (uint32_t)(state01 << 32);
  state[2]= (uint32_t)state23;
  state[3]= (uint32_t)(state23 << 32);
}
"""
    res+=[line+"\n" for line in init_code.split("\n")]


    convFloatCode="""
inline float xoshiro_uint32_to_float(const uint32_t i){
    constexpr float factor(0.5*FLT_EPSILON);//0x1.0p-24
    return (i >> 8) * factor;
};

inline double xoshiro_uint64_to_double(const uint64_t i){
   constexpr double factor(0.5*DBL_EPSILON);//0x1.0p-53
    return (i >> 11) * factor;
};
"""

    res+=[line+"\n" for line in convFloatCode.split("\n")]

    writeFun(open("xoshiro.cxx","w"), res)

