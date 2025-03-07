import getopt
import os
import sys
import re
import copy

import gen_config
import rounding_tool

class postConfig(gen_config.gen_config):

    def __init__(self, argv, environ,config_keys=["POST"]):
        super().__init__(argv,environ, config_keys)

        self.normalize()
        self.check_instr_tab()
        self.check_trace_file()

    def registerOptions(self):
        self.addRegistry("nbRUN",      "int", "NRUNS",        ["--nruns="],           5)
        self.addRegistry("maxNbPROC",  "int", "NUM_THREADS",  ["--num-threads="],  None)
        self.addRegistry("ddQuiet",   "bool", "QUIET",        ["--quiet"],        False)
        self.addRegistry("rep",     "string", "REP",          ["--rep="], "dd.line", "rep_exists")
        self.addRegistry("sub_rep", "string", "CONFIGURATION",["--sub-rep=","--configuration="],  [] , "rep_exists", additive=True)
        self.addRegistry("instr"  , "string", "INSTR",        ["--instr="],   [] , additive=True)
        self.addRegistry("rounding", "string", "ROUNDING_LIST", ["--rounding=","#--rounding-list=","#--rounding-mode"] , [], additive=True,
                         docStr="rounding mode list (coma separated) [default rounding in run.sh]",
                         suggestionForComaList= rounding_tool.roundingDetTabWithFloatPrefix +rounding_tool.roundingNonDetTabWithFloatPrefix + ["mca-rr-53-24", "mca-pb-53-24", "mca-mca-53-24"])
        self.addRegistry("trace_bin",    "bool",   "TRACE_BIN",     ["--trace-bin"],     False)
        self.addRegistry("trace_pattern","string", "TRACE_PATTERN", ["--trace-pattern="], [], additive=True)
        self.addRegistry("trace_file", "string",   "TRACE_FILE",    ["--trace-file="],    None)
        self.addRegistry("seed"               ,  int,         "SEED",                 ["--seed="],                 None,      None)
        self.addRegistry("count_denorm", "bool", "COUNT_DENORM", ["--count-denorm"],False)

    def usageCmd(self):
        print("Usage: "+ os.path.basename(sys.argv[0]) + " [options] runScript cmpScript")
        print(self.get_EnvDoc(self.config_keys[-1]))

        print("Valid rounding modes are:")
        print("\t",  ",".join(rounding_tool.roundingDetTabWithoutFloatPrefix  ))
        print("\t",  ",".join(rounding_tool.roundingNonDetTabWithoutFloatPrefix  ))
        print("\t",  ",".join(["mca-rr-53-24", "mca-pb-53-24", "mca-mca-53-24"]) , "(53 and 24 can be modified)")
        print("\t rounding mode can be prefixed by \"float_\"")
        print("\t det is an alias to "+",".join( [x for x in rounding_tool.roundingDetTabWithoutFloatPrefix if not x in ["float","ftz","daz","dazftz","native"] ] ))
        print("\t no_det is an alias to "+",".join(["random","average", "prandom"]))


    def normalize(self):
        self.rep=os.path.abspath(self.rep)
        if self.trace_file!=None:
            self.trace_file=os.path.abspath(self.trace_file)
        for r in self.rounding:
            if "," in r:
                splitR=r.split(",")
                self.rounding.remove(r)
                self.rounding+=splitR

        if "det" in self.rounding:
            self.rounding.remove("det")
            self.rounding+=[x for x in rounding_tool.roundingDetTabWithoutFloatPrefix if not x in ["float","ftz","daz","dazftz","native"] ]
        if "no_det" in self.rounding:
            self.rounding.remove("no_det")
            self.rounding+=["random","average", "prandom"]
        #check valid rounding
        for r in self.rounding:
            runEnv=rounding_tool.roundingToEnvVar(r,{})

        if len(self.exec_arg)==2:
            self.runScript=self.exec_arg[0]
            self.cmpScript=self.exec_arg[1]
        else:
            self.usageCmd()
            self.failure()

    def check_instr_tab(self):
        for instrConfig in self.instr:
            for instr in instrConfig.split(","):
                validInstrTab=["add","sub", "mul","div", "mAdd", "mSub", "sqrt","conv"]
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
            self.saveParam(os.path.join(self.rep,"cmd_post.last"))
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
            for rep in res:
                self.saveParam(os.path.join(rep,"cmd_post.last"))
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

    def get_count_denorm(self):
        return self.count_denorm

    def findDDmin(self, rep):
        ddminList=[os.path.abspath(os.path.join(rep,x)) for x in os.listdir(rep) if (re.match("^ddmin[0-9]+$",x) or x=="rddmin-cmp") or x=="FullPerturbation" or x=="NoPerturbation"]
        return ddminList
