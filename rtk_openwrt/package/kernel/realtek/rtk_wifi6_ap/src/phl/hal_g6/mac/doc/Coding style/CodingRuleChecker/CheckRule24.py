import re
import pandas as pd
from progressbar import *

whiteFunctionList = ["mac_h2c_common",
                     "mac_h2c_agg_tx",
                     "__fwhdr_download",
                     "__fwhdr_download__",
                     "__sections_download",
                     "__sections_download__"]

class CheckRule24():
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
        for idx, line in enumerate(lines):
            lineno = idx + 1
            if "(" in line and not line.startswith(("#", "\n", "\t", "{")):
                functionName = line[:line.find("(")]
                functionName = functionName[functionName.rfind(" ")+1:]
            line = line.strip()
            if "h2cb_alloc" in line:
                if "h2cb_alloc" in re.findall('[a-zA-Z0-9_=>.\-\*\[\]]+', line) and functionName not in whiteFunctionList:
                    self.addOutput("FAIL", file, lineno, "h2cb should not use in this file(%s)" %line)

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
