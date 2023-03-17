//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "utils.h"
#include "class.h"
#include "core.h"

#include "Test.h"

bool primTestParse(VM *vm, Value *args){
    RET_NUM(123)
}

void extenTestBind(VM *vm, ObjModule *coreModule){
    Class *testClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Test"));
    PRIM_METHOD_BIND(testClass->objHeader.class, "testFun_()", primTestParse);
}