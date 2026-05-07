import pandas as pd
from progressbar import *

class CheckRule16():
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
            FWDLInitRDYJudgement = False
            if "MAC_AX_FW_REG_OFLD" in line.strip():
                FWDLInitRDYJudgement = True
                for i in range(1, 10):
                    if lines[idx+i].startswith("u32"): #function declare
                        FWDLInitRDYJudgement = False
                        self.addOutput("PASS", file, lineno, "Function Delcare in FW_REG_OFLD")
                        break
                    elif not lines[idx+i].strip() or (lines[idx+i].strip().startswith(("u32", "u16", "u8")) and lines[idx+i].strip().endswith(";")):
                        continue
                    elif "#endif" in lines[idx+i]:
                        FWDLInitRDYJudgement = False
                        break
                    else:
                        break
                if "== MAC_AX_FWDL_INIT_RDY" in lines[idx + i] or "!= MAC_AX_FWDL_INIT_RDY" in lines[idx + i]:
                    FWDLInitRDYJudgement = False
                    self.addOutput("PASS", file, lineno, "Find the fwdl ready Judgement")
                if FWDLInitRDYJudgement:
                    self.addOutput("FAIL", file, lineno, "Please check the fwdl ready right after fwofld. HINT: if (adapter->sm.fwdl == MAC_AX_FWDL_INIT_RDY)")

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
