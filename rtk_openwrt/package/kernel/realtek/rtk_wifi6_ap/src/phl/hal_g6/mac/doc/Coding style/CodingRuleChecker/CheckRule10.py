import re
import pandas as pd
from progressbar import *

class CheckRule10():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.Check()

    def addOutput(self, result, filename, Message):
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Message":[Message]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
        inMACAXOPSINITFunction, inMACAXOPSEXITFunction = False, False
        initFuncList, exitFuncList = [], []
        for idx, line in enumerate(lines):
            words= re.findall('[a-zA-Z0-9_]+', line)
            if "mac_ax_ops_init" in words:
                inMACAXOPSINITFunction = True
            elif inMACAXOPSINITFunction and line.startswith("}"):
                inMACAXOPSINITFunction = False
            elif "mac_ax_ops_exit" in words:
                inMACAXOPSEXITFunction = True
            elif inMACAXOPSEXITFunction and line.startswith("}"):
                inMACAXOPSEXITFunction = False
            if inMACAXOPSINITFunction:
                if bool(re.search(".*=.*init.*", line)):
                    initFuncList.extend([w for w in words if "init" in w])
            if inMACAXOPSEXITFunction:
                if bool(re.search("(.*=.*exit.*)|(.*=.*free.*)", line)):
                    exitFuncList.extend([w for w in words if "exit" in w or "free" in w])
        initFuncList = [func.replace("init", "").strip("_") for func in initFuncList]
        exitFuncList = [func.replace("exit", "").replace("free", "").strip("_") for func in exitFuncList]
        for func in initFuncList:
            if func in exitFuncList:
                self.addOutput("PASS", file, func)
            else:
                self.addOutput("FAIL", file, "Cannot find the matching exit function for %s in mac_ax_ops_exit" %func)
            
    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
