import re
import pandas as pd
from progressbar import *

class CheckRule23():
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
            line = line.strip()
            if os.path.basename(file).startswith(("_pcie", "_usb")):
                if "PLTFM_SDIO" in line:
                    self.addOutput("FAIL", file, lineno, "PLTFM_SDIO / MAC_REG_W/R should not use in this file(%s)" %line)
            elif os.path.basename(file).startswith("_sdio"):
                if "PLTFM_REG_" in line:
                    self.addOutput("FAIL", file, lineno, "PLTFM_REG_W/R / MAC_REG_W/R should not use in this file(%s)" %line)
            else:
                if "PLTFM_REG_" in line or "PLTFM_SDIO" in line:
                    self.addOutput("FAIL", file, lineno, "PLTFM_REG_W/R / PLTFM_SDIO should not use in this file(%s)" %line)

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)