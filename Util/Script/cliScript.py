# --- 数据读取 --- #

# 解析 Json 数据 -> 自动补全信息
import json

# Debug Mode
# with open('autoCompletion.json') as fileData:
#     data = json.load(fileData)
#     print('- [OK] The Cli LineNoise Messages is Read')

# RunTime
with open('../Util/Script/autoCompletion.json') as fileData:
    data = json.load(fileData)
    print('- [OK] The Cli LineNoise Messages is Read')

KeyWordList = data["KeyWord"]
Hints = data['hints']
Command = Hints['command']
Font = Hints['font']

# 删除变量,释放内存
del fileData, data, Hints

'''
# Useful Variables
    - KeyWordList -> 关键字信息
    - Command -> 自动补全信息
    - Font -> 字体数据
'''


# print("keyWordList:\n" + str(keyWordList))
# print("command:\n" + str(command))
# print("font:\n" + str(font))


# --- 数据解析 --- #

def keyword_auto(keyWordList) -> list:
    # KeyWord 数据生成

    list.sort(keyWordList)
    keyWordAutoStr = []
    # 生成code模板
    '''
    # Code Template Str ->
        -> autoStrTemplate_1 + strSupplement_1
        -> strSupplement_2 + autoStrTemplate_2 + strSupplement_1
    # format 字符串
        autoStrTemplate_1.format('[ KeyWord ]') -> ret Need Str
    '''

    autoStrTemplate_1 = 'if (buf[0] == \'{}\') '
    autoStrTemplate_2 = ' else if (buf[0] == \'{}\') '
    strSupplement_1 = '{\n'
    strSupplement_2 = '}'
    autoStrTemplate_3 = '    linenoiseAddCompletion(lc, "{}");\n'

    # 关键字排序
    '''
    # 根据关键字首字母进行分类排序
        -> 'a':['add','app','append']
        -> 'i':['is','import','if']
    # 产出变量
        -> keyWordMap : 分类排序后结果
        -> keyWordMap.keys() : 首字母列表
    '''
    keyWordMap = {}
    indexStr = keyWordList[0][0]
    indexList = []
    for i in range(len(keyWordList) + 1):
        if i == len(keyWordList):
            keyWordMap[indexStr] = indexList.copy()
            indexList.clear()
            indexStr = keyWordList[i - 1][0]
            break
        if indexStr != keyWordList[i][0]:
            keyWordMap[indexStr] = indexList.copy()
            indexList.clear()
            indexStr = keyWordList[i][0]
            indexList.append(keyWordList[i])
        else:
            indexList.append(keyWordList[i])
    del keyWordList, indexStr, indexList
    keyWordMapKeys = list(keyWordMap.keys())

    # 产出需求字符串
    index = 0
    keyWordAutoStr.append(
        autoStrTemplate_1.format(keyWordMapKeys[index]) +
        strSupplement_1
    )

    for value in list(keyWordMap.values()):
        if index != 0:
            keyWordAutoStr.append(
                strSupplement_2 +
                autoStrTemplate_2.format(keyWordMapKeys[index]) +
                strSupplement_1
            )
        for keyword in value:
            keyWordAutoStr.append(
                autoStrTemplate_3.format(keyword)
            )
        index += 1

    keyWordAutoStr.append('}\n')

    print('- [OK] The Keyword Auto-Complete information is Generated')

    return keyWordAutoStr


def hint_auto(command, font) -> list:
    """
    if (!strcasecmp(buf, "Tide")) {
        // 命令字体颜色
        *color = 35;
        // 命令字体样式
        *bold = 0;
        // 提示内容
        return " <Name> = <Value>";
    } else if (!strcasecmp(buf, "if")) {
        // 命令字体颜色
        *color = 35;
        // 命令字体样式
        *bold = 0;
        // 提示内容
        return " (Expression) {Statement}";
    """
    """
    # Code Template Str ->
        -> autoStrTemplate_1 + strSupplement_1
        -> strSupplement_2 + autoStrTemplate_2 + strSupplement_1
    # Value 字符串
        *color = 35;
        *bold = 0;
        return "...";
    """
    autoStrTemplate_1 = 'if (!strcasecmp(buf, "{}"))'
    autoStrTemplate_2 = ' else if (!strcasecmp(buf, "{}")) '
    strSupplement_1 = '{\n'
    strSupplement_2 = '}'
    autoStrTemplate_3 = '    *color = {};\n' \
                        '    *bold = {};\n' \
                        '    return "{}";\n'

    commandKeys = list(command.keys())
    commandValues = list(command.values())

    autoStrList = [autoStrTemplate_1.format(commandKeys[0]) +
                   strSupplement_1 +
                   autoStrTemplate_3.format(
                       font[0],
                       font[1],
                       commandValues[0]
                   )]

    for index in range(1, len(commandKeys)):
        autoStrList.append(
            strSupplement_2 +
            autoStrTemplate_2.format(commandKeys[index]) +
            strSupplement_1
        )
        autoStrList.append(
            autoStrTemplate_3.format(
                font[0],
                font[1],
                commandValues[index]
            )
        )
    autoStrList.append('}\n')

    print('- [OK] The Hints Command information is Generated')

    return autoStrList


resKeyWord = keyword_auto(KeyWordList)
resHint = hint_auto(Command, Font)
# 生成目标文件
# TODO 数据写入<*.inc>文件

with open("../Cli/AutoCom_KeyWord.inc", "w") as AutoCom_KeyWord:
    for i in resKeyWord:
        AutoCom_KeyWord.write(i)

with open("../Cli/AutoCom_Hint.inc", "w") as AutoCom_Hint:
    for i in resHint:
        AutoCom_Hint.write(i)

print('- [OK] The Autocomplete information Loads')
print('- [END] autoCompleteMode -')
print('--------------------------')
