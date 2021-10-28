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

import sys
import os
import time

class exec_stat:
    def __init__(self,repName):
        self.repName=repName
        self.timeInit()

    def terminate(self):
        self.timeEnd()
        self.printElapsed(int(self.end- self.start))
        self.printNbRun()

    def timeInit(self):
        self.start = time.time()

    def timeEnd(self):
        self.end = int(time.time())

    def printElapsed(self,duration):
        s= duration % 60
        rm= duration //60
        m=rm%60
        rh=rm//60
        h=rh%24
        rd=rh//24
        print ("\nElapsed Time: %id %ih %imin %is   "%(rd,h,m,s) )

    def isNew(self, filename):
        return ((os.stat(filename).st_mtime) > self.start)

    def printNbRun(self,dirName="."):
        import glob

        runTab=glob.glob(dirName+"/"+self.repName+"/*/dd.run*/dd.run.out")
        runFilter=[filename for filename in runTab if self.isNew(filename)]
        print(self.repName+"  search : %i run (with cache included: %i)"%(len(runFilter),len(runTab)) )
