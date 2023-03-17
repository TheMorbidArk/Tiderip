//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "core.Null.h"
#include "core.h"

//null取非
static bool primNullNot(VM *vm UNUSED, Value *args UNUSED)
{
    RET_VALUE(BOOL_TO_VALUE(true));
}

//null的字符串化
static bool primNullToString(VM *vm, Value *args UNUSED)
{
    ObjString *objString = newObjString(vm, "null", 4);
    RET_OBJ(objString);
}

void coreNullBind(VM *vm, ObjModule *coreModule)
{
    vm->nullClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Null"));
    PRIM_METHOD_BIND(vm->nullClass, "!", primNullNot);
    PRIM_METHOD_BIND(vm->nullClass, "toString", primNullToString);
}