
// @description: 声明 VM 及其相关函数   //

#ifndef SPARROW_VM_H
#define SPARROW_VM_H

/* ~ INCLUDE ~ */
#include "common.h"
#include "class.h"
#include "obj_map.h"
// #include "obj_thread.h"
#include "parser.h"

/* ~ VM ~ */
typedef enum vmResult {
    VM_RESULT_SUCCESS,
    VM_RESULT_ERROR
} VMResult;   //虚拟机执行结果
//如果执行无误,可以将字符码输出到文件缓存,避免下次重新编译

struct vm {
    Class* classOfClass;
    Class* objectClass;
    Class* stringClass;
    Class* mapClass;
    Class* rangeClass;
    Class* listClass;
    Class* nullClass;
    Class* boolClass;
    Class* numClass;
    Class* fnClass;
    Class* threadClass;
    uint32_t allocatedBytes;  //累计已分配的内存量
    ObjHeader* allObjects;  //所有已分配对象链表
    SymbolTable allMethodNames;    //(所有)类的方法名
    ObjMap* allModules;
    ObjThread* curThread;   //当前正在执行的线程
    Parser* curParser;  //当前词法分析器
};

void InitVM(VM* vm);

VM* NewVM(void);

#endif //SPARROW_VM_H
