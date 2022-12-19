import getopt
import os
import sys
import re
import copy

from . import gen_config

class postConfig(gen_config.gen_config):

    def __init__(self, argv, environ,config_keys=["INTERFLOP"]):
        super().__init__(argv,environ, config_keys)

        self.normalize()
        self.check_instr_tab()
        self.check_trace_file()

    def registerOptions(self):
        self.registryTab =[("nbRUN",      "int", "POST_NRUNS",        ["--nruns="],           5, None, False)]
        self.registryTab+=[("maxNbPROC",  "int", "POST_NUM_THREADS",  ["--num-threads="],  None, None, False)]
        self.registryTab+=[("ddQuiet",   "bool", "POST_QUIET",        ["--quiet"],        False, None, False)]
        self.registryTab+=[("rep",     "string", "POST_REP",          ["--rep="], "dd.line", "rep_exists", False)]
        self.registryTab+=[("sub_rep", "string", "POST_CONFIGURATION",["--sub-rep=","--configuration="],  [] , "rep_exists", True)]
        self.registryTab+=[("instr"  , "string", "POST_INSTR",        ["--instr="],   [] ,   None, True)]
        self.registryTab+=[("rounding","string", "POST_ROUNDING",     ["--rounding=", "--rounding-mode="] ,[] , ["all_det","no_det","random","average","nearest","upward","downward", "farthest", "toward_zero","random_det","average_det", "random_comdet","average_comdet","prandom","prandom_det","prandom_comdet"], True)]
        self.registryTab+=[("trace_bin",    "bool",   "POST_TRACE_BIN",     ["--trace-bin"],     False, None, False)]
        self.registryTab+=[("trace_pattern","string", "POST_TRACE_PATTERN", ["--trace-pattern="], [],  None, True)]
        self.registryTab+=[("trace_file", "string",   "POST_TRACE_FILE",    ["--trace-file="],    None, None, False)]


    def usageCmd(self):
        print("Usage: "+ os.path.basename(sys.argv[0]) + " [options] runScript cmpScript")
        print(self.get_EnvDoc(self.config_keys[-1]))


    def get_maxNbPROC(self):
        return self.maxNbPROC

    def get_nbRUN(self):
        return self.nbRUN

    def get_quiet(self):
        return self.ddQuiet

    def get_runScript(self):
        return self.runScript

    def get_cmpScript(self):
        return self.cmpScript

    def get_rep(self):
        return self.rep

    def get_rep_sub_rep(self):
        if self.sub_rep==[]:
            return {self.rep:self.findDDmin(self.rep)}
        else:
            res={}
            for sub_rep in self.sub_rep:
                sub_rep=os.path.abspath(sub_rep)
                rep=os.path.dirname(sub_rep)
                if rep in res:
                    res[rep]+=[sub_rep]
                else:
                    if os.path.isdir(os.path.join(rep,"ref")):
                        res[rep]=[sub_rep]
                    else:
                        print("sub_rep %s is not a valid"%(sub_rep))
                        self.usageCmd()
                        self.failure()
            return res
    def get_instr(self):
        if self.instr==[]:
            return [""]
        else:
            return self.instr

    def getNonDetTab(self):
        if self.rounding==[]:
            return [""]
        return list(filter(lambda x : x in ["random", "random_det", "average","average_det","random_comdet","average_comdet", "prandom", "prandom_det", "prandom_comdet"], self.rounding ))

    def getDetTab(self):
        return list(filter(lambda x: x in ["upward","downward", "farthest", "toward_zero", "nearest"], self.rounding ))


    def get_trace_bin(self):
        return self.trace_bin

    def get_trace_pattern(self):
        return self.trace_pattern

    def get_trace_file(self):
        return self.trace_file

    def get_trace(self):
        if self.trace_bin==True:
            return True
        if self.trace_pattern!=[]:
            return True
        if self.trace_file!=None:
            return True
        return False

    def normalize(self):
        self.rep=os.path.abspath(self.rep)
        if self.trace_file!=None:
            self.trace_file=os.path.abspath(self.trace_file)
        if "all_det" in self.rounding:
            self.rounding.remove("all_det")
            self.rounding+=["upward","downward", "farthest", "toward_zero", "nearest"]
        if "no_det" in self.rounding:
            self.rounding.remove("no_det")
            self.rounding+=["random","average", "prandom"]

    def check_instr_tab(self):
        for instrConfig in self.instr:
            for instr in instrConfig.split(","):
                validInstrTab=["add","sub", "mul","div", "mAdd", "mSub", "conv"]
                if instr not in validInstrTab:
                    print("%s is not a valid instr configuration."%(instr))
                    print("%s should be a coma separated list of element of %s"%(instrConfig, str(validInstrTab)))
                    self.usageCmd()
                    self.failure()

    def check_trace_file(self):
        if self.trace_file !=None and (self.trace_pattern!=[] or self.trace_bin):
            print("--trace_file is incompatible with trace_pattern and trace_bin option")
            self.failure()

        """Basic check : not valid file could sucessed"""
        if self.trace_file!=None:
            numLine=0
            for line in (open(self.trace_file)).readlines():
                numLine+=1
                spline=line.strip()
                if spline.startswith("#") or spline=="":
                    continue
                if not (" " in spline) and not("\t" in spline):
                    print("%s is not a valid trace file"%(self.trace_file))
                    print("%s line %i is not valid"%(spline, numLine))
                    self.usageCmd()
                    self.failure()


    def findDDmin(self, rep):
        ddminList=[os.path.abspath(os.path.join(rep,x)) for x in os.listdir(rep) if (re.match("^ddmin[0-9]+$",x) or x=="rddmin-cmp") or x=="ref"]
        return ddminList
