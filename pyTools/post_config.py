import getopt
import os
import sys
import re
import copy

class postConfig:

    def __init__(self, argv, environ,config_keys=["INTERFLOP"]):
        self.config_keys=config_keys
        self.registerOptions()
        self.readDefaultValueFromRegister()
        self.parseArgv(argv)
        for config_key in self.config_keys:
            self.read_environ(environ, config_key)
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


    def readDefaultValueFromRegister(self):
        for registry in self.registryTab:
            attribut=registry[0]
            default=registry[4]
            exec("self."+attribut+"= copy.deepcopy(default)")

    def optionToStr(self):
        strOption=""
        for registry in self.registryTab:
            attribut=registry[0]
            strOption+="\t%s : %s\n"%(attribut,eval("str(self."+attribut+")"))
        return strOption

    def parseArgv(self,argv):
        shortOptionsForGetOpt="h"
        longOptionsForGetOpt=["help"] + [y[2:]  for x in self.registryTab for y in x[3] ]
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
                for registryName in registry[3]:
                    fromRegistryName=registryName.replace("=","")
                    if opt==fromRegistryName:
                        self.readOneOption(arg,registry[0], registry[1], registry[2],registryName,registry[5], registry[6], parse="parse")
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
                param=[registry[0], registry[1], registry[2], registry[3][0],registry[5], registry[6]]
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

    def readOneOption(self,strOption, attribut,conv_type ,key_name, argv_name, acceptedValue, addAttributTab, parse):        
        value=False

        if conv_type=="int":
            value = int(strOption)
        else:
            value = strOption

        if conv_type=="bool":
            value=True

        cmd="self."+attribut+"= copy.deepcopy(value)"
        if addAttributTab:
            cmd="self."+attribut+"+= [copy.deepcopy(value)]"

        if acceptedValue==None :
            exec(cmd)
            return

        elif acceptedValue=="rep_exists":
            if os.path.isdir(value):
                exec(cmd)
                return
            else:
                if parse=="environ":
                    print("Error : "+ self.PREFIX+"_"+key_name+ " should be a directory")
                else:
                    print("Error : "+ argv_name+" :  " + strOption+" should be a directory")
                self.failure()
        else:
            if value in acceptedValue:
                exec(cmd)
                return
            elif conv_type=="string/int":
                try:
                    value=int(value)
                    exec(cmd)
                    return
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
            if len(option)==1:
                optionStr=str(option[0])
            else:
                optionStr=" or ".join(option)
            optionNameStr="%s or %s"%(PREFIX+"_"+envVar, optionStr)

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
