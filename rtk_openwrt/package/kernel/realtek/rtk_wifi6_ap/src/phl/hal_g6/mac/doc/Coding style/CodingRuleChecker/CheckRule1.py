import re
import pandas as pd
from progressbar import *
skipLineStartPattern = ("#define", "static u8", "case ", "* ", "/*", "//")
skipLineEndPattern = ("? 1 : 0;")

class CheckRule1():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.Check()

    def addOutput(self, result, filename, lineno, Message, word):
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Line Number": [lineno], "Message":[Message], "Word":[word]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
        print(file)
        inSpecialZone = False
        for idx, line in enumerate(lines):
            lineno = idx + 1
            line = line.strip()
            if line.startswith(skipLineStartPattern) or line.endswith(skipLineEndPattern):
                continue
            if "struct " in line or "otp_sec_dis_zone_map_v01" in line or "otp_secure_zone_map_v01" in line or "enum " in line:
                inSpecialZone = True
            elif inSpecialZone and line.startswith("}"):
                inSpecialZone = False
            if not inSpecialZone:
                if bool(re.search('[0-9]+', line)):
                    replacedLine = line.replace(r"0x%X", "").replace(r"0x%x", "")
                    words =  re.findall('[a-zA-Z0-9_]+', replacedLine)
                    for word in words:
                        if bool(re.match('^[0-9][0-9x]*', word, re.IGNORECASE)):
                            self.addOutput("FAIL", file, lineno, line, word)

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)