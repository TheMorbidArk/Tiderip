#include "core.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include "../include/utils.h"
#include "../VM/vm.h"
#include "../Object/obj_thread.h"
#include "../Compiler/compiler.h"
#include "../Object/obj_range.h"
#include "../Object/obj_map.h"
#include "unicodeUtf8.h"
/* Core 标准库 */
#include "core.System/core.System.h"
#include "core.Range/core.Range.h"
#include "core.Map/core.Map.h"
#include "core.List/core.List.h"
#include "core.String/core.String.h"
#include "core.Num/core.Num.h"
#include "core.Null/core.Null.h"
#include "core.Function/core.Function.h"
#include "core.Thread/core.Thread.h"
#include "core.Bool/core.Bool.h"
/* Exten 扩展库 */
#include "extenHeader.h"

#include <stdio.h>
#include <stdarg.h>

void extenPrintf(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    puts(buffer);
}

char *rootDir = NULL;   //根目录

//读取源代码文件
char *readFile(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        IO_ERROR("Could`t open file \"%s\".\n", path);
    }
    
    struct stat fileStat;
    stat(path, &fileStat);
    size_t fileSize = fileStat.st_size;
    char *fileContent = (char *)malloc(fileSize + 1);
    if (fileContent == NULL)
    {
        MEM_ERROR("Could`t allocate memory for reading file \"%s\".\n", path);
    }
    
    size_t numRead = fread(fileContent, sizeof(char), fileSize, file);
    if (numRead < fileSize)
    {
        IO_ERROR("Could`t read file \"%s\".\n", path);
    }
    fileContent[fileSize] = '\0';
    
    fclose(file);
    return fileContent;
}

//将数字转换为字符串
ObjString *num2str(VM *vm, double num)
{
    //nan不是一个确定的值,因此nan和nan是不相等的
    if (num != num)
    {
        return newObjString(vm, "nan", 3);
    }
    
    if (num == INFINITY)
    {
        return newObjString(vm, "infinity", 8);
    }
    
    if (num == -INFINITY)
    {
        return newObjString(vm, "-infinity", 9);
    }
    
    //以下24字节的缓冲区足以容纳双精度到字符串的转换
    char buf[24] = { '\0' };
    int len = sprintf(buf, "%.14g", num);
    return newObjString(vm, buf, len);
}

//校验arg是否为函数
bool validateFn(VM *vm, Value arg)
{
    if (VALUE_TO_OBJCLOSURE(arg))
    {
        return true;
    }
    vm->curThread->errorObj =
        OBJ_TO_VALUE(newObjString(vm, "argument must be a function!", 28));
    return false;
}

//判断arg是否为字符串
bool validateString(VM *vm, Value arg)
{
    if (VALUE_IS_OBJSTR(arg))
    {
        return true;
    }
    SET_ERROR_FALSE(vm, "argument must be string!");
}

//判断arg是否为数字
bool validateNum(VM *vm, Value arg)
{
    if (VALUE_IS_NUM(arg))
    {
        return true;
    }
    SET_ERROR_FALSE(vm, "argument must be number!");
}

//确认value是否为整数
bool validateIntValue(VM *vm, double value)
{
    if (trunc(value) == value)
    {
        return true;
    }
    SET_ERROR_FALSE(vm, "argument must be integer!");
}

//校验arg是否为整数
bool validateInt(VM *vm, Value arg)
{
    //首先得是数字
    if (!validateNum(vm, arg))
    {
        return false;
    }
    
    //再校验数值
    return validateIntValue(vm, VALUE_TO_NUM(arg));
}

//校验参数index是否是落在"[0, length)"之间的整数
static uint32_t validateIndexValue(VM *vm, double index, uint32_t length)
{
    //索引必须是数字
    if (!validateIntValue(vm, index))
    {
        return UINT32_MAX;
    }
    
    //支持负数索引,负数是从后往前索引
    //转换其对应的正数索引.如果校验失败则返回UINT32_MAX
    if (index < 0)
    {
        index += length;
    }
    
    //索引应该落在[0,length)
    if (index >= 0 && index < length)
    {
        return (uint32_t)index;
    }
    
    //执行到此说明超出范围
    vm->curThread->errorObj =
        OBJ_TO_VALUE(newObjString(vm, "index out of bound!", 19));
    return UINT32_MAX;
}

//验证index有效性
uint32_t validateIndex(VM *vm, Value index, uint32_t length)
{
    if (!validateNum(vm, index))
    {
        return UINT32_MAX;
    }
    return validateIndexValue(vm, VALUE_TO_NUM(index), length);
}

//校验key合法性
bool validateKey(VM *vm, Value arg)
{
    if (VALUE_IS_TRUE(arg) ||
        VALUE_IS_FALSE(arg) ||
        VALUE_IS_NULL(arg) ||
        VALUE_IS_NUM(arg) ||
        VALUE_IS_OBJSTR(arg) ||
        VALUE_IS_OBJRANGE(arg) ||
        VALUE_IS_CLASS(arg))
    {
        return true;
    }
    SET_ERROR_FALSE(vm, "key must be value type!");
}

//从码点value创建字符串
Value makeStringFromCodePoint(VM *vm, int value)
{
    uint32_t byteNum = getByteNumOfEncodeUtf8(value);
    ASSERT(byteNum != 0, "utf8 encode bytes should be between 1 and 4!");
    
    //+1是为了结尾的'\0'
    ObjString *objString = ALLOCATE_EXTRA(vm, ObjString, byteNum + 1);
    
    if (objString == NULL)
    {
        MEM_ERROR("allocate memory failed in runtime!");
    }
    
    initObjHeader(vm, &objString->objHeader, OT_STRING, vm->stringClass);
    objString->value.length = byteNum;
    objString->value.start[byteNum] = '\0';
    encodeUtf8((uint8_t *)objString->value.start, value);
    hashObjString(objString);
    return OBJ_TO_VALUE(objString);
}

//用索引index处的字符创建字符串对象
Value stringCodePointAt(VM *vm, ObjString *objString, uint32_t index)
{
    ASSERT(index < objString->value.length, "index out of bound!");
    int codePoint = decodeUtf8((uint8_t *)objString->value.start + index,
        objString->value.length - index);
    
    //若不是有效的utf8序列,将其处理为单个裸字符
    if (codePoint == -1)
    {
        return OBJ_TO_VALUE(newObjString(vm, &objString->value.start[index], 1));
    }
    
    return makeStringFromCodePoint(vm, codePoint);
}

//计算objRange中元素的起始索引及索引方向
uint32_t calculateRange(VM *vm, ObjRange *objRange, uint32_t *countPtr, int *directionPtr)
{
    
    uint32_t from = validateIndexValue(vm, objRange->from, *countPtr);
    if (from == UINT32_MAX)
    {
        return UINT32_MAX;
    }
    
    uint32_t to = validateIndexValue(vm, objRange->to, *countPtr);
    if (to == UINT32_MAX)
    {
        return UINT32_MAX;
    }
    
    //如果from和to为负值,经过validateIndexValue已经变成了相应的正索引
    *directionPtr = from < to ? 1 : -1;
    *countPtr = abs((int)(from - to)) + 1;
    return from;
}

//以utf8编码从source中起始为startIndex,方向为direction的count个字符创建字符串
ObjString *newObjStringFromSub(VM *vm, ObjString *sourceStr,
    int startIndex, uint32_t count, int direction)
{
    
    uint8_t *source = (uint8_t *)sourceStr->value.start;
    uint32_t totalLength = 0, idx = 0;
    
    //计算count个utf8编码的字符总共需要的字节数,后面好申请空间
    while (idx < count)
    {
        totalLength += getByteNumOfDecodeUtf8(source[startIndex + idx * direction]);
        idx++;
    }
    
    //+1是为了结尾的'\0'
    ObjString *result = ALLOCATE_EXTRA(vm, ObjString, totalLength + 1);
    
    if (result == NULL)
    {
        MEM_ERROR("allocate memory failed in runtime!");
    }
    initObjHeader(vm, &result->objHeader, OT_STRING, vm->stringClass);
    result->value.start[totalLength] = '\0';
    result->value.length = totalLength;
    
    uint8_t *dest = (uint8_t *)result->value.start;
    idx = 0;
    while (idx < count)
    {
        int index = (int)(startIndex + idx * direction);
        //解码,获取字符数据
        int codePoint = decodeUtf8(source + index, sourceStr->value.length - index);
        if (codePoint != -1)
        {
            //再将数据按照utf8编码,写入result
            dest += encodeUtf8(dest, codePoint);
        }
        idx++;
    }
    
    hashObjString(result);
    return result;
}

//使用Boyer-Moore-Horspool字符串匹配算法在haystack中查找needle,大海捞针
int findString(ObjString *haystack, ObjString *needle)
{
    //如果待查找的patten为空则为找到
    if (needle->value.length == 0)
    {
        return 0;    //返回起始下标0
    }
    
    //若待搜索的字符串比原串还长 肯定搜不到
    if (needle->value.length > haystack->value.length)
    {
        return -1;
    }
    
    //构建"bad-character shift表"以确定窗口滑动的距离
    //数组shift的值便是滑动距离
    uint32_t shift[UINT8_MAX];
    //needle中最后一个字符的下标
    uint32_t needleEnd = needle->value.length - 1;
    
    //一、 先假定"bad character"不属于needle(即pattern),
    //对于这种情况,滑动窗口跨过整个needle
    uint32_t idx = 0;
    while (idx < UINT8_MAX)
    {
        // 默认为滑过整个needle的长度
        shift[idx] = needle->value.length;
        idx++;
    }
    
    //二、假定haystack中与needle不匹配的字符在needle中之前已匹配过的位置出现过
    //就滑动窗口以使该字符与在needle中匹配该字符的最末位置对齐。
    //这里预先确定需要滑动的距离
    idx = 0;
    while (idx < needleEnd)
    {
        char c = needle->value.start[idx];
        //idx从前往后遍历needle,当needle中有重复的字符c时,
        //后面的字符c会覆盖前面的同名字符c,这保证了数组shilf中字符是needle中最末位置的字符,
        //从而保证了shilf[c]的值是needle中最末端同名字符与needle末端的偏移量
        shift[(uint8_t)c] = needleEnd - idx;
        idx++;
    }
    
    //Boyer-Moore-Horspool是从后往前比较,这是处理bad-character高效的地方,
    //因此获取needle中最后一个字符,用于同haystack的窗口中最后一个字符比较
    char lastChar = needle->value.start[needleEnd];
    
    //长度差便是滑动窗口的滑动范围
    uint32_t range = haystack->value.length - needle->value.length;
    
    //从haystack中扫描needle,寻找第1个匹配的字符 如果遍历完了就停止
    idx = 0;
    while (idx <= range)
    {
        //拿needle中最后一个字符同haystack窗口的最后一个字符比较
        //(因为Boyer-Moore-Horspool是从后往前比较), 如果匹配,看整个needle是否匹配
        char c = haystack->value.start[idx + needleEnd];
        if (lastChar == c &&
            memcmp(haystack->value.start + idx, needle->value.start, needleEnd) == 0)
        {
            //找到了就返回匹配的位置
            return (int)idx;
        }
        
        //否则就向前滑动继续下一伦比较
        idx += shift[(uint8_t)c];
    }
    
    //未找到就返回-1
    return -1;
}

//返回核心类name的value结构
Value getCoreClassValue(ObjModule *objModule, const char *name)
{
    int index = getIndexFromSymbolTable(&objModule->moduleVarName, name, strlen(name));
    if (index == -1)
    {
        char id[MAX_ID_LEN] = { '\0' };
        memcpy(id, name, strlen(name));
        RUN_ERROR("something wrong occur: missing core class \"%s\"!", id);
    }
    return objModule->moduleVarValue.datas[index];
}

//从modules中获取名为moduleName的模块
ObjModule *getModule(VM *vm, Value moduleName)
{
    Value value = mapGet(vm->allModules, moduleName);
    if (value.type == VT_UNDEFINED)
    {
        return NULL;
    }
    return (ObjModule *)(value.objHeader);
}

//载入模块moduleName并编译
ObjThread *loadModule(VM *vm, Value moduleName, const char *moduleCode)
{
    //确保模块已经载入到 vm->allModules
    //先查看是否已经导入了该模块,避免重新导入
    ObjModule *module = getModule(vm, moduleName);
    
    //若该模块未加载先将其载入,并继承核心模块中的变量
    if (module == NULL)
    {
        //创建模块并添加到vm->allModules
        ObjString *modName = VALUE_TO_OBJSTR(moduleName);
        ASSERT(modName->value.start[modName->value.length] == '\0', "string.value.start is not terminated!");
        
        module = newObjModule(vm, modName->value.start);
        mapSet(vm, vm->allModules, moduleName, OBJ_TO_VALUE(module));
        
        //继承核心模块中的变量
        ObjModule *coreModule = getModule(vm, CORE_MODULE);
        uint32_t idx = 0;
        while (idx < coreModule->moduleVarName.count)
        {
            defineModuleVar(vm, module,
                coreModule->moduleVarName.datas[idx].str,
                strlen(coreModule->moduleVarName.datas[idx].str),
                coreModule->moduleVarValue.datas[idx]);
            idx++;
        }
    }
    
    ObjFn *fn = compileModule(vm, module, moduleCode);
    ObjClosure *objClosure = newObjClosure(vm, fn);
    ObjThread *moduleThread = newObjThread(vm, objClosure);
    
    return moduleThread;
}

//获取文件全路径
static char *getFilePath(const char *moduleName)
{
    uint32_t rootDirLength = rootDir == NULL ? 0 : strlen(rootDir);
    uint32_t nameLength = strlen(moduleName);
    uint32_t pathLength = rootDirLength + nameLength + strlen(".vt");
    char *path = (char *)malloc(pathLength + 1);
    
    if (rootDir != NULL)
    {
        memmove(path, rootDir, rootDirLength);
    }
    
    memmove(path + rootDirLength, moduleName, nameLength);
    memmove(path + rootDirLength + nameLength, ".vt", 3);
    path[pathLength] = '\0';
    
    return path;
}

//读取模块
char *readModule(const char *moduleName)
{
    //1 读取内建模块  先放着
    
    //2 读取自定义模块
    char *modulePath = getFilePath(moduleName);
    char *moduleCode = readFile(modulePath);
    free(modulePath);
    
    return moduleCode;  //由主调函数将来free此空间
}

//!object: object取反,结果为false
static bool primObjectNot(VM *vm UNUSED, Value *args)
{
    RET_VALUE(VT_TO_VALUE(VT_FALSE));
}

//args[0] == args[1]: 返回object是否相等
static bool primObjectEqual(VM *vm UNUSED, Value *args)
{
    Value boolValue = BOOL_TO_VALUE(valueIsEqual(args[0], args[1]));
    RET_VALUE(boolValue);
}

//args[0] != args[1]: 返回object是否不等
static bool primObjectNotEqual(VM *vm UNUSED, Value *args)
{
    Value boolValue = BOOL_TO_VALUE(!valueIsEqual(args[0], args[1]));
    RET_VALUE(boolValue);
}

//args[0] is args[1]:类args[0]是否为类args[1]的子类
static bool primObjectIs(VM *vm, Value *args)
{
    //args[1]必须是class
    if (!VALUE_IS_CLASS(args[1]))
    {
        RUN_ERROR("argument must be class!");
    }
    
    Class *thisClass = getClassOfObj(vm, args[0]);
    Class *baseClass = (Class *)(args[1].objHeader);
    
    //有可能是多级继承,因此自下而上遍历基类链
    while (baseClass != NULL)
    {
        
        //在某一级基类找到匹配就设置返回值为VT_TRUE并返回
        if (thisClass == baseClass)
        {
            RET_VALUE(VT_TO_VALUE(VT_TRUE));
        }
        baseClass = baseClass->superClass;
    }
    
    //若未找到基类,说明不具备is_a关系
    RET_VALUE(VT_TO_VALUE(VT_FALSE));
}

//args[0].tostring: 返回args[0]所属class的名字
static bool primObjectToString(VM *vm UNUSED, Value *args)
{
    Class *class = args[0].objHeader->class;
    Value nameValue = OBJ_TO_VALUE(class->name);
    RET_VALUE(nameValue);
}

//args[0].type:返回对象args[0]的类
static bool primObjectType(VM *vm, Value *args)
{
    Class *class = getClassOfObj(vm, args[0]);
    RET_OBJ(class);
}

//args[0].name: 返回类名
static bool primClassName(VM *vm UNUSED, Value *args)
{
    RET_OBJ(VALUE_TO_CLASS(args[0])->name);
}

//args[0].supertype: 返回args[0]的基类
static bool primClassSupertype(VM *vm UNUSED, Value *args)
{
    Class *class = VALUE_TO_CLASS(args[0]);
    if (class->superClass != NULL)
    {
        RET_OBJ(class->superClass);
    }
    RET_VALUE(VT_TO_VALUE(VT_NULL));
}

//args[0].toString: 返回类名
static bool primClassToString(VM *vm UNUSED, Value *args)
{
    RET_OBJ(VALUE_TO_CLASS(args[0])->name);
}

//args[0].same(args[1], args[2]): 返回args[1]和args[2]是否相等
static bool primObjectmetaSame(VM *vm UNUSED, Value *args)
{
    Value boolValue = BOOL_TO_VALUE(valueIsEqual(args[1], args[2]));
    RET_VALUE(boolValue);
}

//执行模块
VMResult executeModule(VM *vm, Value moduleName, const char *moduleCode)
{
    ObjThread *objThread = loadModule(vm, moduleName, moduleCode);
    return executeInstruction(vm, objThread);
}

//table中查找符号symbol 找到后返回索引,否则返回-1
int getIndexFromSymbolTable(SymbolTable *table, const char *symbol, uint32_t length)
{
    ASSERT(length != 0, "length of symbol is 0!");
    uint32_t index = 0;
    while (index < table->count)
    {
        if (length == table->datas[index].length &&
            memcmp(table->datas[index].str, symbol, length) == 0)
        {
            return (int)index;
        }
        index++;
    }
    return -1;
}

//往table中添加符号symbol,返回其索引
int addSymbol(VM *vm, SymbolTable *table, const char *symbol, uint32_t length)
{
    ASSERT(length != 0, "length of symbol is 0!");
    String string;
    string.str = ALLOCATE_ARRAY(vm, char, length + 1);
    memcpy(string.str, symbol, length);
    string.str[length] = '\0';
    string.length = length;
    StringBufferAdd(vm, table, string);
    return (int)table->count - 1;
}

//确保符号已添加到符号表
int ensureSymbolExist(VM *vm, SymbolTable *table, const char *symbol, uint32_t length)
{
    int symbolIndex = getIndexFromSymbolTable(table, symbol, length);
    if (symbolIndex == -1)
    {
        return addSymbol(vm, table, symbol, length);
    }
    return symbolIndex;
}

//定义类
static Class *defineClass(VM *vm, ObjModule *objModule, const char *name)
{
    //1先创建类
    Class *class = newRawClass(vm, name, 0);
    
    //2把类做为普通变量在模块中定义
    defineModuleVar(vm, objModule, name, strlen(name), OBJ_TO_VALUE(class));
    return class;
}

//使class->methods[index]=method
void bindMethod(VM *vm, Class *class, uint32_t index, Method method)
{
    if (index >= class->methods.count)
    {
        Method emptyPad = { MT_NONE, { 0 }};
        MethodBufferFillWrite(vm, &class->methods, emptyPad, index - class->methods.count + 1);
    }
    class->methods.datas[index] = method;
}

//绑定基类
void bindSuperClass(VM *vm, Class *subClass, Class *superClass)
{
    subClass->superClass = superClass;
    
    //继承基类属性数
    subClass->fieldNum += superClass->fieldNum;
    
    //继承基类方法
    uint32_t idx = 0;
    while (idx < superClass->methods.count)
    {
        bindMethod(vm, subClass, idx, superClass->methods.datas[idx]);
        idx++;
    }
}

static const char *coreModuleCode =
#include "core.script.inc"

static const char *extenModuleCode =
#include "exten.script.inc"

//编译核心模块
void buildCore(VM *vm)
{
    
    //核心模块不需要名字,模块也允许名字为空
    ObjModule *coreModule = newObjModule(vm, NULL);
    
    //创建核心模块,录入到vm->allModules
    mapSet(vm, vm->allModules, CORE_MODULE, OBJ_TO_VALUE(coreModule));
    
    //创建object类并绑定方法
    vm->objectClass = defineClass(vm, coreModule, "object");
    PRIM_METHOD_BIND(vm->objectClass, "!", primObjectNot);
    PRIM_METHOD_BIND(vm->objectClass, "==(_)", primObjectEqual);
    PRIM_METHOD_BIND(vm->objectClass, "!=(_)", primObjectNotEqual);
    PRIM_METHOD_BIND(vm->objectClass, "is(_)", primObjectIs);
    PRIM_METHOD_BIND(vm->objectClass, "toString", primObjectToString);
    PRIM_METHOD_BIND(vm->objectClass, "type", primObjectType);
    
    //定义classOfClass类,它是所有meta类的meta类和基类
    vm->classOfClass = defineClass(vm, coreModule, "class");
    
    //objectClass是任何类的基类
    bindSuperClass(vm, vm->classOfClass, vm->objectClass);
    
    PRIM_METHOD_BIND(vm->classOfClass, "name", primClassName);
    PRIM_METHOD_BIND(vm->classOfClass, "supertype", primClassSupertype);
    PRIM_METHOD_BIND(vm->classOfClass, "toString", primClassToString);
    
    //定义object类的元信息类objectMetaclass,它无须挂载到vm
    Class *objectMetaclass = defineClass(vm, coreModule, "objectMeta");
    
    //classOfClass类是所有meta类的meta类和基类
    bindSuperClass(vm, objectMetaclass, vm->classOfClass);
    
    //类型比较
    PRIM_METHOD_BIND(objectMetaclass, "same(_,_)", primObjectmetaSame);
    
    //绑定各自的meta类
    vm->objectClass->objHeader.class = objectMetaclass;
    objectMetaclass->objHeader.class = vm->classOfClass;
    vm->classOfClass->objHeader.class = vm->classOfClass; //元信息类回路,meta类终点

    //执行核心模块
    executeModule(vm, CORE_MODULE, coreModuleCode);
    
    /* Core 标准库 */
    //Bool类
    coreBoolBind(vm, coreModule);
    //Thread类
    coreThreadBind(vm, coreModule);
    //绑定函数类
    coreFunctionBind(vm, coreModule);
    //绑定Null类的方法
    coreNullBind(vm, coreModule);
    //num类
    coreNumBind(vm, coreModule);
    //字符串类
    coreStringBind(vm, coreModule);
    //List类
    coreListBind(vm, coreModule);
    //map类
    coreMapBind(vm, coreModule);
    //range类
    coreRangeBind(vm, coreModule);
    //system类
    coreSystemBind(vm, coreModule);
    
    /* Exten 扩展库 */
    executeModule(vm, CORE_MODULE, extenModuleCode);
    
    // 扩展库绑定至coreModule
    #include "exten.Bind.inc"
    // 添加自定义函数
//    const char soure[] = "fun a(){ return \"a\" }";
//    executeModule(vm, CORE_MODULE, soure);
    
    //在核心自举过程中创建了很多ObjString对象,创建过程中需要调用initObjHeader初始化对象头,
    //使其class指向vm->stringClass.但那时的vm->stringClass尚未初始化,因此现在更正.
    ObjHeader *objHeader = vm->allObjects;
    while (objHeader != NULL)
    {
        if (objHeader->type == OT_STRING)
        {
            objHeader->class = vm->stringClass;
        }
        objHeader = objHeader->next;
    }
}
