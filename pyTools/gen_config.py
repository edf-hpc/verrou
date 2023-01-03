import getopt
import os
import sys
import re
import copy

class gen_config:

    def __init__(self, argv, environ,config_keys=["INTERFLOP"], lengthValidTab=[2]):
        self.config_keys=config_keys
        self.registryTab =[]
        self.registerOptions()
        self.readDefaultValueFromRegister()
        self.parseArgv(argv, lengthValidTab)
        for config_key in self.config_keys:
            self.read_environ(environ, config_key)

    def addRegistry(self,attribut, optionType, ENV, tabOption, default, checkParam=None, additive=False, docStr=None ):
        registry={"attribut":attribut, "type": optionType, "ENV":ENV, "tabOption":tabOption,
                  "default":default, "checkParam":checkParam,
                  "additive":additive,
                  "docStr":docStr}
        self.registryTab+=[registry]

    def readDefaultValueFromRegister(self):
        for registry in self.registryTab:
            attribut=registry["attribut"]
            default=registry["default"]
            exec("self."+attribut+"= copy.deepcopy(default)")

    def optionToStr(self):
        strOption=""
        for registry in self.registryTab:
            attribut=registry["attribut"]
            strOption+="\t%s : %s\n"%(attribut,eval("str(self."+attribut+")"))
        return strOption

    def parseArgv(self,argv, lengthValidTab):
        shortOptionsForGetOpt="h" + "".join([y[1:]  for x in self.registryTab for y in x["tabOption"] if y.startswith("-") and y[1]!="-"])
        longOptionsForGetOpt=["help"]   +   [y[2:]  for x in self.registryTab for y in x["tabOption"] if y.startswith("--")]
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
                for registryName in registry["tabOption"]:
                    fromRegistryName=registryName.replace("=","")
                    fromRegistryName=fromRegistryName.replace(":","")
                    if opt==fromRegistryName:
                        self.readOneOption(arg,registry["attribut"], registry["type"], registry["ENV"],registryName,registry["checkParam"], registry["additive"], parse="parse")
                        break

        if len(args) in lengthValidTab:
            self.exec_arg=[self.checkScriptPath(arg) for arg in args]
        else:
            self.usageCmd()
            self.failure()

    def read_environ(self,environ, PREFIX):
        self.environ=environ #configuration to prepare the call to readOneOption
        self.PREFIX=PREFIX

        for registry in self.registryTab:
            try:
                strValue=self.environ[self.PREFIX+"_"+registry["ENV"]]
                param=[registry["attribut"], registry["type"], registry["ENV"], registry["tabOption"][0],registry["checkParam"], registry["additive"]]
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
            if os.path.isfile(fpath) and not os.access(fpath, os.X_OK):
                print(fpath + " should be executable")
            if not os.path.isfile(fpath):
                print(fpath + " is not a file")
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

    def get_EnvDoc(self,PREFIX="INTERFLOP"):
        doc="""List of env variables and options :\n"""
        for registry in self.registryTab:
#            (attribut, attributType, envVar, option, default, expectedValue, add)=registry

            optionTab=registry["tabOption"]
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

