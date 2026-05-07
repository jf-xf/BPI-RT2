import re
import pandas as pd
from progressbar import *

class CheckRule22():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.ignoreStatementBeginPattern = ("#", "PLTFM_MSG")
        self.Check()

    def addOutput(self, filename, lineno, string):
        row = pd.DataFrame({"Judge": ["FAIL"], "Location": [filename], "Line Number": [lineno], "Message":[string]})
        self.output = pd.concat([self.output, row], ignore_index=True)
    
    def StatementPreprocess(self, line):
        line = line.strip()
        if "/*" in line:
            line = line[:line.find("/*")]
        if "//" in line:
            line = line[:line.find("//")]
        words  = re.findall('[a-zA-Z0-9_]+', line)
        return words, line

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.readlines()
        for idx, line in enumerate(lines):
            lineno = idx + 1
            words, line = self.StatementPreprocess(line)    
            if not line or line.startswith(self.ignoreStatementBeginPattern):
                continue

            if "cv" in words and \
                ("==" in line or "<=" in line or ">=" in line) and \
                self.isCVinWords(words, 'C[A-Z]{1}V'):
                    self.addOutput(file, lineno, "%s" %(line.strip()))
                    
            if "is_cv" in words and \
                self.isCVinWords(words, 'C[A-Z]{1}V'):
                    self.addOutput(file, lineno, "%s" %(line.strip()))
    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)

    def isCVinWords(self, words, pattern):
        return any(filter(lambda x: re.search(pattern, x), words))