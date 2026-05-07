import pandas as pd
from progressbar import *

class CheckRule8():
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
            if ('MAC_REG_W' in line and 'MAC_REG_W_OFLD' not in line) or 'MAC_REG_R' in line:
                if  '0x0c04' in line.lower() or \
                    '0xc04' in line.lower() or \
                    '0x40000' in line.lower() or \
                    'R_AX_FILTER_MODEL_ADDR' in line or \
                    'R_BE_FILTER_MODEL_ADDR' in line or \
                    'R_AX_INDIR_ACCESS_ENTRY' in line or \
                    'R_BE_INDIR_ACCESS_ENTRY' in line:
                    self.addOutput("FAIL", file, lineno, line)

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)