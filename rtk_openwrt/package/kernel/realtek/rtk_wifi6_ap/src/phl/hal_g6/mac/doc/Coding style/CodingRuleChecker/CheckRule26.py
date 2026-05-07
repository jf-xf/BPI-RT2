import re
import pandas as pd
from progressbar import *
typeOrder = ["enum", "struct", "union", "u64", "u32", "u16", "u8"]

class CheckRule26():
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
        inStruct = False
        currentOrder = 0
        for idx, line in enumerate(lines):
            lineno = idx + 1
            line = line[:line.find(";")]
            if line.startswith(("struct", "static struct")) and "{" in line:
                inStruct = line[line.find("struct ")+7:line.find(" {")]
                alignment = 0
                continue
            if inStruct:
                if line[1:].rstrip().startswith(tuple(typeOrder)):
                    line_copy = line.strip()
                    line_copy = line_copy[:line_copy.find(" ")]
                    if "(" in line_copy:
                        line_copy = line_copy[:line_copy.find("(")]
                    type = line_copy
                    ori_quotient = alignment//32
                    if ":" in line:
                        alignment += int(line[line.find(":")+1:].strip())
                    elif "u16 *" in line or "u8 *" in line or "u16 (*" in line or "u8 (*" in line: alignment += 32
                    elif type == "u64": alignment += 64
                    elif type == "u32": alignment += 32
                    elif type == "u16": alignment += 16
                    elif type == "u8":  alignment += 8
                    if typeOrder.index(type) < currentOrder:
                        if type in ["enum", "struct", "union"]:
                            self.addOutput("FAIL", file, lineno, "Please check the variable order in struct(%s). ENUM->STRUCT->UNION" %(inStruct))
                        if alignment//32 > ori_quotient and alignment%32 != 0:
                            self.addOutput("FAIL", file, lineno, "Struct(%s) variable should follow 4 bytes alignment" %(inStruct))
                            inStruct = False
                    if "[" in line:
                        alignment = 0
                    currentOrder = typeOrder.index(type)
                if line.startswith("}"):
                    currentOrder = 0
                    self.addOutput("PASS", file, lineno, "Struct(%s) variable follow 4 byte alignment" %(inStruct))
                    inStruct = False

    def Check(self):
        progress = ProgressBar()
        for cf in progress(self.checkFiles):
            self.CheckOneFile(cf)
