import pandas as pd
from progressbar import *

class CheckRule28():
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
        currentOrder, tempOrder = 0, 0
        for idx, line in enumerate(lines):
            lineno = idx + 1
            line = line.rstrip()
            if line.startswith("#define"):
                tempOrder = 0
                if file.endswith(".c"):
                    self.addOutput("WARN", file, lineno, "Macro should be defined in .h (%s)" %(line))
            elif line.startswith("enum") and "{" in line:
                tempOrder = 1
            elif line.startswith(("struct", "static struct")) and "{" in line:
                tempOrder = 2
            elif file.endswith(".c"):
                if line and not line.startswith(("#", " ", "\t", "{", "}", "\n")) and line.endswith((",", ")")) and " " in line:
                    if self.isFuncDeclareLine(idx, lines):
                        tempOrder = 3
            elif file.endswith(".h"):
                if line and not line.startswith(("#", " ", "\t", "{", "}", "\n", "/", "(")) and line.endswith((",", ")")) and " " in line:
                    tempOrder = 3
            if currentOrder > tempOrder:
                self.addOutput("FAIL", file, lineno, "Please follow the order of definition. Macro->Enum->Struct->Func (%s)" %(line))
                tempOrder = currentOrder
            else:
                currentOrder = tempOrder

    def isFuncDeclareLine(self, index, lines):
        for i in range(10000):
            if lines[index+i].strip().endswith(";"):
                return False
            elif lines[index+i].strip().startswith("{"):
                return True

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
