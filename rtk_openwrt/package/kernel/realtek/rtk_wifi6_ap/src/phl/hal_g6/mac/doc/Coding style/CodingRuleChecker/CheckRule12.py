import re
import pandas as pd
from progressbar import *

class CheckRule12():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.GetMacAXAdapter()
        self.Check()

    def addOutput(self, result, filename, Message):
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Message":[Message]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
        inMacAXAdapter = False
        initAdapterStruct = []
        for line in lines:
            if bool(re.search("mac_ax_adapter mac_[0-9]{4}[a-z]{1,2}_adapter", line, re.IGNORECASE)):
                structName = re.search(" mac_[0-9]{4}[a-z]{1,2}_adapter", line)[0][1:]
                inMacAXAdapter = True
            elif inMacAXAdapter:
                if line.startswith("}"):
                    initAdapterStruct = self.ParseStruct(initAdapterStruct)
                    break
                initAdapterStruct.append(line.strip())
        if len([var for var in initAdapterStruct if type(var) is str]) == len([var for var in self.macAXAdapter if type(var) is str]):
            self.addOutput("PASS", file, "Number of struct variable defined in %s is equal to mac_ax_adapter in mac_def.h" %structName)
        else:
            self.addOutput("FAIL", file, "Please check the struct variable in %s" %structName)
        conditionalVariable = [var for var in self.macAXAdapter if type(var) is list]
        initConditionalVariable = [var for var in initAdapterStruct if type(var) is list]
        for cv in conditionalVariable:
            for icv in initConditionalVariable:
                if cv[0] == icv[0]:
                    if len(cv) != len(icv):
                        self.addOutput("FAIL", file, "Please check the struct variable in %s(%s)" %(structName, icv[0]))
                    break
            else:
                self.addOutput("WARN", file, "Conditional struct variable in %s(%s) is different with mac_ax_adapter in mac_def.h, might cause error" %(structName, cv[0]))

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)

    def GetMacAXAdapter(self):
        if len(self.checkFiles) == 0:
            self.addOutput("FAIL", "-", "Cannot found mac_ax_adapter in mac_def.h")
        else:
            macDefFile = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(self.checkFiles[0]))), "mac_def.h")
            if not os.path.exists(macDefFile):
                macDefFile = os.path.join(os.path.dirname(os.path.dirname(os.path.dirname(self.checkFiles[0]))), "include", "mac_def.h")
        with open(macDefFile, "r") as f:
            lines = f.readlines()
        inMacAXAdapter = False
        adapterStruct = []
        for line in lines:
            if line.startswith("struct mac_ax_adapter {"):
                inMacAXAdapter = True
            elif inMacAXAdapter:
                if line.startswith("}"):
                    self.macAXAdapter = self.ParseStruct(adapterStruct)
                    break
                adapterStruct.append(line.strip())

    def ParseStruct(self, struct):
        ret = []
        for line in struct:
            line = line.strip()
            if len(ret) == 0:
                ret.append(line)
            elif ret[-1][0].startswith("#if") and not self.isEndofCompileFlag(ret[-1]):
                if ret[-1][-1].count("{") != ret[-1][-1].count("}"):
                    ret[-1][-1] += line
                else:    
                    ret[-1].append(line)
            elif line.startswith("#if"):
                ret.append([line])
            elif ret[-1].count("{") != ret[-1].count("}"):
                ret[-1] += line
            else:
                ret.append(line)
        return ret

    def isEndofCompileFlag(self, list):
        ifcount, endifcount = 0, 0
        for element in list:
            if element.startswith("#if"): ifcount += 1
            if element.startswith("#endif"): endifcount += 1
        return True if ifcount == endifcount else False
