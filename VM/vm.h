
// @description: 声明 VM 及其相关函数   //

#ifndef SPARROW_VM_H
#define SPARROW_VM_H

/* ~ INCLUDE ~ */
#include "common.h"

/* ~ VM ~ */
struct vm{
    uint32_t allocatedBytes;    // 已分配的内存量
    Parser * curParser;         // 当前的词法分析器
};

void InitVM(VM* vm);

VM* NewVM(void);

#endif //SPARROW_VM_H
