# This file is part of Verrou, a FPU instrumentation tool.

# Copyright (C) 2014-2021 EDF
#   F. Févotte <francois.fevotte@edf.fr>
#   B. Lathuilière <bruno.lathuiliere@edf.fr>


# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307, USA.

# The GNU Lesser General Public License is contained in the file COPYING.

import os
import math
import sys
import getopt

def exponentialRange(nbRun):
    tab=[int(nbRun / (2**i)) for i in range(1+int(math.floor(math.log(nbRun,2)))) ]
    tab.reverse()
    return tab


class ddConfig:

    def __init__(self, argv, environ,config_keys=["INTERFLOP"]):
        self.config_keys=config_keys
        self.registerOptions()
        self.readDefaultValueFromRegister()
        self.parseArgv(argv)
        for config_key in self.config_keys:
            self.read_environ(environ, config_key)
        self.normalizeOptions()

    def registerOptions(self):
        self.registryTab =[("nbRUN",                 "int",        "DD_NRUNS",                   ("--nruns="),                  5,          None, False)]
        self.registryTab+=[("maxNbPROC",             "int",        "DD_NUM_THREADS",             ("--num-threads="),            None,       None, False )]
        self.registryTab+=[("ddAlgo",                "string",     "DD_ALGO",                    ("--algo="),                   "rddmin",   ["ddmax", "rddmin"], False)]
        self.registryTab+=[("rddminVariant",         "string",     "DD_RDDMIN",                  ("--rddmin="),                 "d",        ["s", "stoch", "d", "dicho", "", "strict"],False)]
        self.registryTab+=[("param_rddmin_tab",      "string",     "DD_RDDMIN_TAB",              ("--rddmin-tab="),             "exp",      ["exp", "all", "single"], False)]
        self.registryTab+=[("param_dicho_tab",       "int/string", "DD_DICHO_TAB" ,              ("--dicho-tab="),              "half",     ["exp", "all", "half", "single"], False)]
        self.registryTab+=[("splitGranularity",      "int",        "DD_DICHO_GRANULARITY",       ("--dicho-granularity="),       2,         None, False)]
        self.registryTab+=[("ddQuiet",               "bool",       "DD_QUIET",                   ("--quiet"),                    False,     None, False)]
        self.registryTab+=[("cache",                 "string",     "DD_CACHE" ,                  ("--cache=") ,                  "continue",["clean", "rename", "rename_keep_result","keep_run", "continue"], False)]
        self.registryTab+=[("rddminHeuristicsCache", "string",     "DD_RDDMIN_HEURISTICS_CACHE", ("--rddmin-heuristics-cache="), "none",    ["none", "cache", "all_cache"], False)]
        self.registryTab+=[("rddminHeuristicsRep"  , "string",     "DD_RDDMIN_HEURISTICS_REP",   ("--rddmin-heuristics-rep="),   [] ,       "rep_exists", True)]
        self.registryTab+=[("resWithAllSamples"    , "bool",       "DD_RES_WITH_ALL_SAMPLES",    ("--res-with-all-samples"),     False,     None, False)]

    def readDefaultValueFromRegister(self):
        for registry in self.registryTab:
            (attribut, attributType, envVar, option, default, expectedValue, add)=registry
            exec("self."+attribut+"= default")

    def optionToStr(self):
        strOption=""
        for registry in self.registryTab:
            (attribut, attributType, envVar, option, default, expectedValue, add)=registry
            strOption+="\t%s : %s\n"%(attribut,eval("str(self."+attribut+")"))
        return strOption

    def parseArgv(self,argv):
        shortOptionsForGetOpt="h"
        longOptionsForGetOpt=["help"] + [x[3][2:] for x in self.registryTab]
        try:
            opts,args=getopt.getopt(argv[1:], shortOptionsForGetOpt, longOptionsForGetOpt)
        except getopt.GetoptError:
            self.usageCmd()
            self.failure()

        for opt, arg in opts:
            if opt in ["-h","--help"]:
                self.usageCmd()
                self.failure()
            for registry in self.registryTab:
                fromRegistryName=registry[3].replace("=","")
                if opt==fromRegistryName:
                    param=[registry[0], registry[1], registry[2],registry[3],registry[5], registry[6]]
                    self.readOneOption(arg,*param, parse="parse")
                    break
        if len(args)==2:
            self.runScript=self.checkScriptPath(args[0])
            self.cmpScript=self.checkScriptPath(args[1])
        else:
            self.usageCmd()
            self.failure()

    def read_environ(self,environ, PREFIX):
        self.environ=environ #configuration to prepare the call to readOneOption
        self.PREFIX=PREFIX

        for registry in self.registryTab:
            try:
                strValue=self.environ[self.PREFIX+"_"+registry[2]]
                param=[registry[0], registry[1], registry[2], registry[3],registry[5], registry[6]]
                self.readOneOption(strValue,*param, parse="environ")
            except KeyError:
                pass

    def usageCmd(self):
        print("Usage: "+ os.path.basename(sys.argv[0]) + " [options] runScript cmpScript")
        print(self.get_EnvDoc(self.config_keys[-1]))

    def failure(self):
        sys.exit(42)

    def checkScriptPath(self,fpath):
        if os.path.isfile(fpath) and os.access(fpath, os.X_OK):
            return os.path.abspath(fpath)
        else:
            print("Invalid Cmd:"+str(sys.argv))
            print(fpath + " should be executable")
            self.usageCmd()
            self.failure()

    def normalizeOptions(self):
        if self.rddminVariant=="stoch":
            self.rddminVariant="s"
        if self.rddminVariant=="dicho":
            self.rddminVariant="d"
        if self.rddminVariant=="strict":
            self.rddminVariant=""


    def readOneOption(self,strOption, attribut,conv_type ,key_name, argv_name, acceptedValue=None, addAttributTab=False, parse="environ"):
        value=False

        if conv_type=="int":
            value = int(strOption)
        else:
            value = strOption

        if conv_type=="bool":
            value=True

        if acceptedValue==None :
            if addAttributTab:
                exec("self."+attribut+"+= [value]")
            else:
                exec("self."+attribut+"= value")
        elif acceptedValue=="rep_exists":
            if os.path.isdir(value):
                if addAttributTab:
                    exec("self."+attribut+"+= [value]")
                else:
                    exec("self."+attribut+"= [value]")
            else:
                if parse=="environ":
                    print("Error : "+ self.PREFIX+"_"+key_name+ " should be a directory")
                else:
                    print("Error : "+ argv_name+" :  " + strOption+" should be a directory")
                self.failure()

        else:
            if value in acceptedValue:
                if addAttributTab:
                    exec("self."+attribut+"+= [value]")
                else:
                    exec("self."+attribut+"= value")
            elif conv_type=="string/int":
                try:
                    if addAttributTab:
                        exec("self."+attribut+"+= [int(value)]")
                    else:
                        exec("self."+attribut+"= int(value)")
                except:
                    if parse=="environ":
                        print("Error : "+ self.PREFIX+"_"+key_name+ " should be in "+str(acceptedValue) +" or be a int value")
                    else:
                        print("Error : "+ argv_name+" :  " + strOption+" should be in "+str(acceptedValue) +" or be a int value")
                    self.failure()
            else:
                if parse=="environ":
                    print("Error : "+ self.PREFIX+"_"+key_name+ " should be in "+str(acceptedValue))
                else:
                    print("Error : "+ argv_name+" :  " + strOption+" should be in "+str(acceptedValue))
                self.failure()

    def get_EnvDoc(self,PREFIX="INTERFLOP"):
        doc="""List of env variables and options :\n"""
        for registry in self.registryTab:
            (attribut, attributType, envVar, option, default, expectedValue, add)=registry
            optionNameStr="%s or %s"%(PREFIX+"_"+envVar, option)

            expectedValueStr=""
            if expectedValue!=None:
                expectedValueStr="in "+str(expectedValue)

            typeStr=""
            if attributType== "int":
                typeStr="int"
            if attributType== "int/string":
                typeStr="or int"
            if attributType=="bool":
                typeStr="set or not"

            defaultStr='(default "%s")'%(default)
            if default==None:
                defaultStr='(default none)'
            if default==False:
                defaultStr='(default not)'
            if default==True:
                defaultStr='(default set)'

            doc+="\t%s : %s %s %s \n"%(optionNameStr,expectedValueStr,typeStr, defaultStr)
        return doc

    ## Accessors
    def get_splitGranularity(self):
        return self.splitGranularity

    def get_ddAlgo(self):
        if self.ddAlgo.endswith("rddmin"):
            return self.rddminVariant+self.ddAlgo
        return self.ddAlgo

    def get_maxNbPROC(self):
        return self.maxNbPROC

    def get_nbRUN(self):
        return self.nbRUN

    def get_quiet(self):
        return self.ddQuiet

    def get_resWithAllsamples(self):
        return self.resWithAllSamples

    def get_rddMinTab(self):
        rddMinTab=None
        nbProc=1
        if self.maxNbPROC!=None:
            nbProc=self.maxNbPROC
        if self.param_rddmin_tab=="exp":
            if nbProc >self.nbRUN:
                return [self.nbRUN]
            else:
                return [x for x in exponentialRange(self.nbRUN) if x>=nbProc]
        if self.param_rddmin_tab=="all":
            if nbProc>self.nbRUN:
                return range(1,self.nbRUN+1)
            else:
                return range(nbProc, self.nbRUN+1)
        if self.param_rddmin_tab=="single":
            rddMinTab=[self.nbRUN]
        return rddMinTab

    def get_splitTab(self):
        splitTab=None
        if self.param_dicho_tab=="exp":
            splitTab=exponentialRange(self.nbRUN)
        if self.param_dicho_tab=="all":
            splitTab=range(self.nbRUN)
        if self.param_dicho_tab=="single":
            splitTab=[self.nbRUN]
        if self.param_dicho_tab=="half":
            splitTab=[ int(math.ceil(self.nbRUN / 2.))]
        if self.param_dicho_tab in [str(i) for i in range(1, self.nbRUN+1) ]:
            splitTab=[self.param_dicho_tab]
        return splitTab

    def get_runScript(self):
        return self.runScript

    def get_cmpScript(self):
        return self.cmpScript

    def get_cache(self):
        return self.cache


    def get_rddminHeuristicsCache(self):
        return self.rddminHeuristicsCache

    def get_rddminHeuristicsRep_Tab(self):
        return self.rddminHeuristicsRep

