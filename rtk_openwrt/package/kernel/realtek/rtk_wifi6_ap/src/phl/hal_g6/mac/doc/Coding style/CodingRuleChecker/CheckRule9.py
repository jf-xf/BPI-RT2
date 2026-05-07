import re
import pandas as pd
from progressbar import *
patchFunctionKeyword = "_patch"
ignoreFunctionPostfix = "((_[0-9]{4}[A-Za-z]{1,2})|(_be)|(_ax)|(_r[0-9]{1,2})){0,1}$"
class CheckRule9():
    def __init__(self, filesNeedToBeChecked):
        self.checkFiles = filesNeedToBeChecked
        self.output = pd.DataFrame()
        self.dicPatchFunctioninFiles = {}
        self.GetAllPatchFunction()
        self.Check()

    def addOutput(self, result, filename, functionName, string):
        if "Function Name" in self.output.columns:
            if functionName in list(self.output["Function Name"]):
                return
        row = pd.DataFrame({"Judge": [result], "Location": [filename], "Function Name":[functionName], "Message":[string]})
        self.output = pd.concat([self.output, row], ignore_index=True)

    def modifyOutput(self, result, filename, functionName, string):
        if functionName in list(self.output["Function Name"]):
            rowIndex = self.output.loc[self.output['Function Name'] == functionName].index[0]
            self.output.iloc[rowIndex]["Judge"] = result
            self.output.iloc[rowIndex]["Message"] = string
        else:
            self.addOutput(result, filename, functionName, "Find the chk_patch_* function before caller: %s" %functionName)

    def CheckOneFile(self, file):
        with open (file, "r") as f:
            lines = f.read()
            lines = lines.replace(",\n", ",").replace("\n\n", "\n")
            lines = lines.split("\n")
        functionName = ""
        isChkPatchFuncExist = False
        chkPatchFuncName = ""
        for line in lines:
            line = line.rstrip()
            if line in self.dicPatchFunctioninFiles[file] and patchFunctionKeyword in line:
                functionName = line[line.find(patchFunctionKeyword):line.find("(")]
                isChkPatchFuncExist = False
            elif line.startswith("}"):
                if functionName:
                    if isChkPatchFuncExist:
                        self.addOutput("PASS", file, functionName, "Get the matching chk_patch_* function in _patch_* function: %s" %chkPatchFuncName)
                    else:
                        self.addOutput("FAIL", file, functionName, "Cannot find the matching chk_patch_* function")
                functionName = ""
                chkPatchFuncName = ""
            elif patchFunctionKeyword in functionName and re.search(ignoreFunctionPostfix, functionName):
                patternLen = len(re.search(ignoreFunctionPostfix, functionName).group())
                subPatchFuncName = functionName[:-patternLen] if patternLen > 0 else functionName
                if ("chk" + subPatchFuncName in line or "chk_ax" + subPatchFuncName in line or "chk_be" + subPatchFuncName in line):
                    isChkPatchFuncExist = True
                    chkPatchFuncName = self.GetChkPatchFunctionName(line)
        self.showWarnForEmptyPatchFunction(file, lines)
        self.findChkPatchBeforeCaller(file, lines)

    def showWarnForEmptyPatchFunction(self, file, lines):
        for idx, line in enumerate(lines[:-3]):
            line = line.rstrip()
            if line in self.dicPatchFunctioninFiles[file] and patchFunctionKeyword in line:
                functionName = line[line.find(patchFunctionKeyword):line.find("(")]
                if lines[idx+1] == "{" and lines[idx+2].strip() == "return MACSUCCESS;" and lines[idx+3] == "}":
                    self.modifyOutput("WARN", file, functionName, "Only return MACSUCCESS in function")

    def findChkPatchBeforeCaller(self, filename, lines):
        lastIfStatement = ""
        for idx, line in enumerate(lines[:-1]):
            words = re.findall('[a-zA-Z0-9_]+', line)
            if "if" in words:
                lastIfStatement = line
                if re.search('chk((_be)|(_ax)){0,1}_patch', line) and "_patch" in lines[idx+1]:
                    chkPatchFuncName = self.GetChkPatchFunctionName(line)
                    nextLineWords = re.findall('[a-zA-Z0-9_]+', lines[idx+1])
                    patchFuncName = [word for word in nextLineWords if "_patch" in word][0]
                    subPatchFuncName = patchFuncName.replace(re.search(ignoreFunctionPostfix, patchFuncName).group(), "")
                    if subPatchFuncName in chkPatchFuncName:
                        self.modifyOutput("PASS", filename, patchFuncName, "Find the chk_patch_* function before caller: %s" %chkPatchFuncName)
            elif "else" in words and re.search('chk((_be)|(_ax)){0,1}_patch', lastIfStatement) and "_patch" in lines[idx+1]:
                chkPatchFuncName = self.GetChkPatchFunctionName(lastIfStatement)
                nextLineWords = re.findall('[a-zA-Z0-9_]+', lines[idx+1])
                patchFuncName = [word for word in nextLineWords if "_patch" in word][0]
                subPatchFuncName = patchFuncName.replace(re.search(ignoreFunctionPostfix, patchFuncName).group(), "")
                if subPatchFuncName in chkPatchFuncName:
                    self.modifyOutput("PASS", filename, patchFuncName, "Find the chk_patch_* function before caller: %s" %chkPatchFuncName)

    def GetChkPatchFunctionName(self, string):
        words  = re.findall('[a-zA-Z0-9_]+', string)
        for word in words:
            if "chk" in word:
                return word

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)

    def GetAllPatchFunction(self):
        for cf in self.checkFiles:
            self.dicPatchFunctioninFiles[cf] = []
            with open (cf, "r") as f:
                lines = f.read()
                lines = lines.replace(",\n", ",").replace("\n\n", "\n")
            functionList = self.GetPatchFuncDeclaration(lines)
            if functionList:
                self.dicPatchFunctioninFiles[cf] = functionList
        
    def GetPatchFuncDeclaration(self, lines):
        functionDef  = re.findall('.*\n{', lines)
        functionList = [func.rstrip("\n{") for func in functionDef if not func.startswith((" ", "\t")) and " _patch" in func]
        return functionList