//
// Created by MorbidArk on 2023/3/16.
//

#include <regex.h>
#include <string.h>
#include "utils.h"
#include "vm.h"
#include "compiler.h"
#include "obj_list.h"
#include "core.h"

#include "Regex.h"

bool primRegexParse(VM *vm, Value *args) {
    regex_t reg;    //定义一个正则实例
    ObjString *objPattern = VALUE_TO_OBJSTR(args[1]); //定义模式串
    ObjString *objBuf = VALUE_TO_OBJSTR(args[2]);   //定义待匹配串
    bool flagICASE = VALUE_IS_TRUE(args[3]);        // 是否匹配大小写
    bool flagNEWLINE = VALUE_IS_TRUE(args[4]);      // 是否识别换行符
    
    char *pattern = objPattern->value.start;
    char *buf = objBuf->value.start;
    
    // Flag 解析
    if (flagICASE == 1 && flagNEWLINE == 1) {
        regcomp(&reg, pattern, REG_EXTENDED | REG_ICASE | REG_NEWLINE);    //编译正则模式串
    } else if (flagICASE == 1 && flagNEWLINE != 1) {
        regcomp(&reg, pattern, REG_EXTENDED | REG_ICASE);    //编译正则模式串
    } else if (flagICASE != 1 && flagNEWLINE == 1) {
        regcomp(&reg, pattern, REG_EXTENDED | REG_NEWLINE);    //编译正则模式串
    } else {
        regcomp(&reg, pattern, REG_EXTENDED);    //编译正则模式串
    }
    
    const size_t nmatch = 1;    //定义匹配结果最大允许数
    regmatch_t pmatch[1];   //定义匹配结果在待匹配串中的下标范围
    int status = regexec(&reg, buf, nmatch, pmatch, 0); //匹配,status存储匹配结果(bool)
    
    int len = (pmatch[0].rm_eo - pmatch[0].rm_so);
    char retString[len];    //存储匹配结果
    
    int retEndIndex = pmatch[0].rm_eo;
    
    if (status == REG_NOMATCH) { //如果没匹配上
        RET_NULL    // 返回null
    } else if (status == 0) {  //如果匹配上了
        for (int i = pmatch[0].rm_so, j = 0; i < pmatch[0].rm_eo; i++) {    //遍历输出匹配范围的字符串
            retString[j++] = buf[i];
        }
    }
    regfree(&reg);  //释放正则表达式
    
    int proLen = (int)(strlen(buf) - retEndIndex);
    char processString[proLen];
    for(int i = retEndIndex,j = 0; i < strlen(buf); i++){
        processString[j++] = buf[i];
    }
    
    ObjList *objList = newObjList(vm, 0);
    ObjString *objString = newObjString(vm, (const char *) retString, len);
    ObjString *objProString = newObjString(vm, (const char *) processString, proLen);
    ValueBufferAdd(vm, &objList->elements, OBJ_TO_VALUE(objString));    // 匹配结果
    ValueBufferAdd(vm, &objList->elements, OBJ_TO_VALUE(objProString)); // 剩余字符串
    RET_OBJ(objList);
}