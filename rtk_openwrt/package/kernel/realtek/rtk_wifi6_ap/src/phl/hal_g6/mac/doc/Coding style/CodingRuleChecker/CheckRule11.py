import re
import pandas as pd
from progressbar import *

class CheckRule11():
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
        structName, functionName = "", ""
        for idx, line in enumerate(lines):
            lineno = idx + 1
            line = line.rstrip()
            if ("static struct" in line or "struct" in line) and line.endswith("{"):
                structName = line[line.find("struct")+7:line.find("=")-1]
                functionName = ""
                continue
            if "(" in line and not line.startswith(("#", "\n", "\t", "{", " ")):
                functionName = line[:line.find("(")]
                structName = ""
                continue
            if ("R_AX_H2CREG_CTRL" in line or "R_BE_H2CREG_CTRL" in line):
                if structName == "mac_ax_h2creg_offset h2creg_offset":
                    self.addOutput("PASS", file, lineno, "H2CREG_CTRL is in structure(h2creg_offset)")
                else:
                    self.addOutput("FAIL", file, lineno, "Please check the location of H2CREG_CTRL")
            if ("R_AX_C2HREG_CTRL" in line or "R_BE_C2HREG_CTRL" in line):
                if structName == "mac_ax_c2hreg_offset c2hreg_offset":
                    self.addOutput("PASS", file, lineno, "C2HREG_CTRL is in structure(c2hreg_offset)")
                else:
                    self.addOutput("FAIL", file, lineno, "Please check the location of H2CREG_CTRL")
            if "h2creg_offset" in line:
                words  = re.findall('[a-zA-Z0-9_]+', line)
                if "h2creg_offset" in words:
                    if "get_h2creg_offset" in functionName or "_mac_send_h2creg" in functionName:
                        self.addOutput("PASS", file, lineno, "get_h2creg_offset is in function(get_h2creg_offset)")
                    else:
                        self.addOutput("FAIL", file, lineno, "Please check the location of h2creg_offset")
                if "get_h2creg_offset" in words:
                    if "mac_ax_priv_ops" not in structName and not "_mac_send_h2creg" in functionName:
                        self.addOutput("FAIL", file, lineno, "Please check the location of get_h2creg_offset")
            if "c2hreg_offset" in line:
                words  = re.findall('[a-zA-Z0-9_]+', line)
                if "c2hreg_offset" in words:
                    if "get_c2hreg_offset" in functionName or "__recv_c2hreg" in functionName:
                        self.addOutput("PASS", file, lineno, "get_c2hreg_offset is in function(get_c2hreg_offset)")
                    else:
                        self.addOutput("FAIL", file, lineno, "Please check the location of c2hreg_offset")
                if "get_c2hreg_offset" in words:
                    if "mac_ax_priv_ops" not in structName and not "__recv_c2hreg" in functionName:
                        self.addOutput("FAIL", file, lineno, "Please check the location of get_c2hreg_offset")

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)