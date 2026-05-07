import re
import pandas as pd
from collections import OrderedDict
from progressbar import *

class CheckRule21():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.ignoreStatementBeginPattern = ("#", "PLTFM_MSG")
        self.Check()

    def addOutput(self, filename, lineno, string):
        row = pd.DataFrame({"Judge": ["FAIL"], "Location": [filename], "Line Number": [lineno], "Message":[string]})
        self.output = pd.concat([self.output, row], ignore_index=True)
    
    def StatementPreprocess(self, line):
        if "fall" in line.lower() and "through" in line.lower():
            return [], line.lower().strip()
        line = line.strip()
        if "/*" in line:
            line = line[:line.find("/*")]
        if "//" in line:
            line = line[:line.find("//")]
        words  = re.findall('[a-zA-Z0-9_]+', line)
        return words, line

    def CheckOneFile(self, file):
        incvSwitch = 0
        inCase = 0
        switchBuf = OrderedDict()
        with open (file, "r") as f:
            lines = f.readlines()
        for idx, line in enumerate(lines):
            lineno = idx + 1
            words, line = self.StatementPreprocess(line)
            if not line or line.startswith(self.ignoreStatementBeginPattern):
                continue
            ### Scan all statement in switch(cv) ###
            if incvSwitch:
                if not inCase and line.startswith("}"):
                    ### No default statement ###
                    if "default" not in switchBuf:
                        defaultStatement = ""
                        self.addOutput(file, lineno, "Need fall through comment for specific compiler")
                    else:
                        defaultStatement = switchBuf["default"]
                        del switchBuf["default"]
                        tupleList = list(switchBuf.items())
                        ### Default statement and latest statement is not mismatch ###
                        latestStatement = tupleList[-1][1]
                        if len(tupleList) > 0 and defaultStatement != latestStatement and \
                        not (len(latestStatement) == 1 and "fall" in latestStatement[0] and "through" in latestStatement[0]):
                            self.addOutput(file, lineno, "Default statement is not equivalent to lastest statement")
                    switchBuf = OrderedDict()
                    incvSwitch = 0
                    inCase = 0
                elif not inCase and (line.startswith("case") or line.startswith("default")):
                    lastCondition = line.replace("case", "").replace(":", "")
                    switchBuf[lastCondition] = []
                    inCase += 1
                elif inCase > 0 : #For Nested Switch Case
                    switchBuf[lastCondition].append(line)
                    if (line.startswith(("return", "break")) or ("fall" in line and "through" in line)):
                        inCase -= 1
                    elif (line.startswith("case") or line.startswith("default")):
                        inCase += 1
                else:
                    if not line.startswith("PLTFM_MSG_ERR"):
                        switchBuf[lastCondition].append(line)
            if ("is_cv" in words or "cv" in words) and "!cv" not in line:
                if not incvSwitch:
                    if line.startswith("switch"):
                        incvSwitch = 1
                        switchBuf = OrderedDict()

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)