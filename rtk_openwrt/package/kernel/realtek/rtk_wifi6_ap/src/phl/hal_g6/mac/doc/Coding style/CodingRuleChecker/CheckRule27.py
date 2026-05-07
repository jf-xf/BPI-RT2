import pandas as pd
from progressbar import *

class CheckRule27():
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
            if 'MAC_REG_W' in line and 'MAC_REG_W_OFLD' not in line and functionName not in ["fw_pc_dbg_dump_ax", "fw_pc_dbg_dump_be"]:
                if  '0x58' in line.lower() and "DBG_SEL_FW_PROG_CNTR" in line or \
                    '0x58' in line.lower() and "0xf200f2" in line or \
                    'R_AX_DBG_CTRL' in line.lower() and "DBG_SEL_FW_PROG_CNTR" in line or \
                    'R_AX_DBG_CTRL' in line.lower() and "0xf200f2" in line:
                    self.addOutput("FAIL", file, lineno, line)

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)