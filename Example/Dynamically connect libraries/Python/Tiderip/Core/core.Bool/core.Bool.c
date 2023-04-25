//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "core.Bool.h"
#include "core.h"

//返回bool的字符串形式:"true"或"false"
static bool primBoolToString(VM *vm, Value *args)
{
    ObjString *objString;
    if (VALUE_TO_BOOL(args[0]))
    {  //若为VT_TRUE
        objString = newObjString(vm, "true", 4);
    }
    else
    {
        objString = newObjString(vm, "false", 5);
    }
    RET_OBJ(objString);
}

//bool值取反
static bool primBoolNot(VM *vm UNUSED, Value *args)
{
    RET_BOOL(!VALUE_TO_BOOL(args[0]));
}

void coreBoolBind(VM *vm, ObjModule *coreModule)
{
    vm->boolClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Bool"));
    PRIM_METHOD_BIND(vm->boolClass, "toString", primBoolToString);
    PRIM_METHOD_BIND(vm->boolClass, "!", primBoolNot);
}