import getopt
import os
import sys
import re
import copy

from . import gen_config
from . import rounding_tool

class postConfig(gen_config.gen_config):

    def __init__(self, argv, environ,config_keys=["POST"]):
        super().__init__(argv,environ, config_keys)

        self.normalize()
        self.check_instr_tab()
        self.check_trace_file()

    def registerOptions(self):
        self.addRegistry("nbRUN",      "int", "NRUNS",        ["--nruns="],           5, None)
        self.addRegistry("maxNbPROC",  "int", "NUM_THREADS",  ["--num-threads="],  None, None)
        self.addRegistry("ddQuiet",   "bool", "QUIET",        ["--quiet"],        False, None)
        self.addRegistry("rep",     "string", "REP",          ["--rep="], "dd.line", "rep_exists")
        self.addRegistry("sub_rep", "string", "CONFIGURATION",["--sub-rep=","--configuration="],  [] , "rep_exists", True)
        self.addRegistry("instr"  , "string", "INSTR",        ["--instr="],   [] ,   None, True)
        self.addRegistry("rounding","string", "ROUNDING",     ["--rounding=", "--rounding-mode="] ,[] , ["all_det","no_det", None]+rounding_tool.allRoundingTab, True)
        self.addRegistry("trace_bin",    "bool",   "TRACE_BIN",     ["--trace-bin"],     False, None)
        self.addRegistry("trace_pattern","string", "TRACE_PATTERN", ["--trace-pattern="], [],  None, True)
        self.addRegistry("trace_file", "string",   "TRACE_FILE",    ["--trace-file="],    None, None)


    def usageCmd(self):
        print("Usage: "+ os.path.basename(sys.argv[0]) + " [options] runScript cmpScript")
        print(self.get_EnvDoc(self.config_keys[-1]))

    def normalize(self):
        self.rep=os.path.abspath(self.rep)
        if self.trace_file!=None:
            self.trace_file=os.path.abspath(self.trace_file)
        if "all_det" in self.rounding:
            self.rounding.remove("all_det")
            self.rounding+=[x for x in rounding_tool.roundingDetTab if x !="float" ]
        if "no_det" in self.rounding:
            self.rounding.remove("no_det")
            self.rounding+=["random","average", "prandom"]

        self.runScript=self.exec_arg[0]
        self.cmpScript=self.exec_arg[1]

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

#accessors
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
        return rounding_tool.filterNonDetTab(self.rounding)

    def getDetTab(self):
        return rounding_tool.filterDetRoundingTab(self.rounding)

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



    def findDDmin(self, rep):
        ddminList=[os.path.abspath(os.path.join(rep,x)) for x in os.listdir(rep) if (re.match("^ddmin[0-9]+$",x) or x=="rddmin-cmp") or x=="ref"]
        return ddminList
