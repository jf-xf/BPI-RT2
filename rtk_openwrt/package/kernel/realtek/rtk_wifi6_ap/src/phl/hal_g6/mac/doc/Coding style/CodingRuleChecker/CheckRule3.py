import re
import pandas as pd
from progressbar import *
alloc_patterns  = [r"h2cb_alloc", r"h2db_alloc", r"PLTFM_MALLOC"]
alloc_regex     = ["[ ]*(.*)?[ ]\=.*" + pattern + ".*?\((.*)\);" for pattern in alloc_patterns]
if_condition_regex      = r"if.*(!%s)"
after_lines = 5

class CheckRule3():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.Check()

    def StatementPreprocess(self, line):
        line = line.strip()
        if "/*" in line:    line = line[:line.find("/*")]
        if "//" in line:    line = line[:line.find("//")]
        return line

    def addOutput(self, result, filename, lineno, mallocMessage, protectionMsg):
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Line Number": [lineno], "MALLOC Message":[mallocMessage], "Protection Message":[protectionMsg]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def GetMallocVariable(self, string):
        words = re.findall('[a-zA-Z0-9_=>.\-\*\[\]\(\)]+', string)
        if "=" in words:    variable = words[words.index("=")-1]
        else:               variable = string.split("=")[0].strip()
        return variable.replace("*", "\*").replace(")", "\)").replace("(", "\(").replace("[", "\[").replace("]", "\]")

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
        prevLine = ""
        for idx, line in enumerate(lines):
            lineno = idx + 1
            line = self.StatementPreprocess(line).strip()
            if prevLine:
                line = prevLine + " " + line
                prevLine = ""
            if  line.endswith("=") or \
                line.endswith(")") or \
                line.count("(") != line.count(")"):
                ## incomplete statement need to be recorded and combined with next line
                prevLine = line
                continue
            else:
                for pattern in alloc_patterns:
                    if pattern in line:
                        for regex in alloc_regex:
                            if bool(re.search(regex, line)):
                                mallocVariable = self.GetMallocVariable(line)
                                for i in range(after_lines):
                                    if(idx + i < len(lines)):
                                        if bool(re.search(if_condition_regex %mallocVariable, lines[idx + i])):
                                            self.addOutput("PASS", file, lineno, line, lines[idx + i])
                                            break
                                else:
                                    self.addOutput("FAIL", file, lineno, line, "Cannot find the protection of NULL Pointer")
            prevLine = ""

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
