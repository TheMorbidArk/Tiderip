import os

'''
数据预处理
'''
print('- [START] PluginSystem -')

RootPath = '../Extension/'
extenScriptPath = '../Extension/exten.script.inc'

fileNames = os.listdir(RootPath)

fileNames.remove('exten.Bind.inc')
fileNames.remove('exten.script.inc')
fileNames.remove('extenHeader.h')

extenModule = fileNames.copy()

filePaths = []

for fileName in fileNames:
    filePaths.append(RootPath + fileName)

for i in range(len(fileNames)):
    fileNames[i] += '.vt'

del i, fileName

with open(extenScriptPath, "r+") as f:
    f.seek(0)
    f.truncate()  # 清空文件
del f
print('- [OK] The Basic Information of The File is Read')


def readFile(filePath, fileName):
    """
    生成Need文件
    """
    with open(filePath + '/' + fileName, encoding='utf-8') as flie:
        fileLines = flie.readlines()

    for index in range(len(fileLines)):
        count = fileLines[index].count('"')
        if count != 0:
            strlist = fileLines[index].split('"')
            fileLines[index] = ''
            for i in range(len(strlist)):
                if i == len(strlist) - 1:
                    break
                strlist[i] = strlist[i] + '\\' + '"'
                fileLines[index] += strlist[i]

    for index in range(len(fileLines)):
        fileLines[index] = '\"' + fileLines[index] + '\"'

    with open(extenScriptPath, "a", encoding='utf-8') as AutoCom_Hint:
        for fileLine in fileLines:
            strFile = repr(fileLine).lstrip('\'').rstrip('\'')
            # static Tide pattern = \\".*\\"
            for i in range(len(strFile) - 1):
                if i == len(strFile) - 1:
                    break
                if strFile[i] == '\\' and strFile[i] == strFile[i + 1]:
                    strFile = strFile[:i] + strFile[i + 1:]
            AutoCom_Hint.write(strFile)
            AutoCom_Hint.write('\n')


'''
Main for-each
'''
for fileName, filePath in zip(fileNames, filePaths):
    readFile(filePath, fileName)
with open(extenScriptPath, "a", encoding='utf-8') as AutoCom_Hint:
    AutoCom_Hint.write(';')
print('- [OK] The <exten.script.inc> build is complete')

# 生成 exten.Bind.inc
# extenModule = [ModuleName]
with open('../Extension/exten.Bind.inc', 'w', encoding='utf-8') as extenBindFile:
    for ModuleName in extenModule:
        extenBindFile.write('\t// ' + ModuleName + '类\n')
        extenBindFile.write('\texten' + ModuleName + 'Bind(vm, coreModule);\n')
print('- [OK] The <exten.Bind.inc> build is complete')

# 生成 extenHeader.h
with open('../Extension/extenHeader.h', 'w', encoding='utf-8') as extenHeaderFile:
    for ModuleName in extenModule:
        extenHeaderFile.write(
            '#include "' +
            ModuleName + '/' + ModuleName +
            '.h"\n'
        )
print('- [OK] The <extenHeader.h> build is complete')
print('- [END] PluginSystem -')
