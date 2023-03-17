//
// Created by MorbidArk on 2023/3/16.
//

#include <time.h>
#include "core.System.h"

//输出字符串
static void printString(const char *str)
{
    //输出到缓冲区后立即刷新
    printf("%s", str);
    fflush(stdout);
}

char *fgetsNoEndline(char *str, int n, FILE *stream)
{
    char *ret = fgets(str, n, stream);
    unsigned int l;
    if (ret)
    {
        l = strlen(str) - 1;
        if (str[l] == '\n') str[l] = '\0';
    }
    return ret;
}

//输入字符串
static const char *inputString()
{
    //输出到缓冲区后立即刷新
    char *str;
    fgetsNoEndline(str, 1024, stdin);
    fflush(stdin);
    return (const char *)str;
}

//导入模块moduleName,主要是把编译模块并加载到vm->allModules
static Value importModule(VM *vm, Value moduleName)
{
    //若已经导入则返回NULL_VAL
    if (!VALUE_IS_UNDEFINED(mapGet(vm->allModules, moduleName)))
    {
        return VT_TO_VALUE(VT_NULL);
    }
    ObjString *objString = VALUE_TO_OBJSTR(moduleName);
    const char *sourceCode = readModule(objString->value.start);
    
    ObjThread *moduleThread = loadModule(vm, moduleName, sourceCode);
    return OBJ_TO_VALUE(moduleThread);
}

//在模块moduleName中获取模块变量variableName
static Value getModuleVariable(VM *vm, Value moduleName, Value variableName)
{
    //调用本函数前模块已经被加载了
    ObjModule *objModule = getModule(vm, moduleName);
    if (objModule == NULL)
    {
        ObjString *modName = VALUE_TO_OBJSTR(moduleName);
        
        //24是下面sprintf中fmt中除%s的字符个数
        ASSERT(modName->value.length < 512 - 24, "id`s buffer not big enough!");
        char id[512] = { '\0' };
        int len = sprintf(id, "module \'%s\' is not loaded!", modName->value.start);
        vm->curThread->errorObj = OBJ_TO_VALUE(newObjString(vm, id, len));
        return VT_TO_VALUE(VT_NULL);
    }
    
    ObjString *varName = VALUE_TO_OBJSTR(variableName);
    
    //从moduleVarName中获得待导入的模块变量
    int index = getIndexFromSymbolTable(&objModule->moduleVarName,
        varName->value.start, varName->value.length);
    
    if (index == -1)
    {
        //32是下面sprintf中fmt中除%s的字符个数
        ASSERT(varName->value.length < 512 - 32, "id`s buffer not big enough!");
        ObjString *modName = VALUE_TO_OBJSTR(moduleName);
        char id[512] = { '\0' };
        int len = sprintf(id, "variable \'%s\' is not in module \'%s\'!",
            varName->value.start, modName->value.start);
        vm->curThread->errorObj = OBJ_TO_VALUE(newObjString(vm, id, len));
        return VT_TO_VALUE(VT_NULL);
    }
    
    //直接返回对应的模块变量
    return objModule->moduleVarValue.datas[index];
}

//System.clock: 返回以秒为单位的系统时钟
static bool primSystemClock(VM *vm UNUSED, Value *args UNUSED)
{
    RET_NUM((double)time(NULL));
}

//System.importModule(_): 导入并编译模块args[1],把模块挂载到vm->allModules
static bool primSystemImportModule(VM *vm, Value *args)
{
    if (!validateString(vm, args[1]))
    { //模块名为字符串
        return false;
    }
    
    //导入模块name并编译 把模块挂载到vm->allModules
    Value result = importModule(vm, args[1]);
    
    //若已经导入过则返回NULL_VAL
    if (VALUE_IS_NULL(result))
    {
        RET_NULL;
    }
    
    //若编译过程中出了问题,切换到下一线程
    if (!VALUE_IS_NULL(vm->curThread->errorObj))
    {
        return false;
    }
    
    //回收1个slot空间
    vm->curThread->esp--;
    
    ObjThread *nextThread = VALUE_TO_OBJTHREAD(result);
    nextThread->caller = vm->curThread;
    vm->curThread = nextThread;
    //返回false,vm会切换到此新加载模块的线程
    return false;
}

//System.getModuleVariable(_,_): 获取模块args[1]中的模块变量args[2]
static bool primSystemGetModuleVariable(VM *vm, Value *args)
{
    if (!validateString(vm, args[1]))
    {
        return false;
    }
    
    if (!validateString(vm, args[2]))
    {
        return false;
    }
    
    Value result = getModuleVariable(vm, args[1], args[2]);
    if (VALUE_IS_NULL(result))
    {
        //出错了,给vm返回false以切换线程
        return false;
    }
    
    RET_VALUE(result);
}

//System.writeString_(_): 输出字符串args[1]
static bool primSystemWriteString(VM *vm UNUSED, Value *args)
{
    ObjString *objString = VALUE_TO_OBJSTR(args[1]);
    ASSERT(objString->value.start[objString->value.length] == '\0', "string isn`t terminated!");
    printString(objString->value.start);
    RET_VALUE(args[1]);
}

//System.inputString_(): 输出字符串args[1]
static bool primSystemInputString(VM *vm UNUSED, Value *args UNUSED)
{
    const char *str = inputString();
    ObjString *objString = newObjString(vm, str, strlen(str));
    ASSERT(objString->value.start[objString->value.length] == '\0', "string isn`t terminated!");
    RET_VALUE(OBJ_TO_VALUE(objString));
}

//System.getRand(_,_): 返回区间内随机数
static bool primSystemGetRand(VM *vm, Value *args)
{
    
    if (!validateIntValue(vm, VALUE_IS_NUM(args[1])))
    {
        return false;
    }
    
    if (!validateIntValue(vm, VALUE_IS_NUM(args[2])))
    {
        return false;
    }
    
    int rands;
    int start = VALUE_TO_NUM(args[1]);
    int end = VALUE_TO_NUM(args[2]);
    srand((unsigned)time(NULL));
    rands = rand() % ((end - start + 1) + start);
    
    RET_NUM(rands)
    
}

void coreSystemBind(VM *vm, ObjModule *coreModule)
{
    Class *systemClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "System"));
    PRIM_METHOD_BIND(systemClass->objHeader.class, "clock", primSystemClock);
    PRIM_METHOD_BIND(systemClass->objHeader.class, "importModule(_)", primSystemImportModule);
    PRIM_METHOD_BIND(systemClass->objHeader.class, "getModuleVariable(_,_)", primSystemGetModuleVariable);
    PRIM_METHOD_BIND(systemClass->objHeader.class, "writeString_(_)", primSystemWriteString);
    PRIM_METHOD_BIND(systemClass->objHeader.class, "inputString_()", primSystemInputString);
    PRIM_METHOD_BIND(systemClass->objHeader.class, "getRand(_,_)", primSystemGetRand);
}

