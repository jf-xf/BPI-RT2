import pandas as pd
from progressbar import *

class CheckRule7():
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
        dict = {"Lock":0, "Indirect Access": 0, "Unlock": 0}
        for idx, line in enumerate(lines):
            lineno = idx + 1
            if "(" in line and not line.startswith(("#", "\n", "\t", "{")):
                functionName = line[:line.find("(")]
                functionName = functionName[functionName.rfind(" ")+1:]
            if 'R_AX_FILTER_MODEL_ADDR' in line or \
                'R_BE_FILTER_MODEL_ADDR' in line:
                dict["Indirect Access"] += 1
            if "PLTFM_MUTEX_LOCK" in line:
                dict["Lock"] += 1
            if "PLTFM_MUTEX_UNLOCK" in line:
                dict["Unlock"] += 1
            if line.startswith("}"):
                if dict["Indirect Access"] > 0:
                    if dict["Lock"] > 0 and dict["Unlock"] > 0:
                        self.addOutput("PASS", file, lineno, functionName)
                    else:
                        self.addOutput("FAIL", file, lineno, "Cannot find PLTFM_MUTEX_LOCK/UNLOCK in %s" %functionName)
                dict = {"Lock":0, "Indirect Access": 0, "Unlock": 0}
    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)