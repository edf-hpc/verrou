import getopt
import os
import sys
import copy
import datetime
import hashlib
from pathlib import Path

class gen_config:

    def __init__(self, argv, environ,config_keys=["INTERFLOP"], lengthValidTab=[2]):
        self.config_keys=config_keys
        self.registryTab =[]
        self.registerOptions()
        self.readDefaultValueFromRegister()
        if sys.argv[1]=="bash_completion":
            self.write_bashCompletion_script()
            sys.exit(0)
        self.argv=argv
        self.parseArgv(argv, lengthValidTab)
        for config_key in self.config_keys:
            self.read_environ(environ, config_key)

    def addRegistry(self,attribut, optionType, ENV, tabOption, default, checkParam=None, additive=False, docStr=None,suggestionForComaList=None ):
        registry={"attribut":attribut, "type": optionType, "ENV":ENV,
                  "tabOptionBrut":tabOption,
                  "tabOptionAll":[option.replace("#","") for option in tabOption],
                  "tabOptionVisible":[option for option in tabOption if not option.startswith("#")],
                  "default":default, "checkParam":checkParam,
                  "additive":additive,
                  "docStr":docStr,
                  "suggestionForComaList":suggestionForComaList,
                  }
        self.registryTab+=[registry]
        self.instr_ignore=[]
        self.instr_prefix=None

    def get_envVarSeed(self):
        if self.instr_prefix!=None:
            return self.instr_prefix+"_SEED"
        else:
            "INTERFLOP_SEED"

    def readDefaultValueFromRegister(self):
        for registry in self.registryTab:
            setattr(self,registry["attribut"], copy.deepcopy(registry["default"]))

    def optionToStr(self):
        strOption=""
        for registry in self.registryTab:
            strOption+="\t%s : %s\n"%(registry["attribut"], getattr(self, registry["attribut"]))
        return strOption

    def parseArgv(self,argv, lengthValidTab):
        shortOptionsForGetOpt="h" + "".join([y[1:]  for x in self.registryTab for y in x["tabOptionAll"] if y.startswith("-") and y[1]!="-"])
        longOptionsForGetOpt=["help"]   +   [y[2:]  for x in self.registryTab for y in x["tabOptionAll"] if y.startswith("--")]
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
                for registryName in registry["tabOptionAll"]:
                    fromRegistryName=registryName.replace("=","")
                    fromRegistryName=fromRegistryName.replace(":","")
                    if opt==fromRegistryName:
                        self.readOneOption(arg,registry["attribut"], registry["type"], registry["ENV"],registryName,registry["checkParam"], registry["additive"], parse="parse")
                        break

        if len(args) in lengthValidTab:
            self.exec_arg=[self.checkScriptPath(Path(arg)) for arg in args]
        else:
            self.usageCmd()
            self.failure()

    def read_environ(self,environ, PREFIX):
        self.environ=environ #configuration to prepare the call to readOneOption
        self.PREFIX=PREFIX

        for registry in self.registryTab:
            try:
                strValue=self.environ[self.PREFIX+"_"+registry["ENV"]]
                param=[registry["attribut"], registry["type"], registry["ENV"], registry["tabOptionAll"][0],registry["checkParam"], registry["additive"]]
                self.readOneOption(strValue,*param, parse="environ")
            except KeyError:
                pass

    def confToStr(self):
        """Need to be called after read_environ"""
        resEnv="ENV %s:\n"%(self.argv[0])
        for env in  self.environ:
            for prefix in self.config_keys:
                if env.startswith(prefix):
                    find=False
                    for registry in self.registryTab:
                        if env==self.prefix+"_"+registry["ENV"]:
                            strValue=self.environ[env]
                            resEnv+="\t"+self.prefix+"_"+registry["ENV"] + "="+strValue+"\n"
                            find=True
                            break
                    if not find:
                        print("Warning : unknown env variable :", env)
            if env in ["PWD"]:
                resEnv+="\t"+env+"="+self.environ[env]+"\n"

        if self.instr_prefix!=None:
            resInstr="ENV "+self.instr_prefix+":\n"
            instr=False
            for env in  self.environ:
                if env.startswith(self.instr_prefix):
                    strValue=self.environ[env]
                    resInstr+="\t"+env + "="+strValue+"\n"
                    instr=True
            if instr:
                resEnv+=resInstr

        resCmd="Cmd: "+ " ".join(self.argv)+"\n"

        now=datetime.datetime.now()
        resTime="TimeStamp: "+now.strftime("%m-%d-%Y_%Hh%Mm%Ss")+"\n"

        return resEnv + resCmd + resTime

    def saveParam(self,fileName):
        if fileName.is_file():
            timeValue=datetime.datetime.fromtimestamp(fileName.stat().st_mtime)
            timeStr=(timeValue.strftime(".%m-%d-%Y_%Hh%Mm%Ss"))
            fileName.rename(Path(str(fileName).replace(".last",timeStr)))

        strRes=self.confToStr()+"\n"

        for arg in self.exec_arg:
            content=open(arg).read()
            md5Script=hashlib.md5((content).encode('utf-8')).hexdigest()
            strRes+=("arg: "+ str(arg) + " (md5="+md5Script+")\n")
            if content.count("\n") < 100:
                strRes+="--begin content--\n"
                strRes+=content
                strRes+="--end content--\n"

        with open(fileName,"w") as f:
            f.write(strRes)
        self._md5Param=hashlib.md5((strRes).encode('utf-8')).hexdigest()

    def usageCmd(self):
        if sys.argv[1]=="bash_completion":
            self.write_bashCompletion_script()
            sys.exit(0)
        print("Usage: "+ Path(sys.argv[0]).name + " [options] runScript cmpScript")
        print(self.get_EnvDoc(self.config_keys[-1]))

    def failure(self):
        sys.exit(42)

    def md5Param(self):
        return self._md5Param

    def checkScriptPath(self,fpath,hardFailure=True):
        if fpath.is_file() and os.access(fpath, os.X_OK):
            return fpath.absolute()
        else:
            if hardFailure:
                print("Invalid Cmd:"+str(sys.argv))
                if fpath.is_file() and not os.access(fpath, os.X_OK):
                    print(fpath , " should be executable")
                if not fpath.is_file():
                    print(fpath , " is not a file")
                self.usageCmd()
                self.failure()
            else:
                return None

    def readOneOption(self,strOption, attribut,conv_type ,key_name, argv_name, acceptedValue, addAttributTab, parse):
        value=False

        if conv_type=="int":
            value = int(strOption)
        else:
            value = strOption

        if conv_type=="bool":
            if strOption in ["True","true","",None]:
                value=True
            if strOption in ["False","false"]:
                value=False

        cmd="self."+attribut+"= copy.deepcopy(value)"
        if addAttributTab:
            cmd="self."+attribut+"+= [copy.deepcopy(value)]"

        if acceptedValue==None :
            exec(cmd)
            return

        elif acceptedValue=="rep_exists":
            if Path(value).is_dir():
                exec(cmd)
                return
            else:
                if parse=="environ":
                    print("Error : "+ self.PREFIX+"_"+key_name+ " should be a directory")
                else:
                    print("Error : "+ argv_name+" :  " + strOption+" should be a directory")
                self.failure()
        elif acceptedValue=="file_exists":
            if Path(value).is_file():
                exec(cmd)
                return
            else:
                if parse=="environ":
                    print("Error : "+ self.PREFIX+"_"+key_name+ " should be a file")
                else:
                    print("Error : "+ argv_name+" :  " + strOption+" should be a file")
                self.failure()
        else:
            if value in acceptedValue or None in acceptedValue:
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

    def write_bashCompletion_script(self):
        cmdName=Path(sys.argv[0]).name
        handler=open("bash_completion_"+cmdName,"w")
        handler.write("#bash completion for %s  -*- shell-script -*-\n"%(cmdName))
        handler.write("#generated by %s\n"%(" ".join(sys.argv)))
        begin="""
_%s()
{
    local cur prev words cword
    _init_completion -n = || return
    local value prefix localCmd
"""%(cmdName)
        handler.write(begin)

        handler.write(" "*4+ "case $cur in\n")
        for registry in  self.registryTab:
            if registry["suggestionForComaList"]==None:
                continue
            optionTab=registry["tabOptionVisible"]
            caseMatchComa="|".join([option+"*,*" for option in optionTab if option.endswith("=")  ])
            caseMatchStar="|".join([option+"*" for option in optionTab if option.endswith("=")  ])
            if caseMatchComa!="" and caseMatchStar!="" :
                handler.write(" "*8+caseMatchComa+")\n")
                handler.write(" "*12+"value=${cur#*=}\n")
                handler.write(" "*12+"cur=${value}\n")
                handler.write(" "*12+"prefix=${value%,*},\n")
                handler.write(" "*12+"localCmd=${cur}\n")
                handler.write(" "*12+"if (( COMP_TYPE == 63 )); then\n")
                handler.write(" "*16+"prefix=\"\"\n")
                handler.write(" "*16+"localCmd=${cur##*,}\n")
                handler.write(" "*12+"fi\n")
                prefixSuggest=["${prefix}"+suggest for suggest in registry["suggestionForComaList"]]
                cmpReply=" "*12+ "COMPREPLY=($(compgen -W \"%s\" -- \"$localCmd\"))\n"%(" ".join(prefixSuggest))
                handler.write(cmpReply)
                handler.write(" "*12+"compopt -o nospace\n")
                handler.write(" "*12+"return\n")
                handler.write(" "*12+";;\n")

                handler.write(" "*8+caseMatchStar+")\n")
                handler.write(" "*12+"cur=${cur#*=}\n")
                cmpReply=" "*12+ "COMPREPLY=($(compgen -W '%s' -- \"$cur\"))\n"%(" ".join(registry["suggestionForComaList"]))
                handler.write(cmpReply)
                handler.write(" "*12+"compopt -o nospace\n")
                handler.write(" "*12+"return\n")
                handler.write(" "*12+";;\n")

        handler.write(" "*4+ "esac\n")
        strOptionTab=""
        handler.write(" "*4+ "case $cur in\n")
        for registry in  self.registryTab:
            optionTab=registry["tabOptionVisible"]
            strOptionTab+=" "+" ".join([option.replace(":","") for option in optionTab])

            if registry["suggestionForComaList"]!=None:
                continue
            expectedValue=registry["checkParam"]
            caseMatch="|".join([option+"*" for option in optionTab if option.endswith("=")  ])
            if caseMatch!="":
                handler.write(" "*8+caseMatch+")\n")
                handler.write(" "*12+"cur=${cur#*=}\n")
                cmpReply=""
                if str(type(expectedValue))=="<class 'list'>":
                    if None not in expectedValue and len(expectedValue)>0:
                        cmpReply=" "*12+ "COMPREPLY=($(compgen -W '%s' -- \"$cur\"))\n"%(" ".join(expectedValue))
                if expectedValue=="rep_exists":
                    cmpReply=" "*12+ "_filedir -d\n"
                if expectedValue=="file_exists":
                    cmpReply=" "*12+ "_filedir\n"
                handler.write(cmpReply)
                handler.write(" "*12+"return\n")
                handler.write(" "*12+";;\n")
        handler.write("\n"+" "*4+"esac\n")

        handler.write("\n"+ " "*4+"case $prev in\n")
        for registry in self.registryTab:
            optionTab=registry["tabOptionVisible"]
            caseMatch="|".join([option.replace(":","") for option in optionTab if option.endswith(":") ])
            if caseMatch!="":
                handler.write(" "*8+caseMatch+")\n")
                cmpReply=""
                if str(type(expectedValue))=="<class 'list'>":
                    if None not in expectedValue and len(expectedValue)>0:
                        cmpReply=" "*12+ "COMPREPLY=($(compgen -W '%s' -- \"$cur\"))\n"%(" ".join(expectedValue))
                if expectedValue=="rep_exists":
                    cmpReply=" "*12+ "_filedir -d\n"
                if expectedValue=="file_exists":
                    cmpReply=" "*12+ "_filedir\n"
                if cmpReply=="":
                    cmpReply=" "*12+"COMPREPLY=\n"
                    cmpReply+=" "*12+"compopt -o nospace\n"
                handler.write(cmpReply)
                handler.write(" "*12+"return\n")
                handler.write(" "*12+";;\n")
        handler.write("\n"+" "*4+"esac\n")

        handler.write("\n"+" "*4+"COMPREPLY=($(compgen -W '%s' -- \"$cur\"))\n"%(strOptionTab))
        handler.write("\n"+" "*4+"_filedir\n\n")

        handler.write("\n"+" "*4+"[[ ${COMPREPLY-} == *= ]] && compopt -o nospace\n")

        end="""
} &&
    complete -F _%s %s
"""%(cmdName,cmdName)
        handler.write(end)


    def get_EnvDoc(self,PREFIX="INTERFLOP"):
        doc="""List of env variables and options :\n"""
        for registry in self.registryTab:
#            (attribut, attributType, envVar, option, default, expectedValue, add)=registry

            optionTab=registry["tabOptionBrut"]
            if len(optionTab)==1:
                optionStr=str(optionTab[0])
            else:
                optionStr=" or ".join(optionTab)
            optionNameStr="%s or %s"%(PREFIX+"_"+registry["ENV"], optionStr)

            expectedValue=registry["checkParam"]
            expectedValueStr=""
            if expectedValue!=None:
                if str(type(expectedValue))=="<class 'list'>":
                    v=copy.deepcopy(expectedValue)
                    if None in v:
                        v.remove(None)
                        v+=["..."]
                    expectedValueStr="in "+str(v)
                else:
                    expectedValueStr="in "+str(expectedValue)

            attributType=registry["type"]
            typeStr=""
            if attributType== "int":
                typeStr="int"
            if attributType== "int/string":
                typeStr="or int"
            if attributType=="bool":
                typeStr="set or not"

            default=registry["default"]
            defaultStr='(default "%s")'%(default)
            if default==None:
                defaultStr='(default none)'
            if default==False:
                defaultStr='(default not)'
            if default==True:
                defaultStr='(default set)'
            if registry["docStr"]==None:
                doc+="\t%s : %s %s %s \n"%(optionNameStr,expectedValueStr,typeStr, defaultStr)
            else:
                doc+="\t%s : %s\n"%(optionNameStr,registry["docStr"])
        return doc

