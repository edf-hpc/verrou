
class tabular:
    def __init__(self):
        self.currentStr=""
    def begin(self):
        pass
    def end(self):
        pass
    def lineSep(self):
        print("")
    def endLine(self):
        print(self.currentStr)
        self.currentStr=""
    def line(self,tab):
        self.currentStr+=("\t".join(tab))

    def lineMultiple(self, tab):
        lineTab=[]
        for (nb, value) in tab:
            for i in range(nb):
                lineTab+=[value]
        self.currentStr+= ("\t".join(lineTab))

class tabularLatex:
    def __init__(self,keyStr="c", output=None):
        self.currentStr=""
        self.keyStr=keyStr
        self.output=output
    def begin(self):
        self.currentStr+="\\begin{tabular}{%s}\\toprule\n"%self.keyStr

    def end(self):
        self.currentStr+="\\bottomrule\n"
        self.currentStr+="\end{tabular}\n"
        if self.output==None:
            print(self.currentStr)
        else:
            handler=open(self.output,"w")
            handler.write(self.currentStr)

    def lineSep(self):
        self.currentStr+="\\midrule\n"
    def endLine(self):
        self.currentStr+="\\\\\n"
    def line(self,tab):
        lineStr=("\t&\t".join(tab))
        lineStr=lineStr.replace("_","\_")
        self.currentStr+=lineStr

    def lineMultiple(self, tab):
        lineStr=""
        lineTab=[]
        for (nb, value) in tab:
            if nb>1:
                lineTab+=["\multicolumn{%s}{c}{%s}"%(str(nb), value.replace("_","\_")) ]
            if nb==1:
                lineTab+=[value.replace("_","\_")]
        lineStr+= ("\t&\t".join(lineTab))
        self.currentStr+=lineStr
