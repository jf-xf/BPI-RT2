import re
import pandas as pd
from progressbar import *

class CheckRule2():
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
            if "=" in line:
                if re.search(".* = .*\(", line):
                # if line.startswith("ret = ") and "("in line:
                #    if re.search('ret =.*\(', line):
                    varname = line[:line.find("=")].strip().replace("[", "\[").replace("]", "\]")
                    funcname = line[line.find("=")+1:].strip()
                    funcname = funcname[:funcname.find("(")].strip()
                    if " " in varname or "." in varname or "->" in varname or "*" in varname:
                        continue
                    if  not funcname or \
                        " " in funcname or \
                        "*" in funcname or \
                        "!" in funcname or \
                        "&" in funcname or \
                        "pltfm_cb" in funcname or \
                        re.match('[A-Z]', funcname) or \
                        re.match('pcie_cfgspc_.*', funcname) or \
                        re.match('reg_read.*_sdio', funcname) or \
                        re.match('.*r.*_indir_sdio', funcname) or \
                        re.match('get_mac_.*_adapter', funcname) or \
                        re.match('get_hv.*_ops', funcname) or \
                        funcname in ["sizeof", "vir2phy", "shift_mask", "le32_to_cpu", "cpu_to_le32", "reg_chk_sdio", "get_priv", "get_mac_ax_priv_size", "get_bp_idx"]:
                        continue
                    nextlineidx = idx
                    while not lines[nextlineidx].strip().endswith(";"):
                        nextlineidx += 1
                    nextlineidx += 1
                    while True:
                        lines[nextlineidx] = lines[nextlineidx].strip()
                        if not lines[nextlineidx] or lines[nextlineidx].strip().startswith(("//", "/*", "PLTFM_MSG_")):
                            nextlineidx += 1
                            continue
                        if lines[nextlineidx] in ["}"]:
                            nextlineidx += 1
                            continue
                        break
                    if  re.match(".*if.*%s.*" %varname, lines[nextlineidx]) or \
                        re.match(".*return %s.*" %varname, lines[nextlineidx]):
                        self.addOutput("PASS", file, lineno, "%s" %lines[nextlineidx])
                    else:
                        if varname == "ret": self.addOutput("WARN", file, lineno, "%s, Function return value should be judged. Next line:%s" %(line, lines[nextlineidx].strip()))
                        else:                self.addOutput("FAIL", file, lineno, "%s, Function return value should be judged. Next line:%s" %(line, lines[nextlineidx].strip()))
    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)