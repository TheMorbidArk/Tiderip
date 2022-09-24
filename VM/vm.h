
// @description: 声明 VM 及其相关函数   //

#ifndef SPARROW_VM_H
#define SPARROW_VM_H

/* ~ INCLUDE ~ */
#include "common.h"
#include "class.h"
#include "parser.h"

/* ~ VM ~ */
struct vm{
    Class* stringClass;
    Class* fnClass;
    uint32_t allocatedBytes;  //累计已分配的内存量
    struct objHeader *allObjects;    //所有已分配对象链表
    Parser* curParser;  //当前词法分析器
};

void InitVM(VM* vm);

VM* NewVM(void);

#endif //SPARROW_VM_H
