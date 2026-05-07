import os, glob, time, re
import pandas as pd
import argparse
from CheckRule1 import CheckRule1
from CheckRule2 import CheckRule2
from CheckRule3 import CheckRule3
from CheckRule7 import CheckRule7
from CheckRule8 import CheckRule8
from CheckRule9 import CheckRule9
from CheckRule10 import CheckRule10
from CheckRule11 import CheckRule11
from CheckRule12 import CheckRule12
from CheckRule16 import CheckRule16
from CheckRule20 import CheckRule20
from CheckRule21 import CheckRule21
from CheckRule22 import CheckRule22
from CheckRule23 import CheckRule23
from CheckRule24 import CheckRule24
from CheckRule25 import CheckRule25
from CheckRule26 import CheckRule26
from CheckRule27 import CheckRule27
from CheckRule28 import CheckRule28

def ParsingCheckRuleSetting(path):
    settingDF = pd.read_csv(path)
    settingDF = settingDF.loc[settingDF["Enable"]]
    settingDF = settingDF.fillna("")
    settingDF.loc[:,"Allowlist"] = settingDF["Allowlist"].apply(lambda x : [file.strip() for file in x.split(", ")])
    return settingDF

def DumpSheet(writer, sheetName, df):
    # Add hyperlink to Location
    if "Judge" in df.columns and sheetName != "Summary":   df = df.sort_values(["Judge", "Location"])
    if "Location" in df:
        if "Line Number" in df:
            df["Location"] = "=HYPERLINK(\"" + df["Location"] + "\", \"" + df["Location"] + ":" + df["Line Number"].astype(str) + "\")"
            del df["Line Number"]
        else:
            df["Location"] = df["Location"].apply(lambda x:'=HYPERLINK("%s", "%s")' %(x, x))
    df.to_excel(writer, sheet_name = sheetName, index = False)

    worksheet = writer.sheets[sheetName]
    for idx, col in enumerate(df.columns):
        series = df[col]
        worksheet.autofilter(0, 0, len(df), len(df.columns))
        def customLen(string):
            if "HYPERLINK" in string:
                return len(string) - string.find(", ") - len(", ''")
            else:                       return len(string)
        max_len = max((series.astype(str).map(customLen).max(), len(str(series.name)))) + 1
        worksheet.set_column(idx, idx, max_len)
        if col == "Judge":
            failFormat = writer.book.add_format({'bg_color' : 'red'})
            warnFormat = writer.book.add_format({'bg_color' : 'yellow'})
            passFormat = writer.book.add_format({'bg_color' : 'green'})
            number2Alphabet = {0:'A', 1:'B', 2:'C', 3:'D', 4:'E', 5:'F', 6:'G', 7:'H', 8:'I', 9:'J'}
            indexofJudge = number2Alphabet[idx]
            worksheet.conditional_format('%s2:%s%s'%(indexofJudge, indexofJudge, str(len(df)+1)), {'type': 'cell', 'criteria': '==', 'value': "\"FAIL\"", 'format': failFormat})
            worksheet.conditional_format('%s2:%s%s'%(indexofJudge, indexofJudge, str(len(df)+1)), {'type': 'cell', 'criteria': '==', 'value': "\"WARN\"", 'format': warnFormat})
            worksheet.conditional_format('%s2:%s%s'%(indexofJudge, indexofJudge, str(len(df)+1)), {'type': 'cell', 'criteria': '==', 'value': "\"PASS\"", 'format': passFormat})

def DumpExcel(infoDF, filename):
    writer = pd.ExcelWriter(filename, engine='xlsxwriter')
    DumpSheet(writer, "Summary", infoDF[["Check Rule Name", "Enable", "Judge", "FAIL Count", "WARN Count", "PASS Count", "Allowlist", "Number of Files", "Description", "Execution Time"]])
    for idx, row in infoDF.iterrows():
        DumpSheet(writer, row["Check Rule Name"], row["Class"].output)
    print("Report Path: "+ filename.replace(" ", "\b"))
    writer.save()

def parseArgument():
    parser = argparse.ArgumentParser()
    parser.add_argument("workspace", help="Input the halmac folder path")
    args = parser.parse_args()
    return args.workspace

if __name__ == '__main__':
    print("Start Checking coding Rule...")
    halmacFolder = parseArgument()
    checkRuleSetting = os.path.join(halmacFolder, "doc", "Coding style", "CodingRuleChecker", "CheckRuleSetting.csv")
    outputFile = os.path.join(halmacFolder, "doc", "Coding style", "CodingRuleChecker", "GenResult.xlsx")
    allDotCFile = glob.glob(os.path.join(halmacFolder, "**", "*.c"), recursive = True)
    allDotHFile = glob.glob(os.path.join(halmacFolder, "**", "*.h"), recursive = True)

    checkruleInfoDF = ParsingCheckRuleSetting(checkRuleSetting)
    checkruleInfoDF["Execution Time"] = "N/A"
    checkruleInfoDF["Class"] = "N/A"
    checkruleInfoDF["Judge"] = "N/A"
    checkruleInfoDF["FAIL Count"] = 0
    checkruleInfoDF["WARN Count"] = 0
    checkruleInfoDF["PASS Count"] = 0

    if os.path.exists(outputFile):
        os.remove(outputFile)
    for idx, row in checkruleInfoDF.iterrows():
        startTime = time.time()
        print(row["Check Rule Name"])
        checkFiles = allDotCFile
        if row["Check Rule Name"] in ["CheckRule26", "CheckRule28"]:
            checkFiles = allDotCFile + allDotHFile
        for regex in row["Allowlist"]:
            if regex:
                checkFiles = [f for f in checkFiles if not bool(re.search(regex+'$', f, re.IGNORECASE))]
        checkruleInfoDF.loc[idx, "Number of Files"] = len(checkFiles)
        checkFiles = sorted(checkFiles)
        checkRuleClass = eval(row["Check Rule Name"] + '(checkFiles)')
        checkruleInfoDF.loc[idx, "Class"] = checkRuleClass
        if not checkRuleClass.output.empty and "FAIL" in list(checkRuleClass.output["Judge"]):
            checkruleInfoDF.loc[idx, "Judge"] = "FAIL"
            checkruleInfoDF.loc[idx, "FAIL Count"] = list(checkRuleClass.output["Judge"]).count("FAIL")
            checkruleInfoDF.loc[idx, "WARN Count"] = list(checkRuleClass.output["Judge"]).count("WARN")
            checkruleInfoDF.loc[idx, "PASS Count"] = list(checkRuleClass.output["Judge"]).count("PASS")
        else:
            checkruleInfoDF.loc[idx, "Judge"] = "PASS"
            if checkRuleClass.output.empty:
                checkruleInfoDF.loc[idx, ["FAIL Count", "WARN Count", "PASS Count"]] = 0, 0, 0
            else:
                checkruleInfoDF.loc[idx, "FAIL Count"] = list(checkRuleClass.output["Judge"]).count("FAIL")
                checkruleInfoDF.loc[idx, "WARN Count"] = list(checkRuleClass.output["Judge"]).count("WARN")
                checkruleInfoDF.loc[idx, "PASS Count"] = list(checkRuleClass.output["Judge"]).count("PASS")

        checkruleInfoDF.loc[idx, "Execution Time"] = "%.1f" %(time.time() - startTime) + "s"
    DumpExcel(checkruleInfoDF, outputFile)

    with open(outputFile.replace(".xlsx", ".txt"), 'w', encoding='utf-8') as f:
        if "FAIL" in list(checkruleInfoDF["Judge"]):    f.write("FAIL")
        else:                                           f.write("PASS")