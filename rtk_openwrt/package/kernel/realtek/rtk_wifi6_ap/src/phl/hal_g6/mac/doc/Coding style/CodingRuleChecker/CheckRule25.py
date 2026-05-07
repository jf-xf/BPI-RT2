import re
import pandas as pd
from progressbar import *

done_ack_hdl = {}
recv_ack_hdl = {}

class CheckRule25():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.Check()

    def addOutput(self, result, filename, lineno, Message):
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Line Number": [lineno], "Message":[Message]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def GetAckHDL(self, file):
        global done_ack_hdl
        global recv_ack_hdl
        with open (file, "r") as f:
            lines = f.readlines()

            functionName = ""
            for _, line in enumerate(lines):
                if "(" in line and not line.startswith(("#", "\n", "\t", "{")):
                    functionName = line[:line.find("(")]
                    functionName = functionName[functionName.rfind(" ")+1:]
                if functionName == "c2h_fwi_done_ack":
                    if re.search("[\w]*done_ack_hdl", line):
                        done_ack_hdl[re.search("[\w]*done_ack_hdl", line)[0]] = []
                elif functionName == "c2h_fwi_rev_ack":
                    if re.search("[\w]*rcv_ack_hdl", line):
                        recv_ack_hdl[re.search("[\w]*rcv_ack_hdl", line)[0]] = []
            for _, line in enumerate(lines):
                if "(" in line and not line.startswith(("#", "\n", "\t", "{")):
                    functionName = line[:line.find("(")]
                    functionName = functionName[functionName.rfind(" ")+1:]
                if functionName in done_ack_hdl.keys():
                    if re.search("FWCMD[\w]*", line):
                        done_ack_hdl[functionName].append(re.search("FWCMD[\w]*", line)[0])
                if functionName in recv_ack_hdl.keys():
                    if re.search("FWCMD[\w]*", line):
                        recv_ack_hdl[functionName].append(re.search("FWCMD[\w]*", line)[0])
        done_ack_hdl = [item for list in done_ack_hdl.values() for item in list]
        recv_ack_hdl = [item for list in recv_ack_hdl.values() for item in list]

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
            for idx, line in enumerate(lines):
                lineno = idx + 1
                if "(" in line and not line.startswith(("#", "\n", "\t", "{")):
                    functionName = line[:line.find("(")]
                    functionName = functionName[functionName.rfind(" ")+1:]
                if ".h2c_func" in line:
                    if re.search("FWCMD[\w]*", line):
                        h2cID = re.search("FWCMD[\w]*", line)[0]
                if ".rec_ack" in line:
                    if "1" in line:
                        if h2cID not in recv_ack_hdl:
                            self.addOutput("FAIL", file, lineno, "Cannot find recv ack handler for %s in %s" %(h2cID, functionName))
                        else:
                            self.addOutput("PASS", file, lineno, "recv ack handler was found(%s)" %(h2cID))
                if ".done_ack" in line:
                    if "1" in line:
                        if h2cID not in done_ack_hdl:
                            self.addOutput("FAIL", file, lineno, "Cannot find done ack handler for %s in %s" %(h2cID, functionName))
                        else:
                            self.addOutput("PASS", file, lineno, "done ack handler was found(%s)" %(h2cID))

    def Check(self):
        progress = ProgressBar()
        for cf in self.checkFiles:
            if cf.endswith("fwcmd.c"):
                self.GetAckHDL(cf)
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
