import gzip
import os

import subprocess


def runCmdAsync(cmd, fname, envvars=None):
    """Run CMD, adding ENVVARS to the current environment, and redirecting standard
    and error outputs to FNAME.out and FNAME.err respectively.

    Returns CMD's exit code."""
    if envvars is None:
        envvars = {}

    with open("%s.out"%fname, "w") as fout:
        with open("%s.err"%fname, "w") as ferr:
            env = copy.deepcopy(os.environ)
            for var in envvars:
                env[var] = envvars[var]
            return subprocess.Popen(cmd, env=env, stdout=fout, stderr=ferr)

def getResult(subProcess):
    subProcess.wait()
    return subProcess.returncode

#code coming from verificarlo : https://github.com/verificarlo/verificarlo/blob/master/src/tools/ddebug/main.py
def disable_ASLR():
    import ctypes
    import ctypes.util
    # We call personality(ADDR_NO_RANDOMIZE) to disable ASLR, so that addresses during reference run
    # always match addresses during sample runs (even for .so code).
    ADDR_NO_RANDOMIZE = 0x0040000
    libc_name = ctypes.util.find_library("c")
    libc = ctypes.CDLL(libc_name)
    personality = libc.personality
    personality(ADDR_NO_RANDOMIZE)


class openGz:
    """ Class to read/write  gzip file or ascii file """
    def __init__(self,name, mode="r", compress=None):
        self.name=name
        potentialName=name.parent / (name.name + ".gz")
        if potentialName.is_file() and compress==None:
            self.name=potentialName

        if (self.name.suffix==".gz" and compress==None) or compress==True:
            self.compress=True
            self.handler=gzip.open(self.name, mode)
        else:
            self.compress=False
            self.handler=open(self.name, mode)

    def readline(self):
        if self.compress:
            return self.handler.readline().decode("ascii")
        else:
            return self.handler.readline()
    def readlines(self):
        if self.compress:
            return [line.decode("ascii") for line in self.handler.readlines()]
        else:
            return self.handler.readlines()


    def write(self, line):
        self.handler.write(line)

