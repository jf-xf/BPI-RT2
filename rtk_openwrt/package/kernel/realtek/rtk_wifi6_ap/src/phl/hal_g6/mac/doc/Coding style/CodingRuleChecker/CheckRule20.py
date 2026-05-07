import pandas as pd
import re
from progressbar import *

AXIC = ["8852A", "8852B", "8852C", "8852D", "8192XB", "8851B", "8851E", "8852BT"]
BEIC = ["1115E", "8922A", "8952A", "8934A", "8922B", "8922D"]

class CheckRule20():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.Check()

    def addOutput(self, result, filename, lineno, Message):
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Line Number": [lineno], "Message":[Message]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
        #Per IC file
        if bool(re.search('[0-9]{4}[a-z]{1,2}', os.path.basename(file))):
            ICName = re.search('[0-9]{4}[a-z]{1,2}', os.path.basename(file))[0].upper()
            for idx, line in enumerate(lines):
                lineno = idx + 1
                line = line[:line.find("/*")]
                if re.findall('MAC_((AX)|(BE))_[0-9]{4}[A-Z]{1,2}', line):
                    if len(re.findall('MAC_[A-Z]{2}_%s' %ICName, line)) == 1 and len(re.findall('MAC_[A-Z]{2}_[0-9]{4}[A-Z]{1,2}', line)) == 1:
                        self.addOutput("PASS", file, lineno, "Support IC match the filename (%s)" %line)
                    else:
                        self.addOutput("FAIL", file, lineno, "support ICs doesn't match the file (%s)" %line)
        #Common file
        else:
            ICCompileFlag = False
            currFunc = "GLOBAL"
            currLine = 0
            compileFlagCNT = dict()
            runFlagCNT = dict()
            if "mac_g6" in file or "hv_g6" in file:     icList = AXIC
            elif "mac_g7" in file or "hv_g7" in file:   icList = BEIC
            else:                                       icList = AXIC + BEIC
            for ic in icList:
                compileFlagCNT[ic]  = 0
                runFlagCNT[ic] = 0
            for idx, line in enumerate(lines):
                lineno = idx + 1
                line = line.rstrip()
                if "/*" in line:
                    line = line[:line.find("/*")]
                if "//" in line:
                    line = line[:line.find("//")]
                if line.startswith("}") and not line.startswith("};"):
                    if self.IsRunTimeFlagMoreThanCompileFlag(runFlagCNT, compileFlagCNT) and currFunc != "GLOBAL" and "eco_patch_chk" not in file and "get_chip_id_hw_def" not in currFunc:
                        self.addOutput("FAIL", file, currLine, "Support ic run time flag is less than compile flag in %s, Hint(runtimeflag%s, compileflag%s)" %(currFunc, runFlagCNT, compileFlagCNT))
                    elif any(compileFlagCNT.values()):
                        self.addOutput("PASS", file, currLine, "Support ic run time flag is the same as(or greater than) compile flag in %s" %(currFunc))
                if line and not line.startswith(("#", " ", "\t", "{", "}", "\n")) and line.endswith((",", ")")) and " " in line and "(" in line:
                    if self.isFuncDeclareLine(idx, lines):
                        line = line[:line.find("(")]
                        line = line[line.rfind(" "):].strip()
                        currFunc = line
                        currLine = lineno
                        for ic in icList:
                            compileFlagCNT[ic]  = 0
                            runFlagCNT[ic] = 0
                if re.search('MAC_((AX)|(BE))_[0-9]{4}[A-Z]{1,2}', line):
                    if "!" in line:
                        self.addOutput("FAIL", file, lineno, "Please use the positive compile flag, do not use '!' (%s)" %line)
                    for ic in re.findall('MAC_[A-Z]{2}_[0-9]{4}[A-Z]{1,2}', line):
                        ic = ic[ic.rfind("_")+1:]
                        if ic not in compileFlagCNT:
                            self.addOutput("FAIL", file, currLine, "%s should not appear in this file" %ic)
                        else:
                            compileFlagCNT[ic] += 1
                if re.search('MAC_((AX)|(BE))_CHIP_ID_[0-9]{4}[A-Z]{1,2}', line) or re.search('CHIP_ID_HW_DEF_[0-9]{4}[A-Z]{1,2}', line):
                    if re.search('!\({0,10}is_chip_id', line):
                        self.addOutput("FAIL", file, lineno, "Please use the positive run time flag, do not use '!' (%s)" %line)
                    for ic in re.findall('MAC_[A-Z]{2}_CHIP_ID_[0-9]{4}[A-Z]{1,2}', line) + re.findall('CHIP_ID_HW_DEF_[0-9]{4}[A-Z]{1,2}', line):
                        ic = ic[ic.rfind("_")+1:]
                        if ic not in runFlagCNT:
                            self.addOutput("FAIL", file, currLine, "%s should not appear in this file" %ic)
                        else:
                            runFlagCNT[ic] += 1
                if line.strip().startswith("#if"):
                    if re.search('MAC_[A-Z]{2}_[0-9]{4}[A-Z]{1,2}', line):
                        ICCompileFlag = True
                    else:
                        ICCompileFlag = False
                elif ICCompileFlag and line.strip().startswith("#endif"):
                        ICCompileFlag = False
                if line.strip() == "#else" and ICCompileFlag:
                    self.addOutput("FAIL", file, lineno, "'#else' is prohibited. Please use the positive compile flag")

    def isFuncDeclareLine(self, index, lines):
        for i in range(10000):
            if lines[index+i].strip().endswith(";"):
                return False
            elif lines[index+i].strip().startswith("{"):
                return True

    def IsRunTimeFlagMoreThanCompileFlag(self, runtimeFlagCNT, compileFlagCNT):
        for ic in compileFlagCNT.keys():
            if compileFlagCNT[ic] > runtimeFlagCNT[ic]:
                break
        else:
            return False
        return True

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
