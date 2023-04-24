//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "core.Function.h"
#include "core.h"

//Fn.new(_):新建一个函数对象
static bool primFnNew(VM *vm, Value *args)
{
    //代码块为参数必为闭包
    if (!validateFn(vm, args[1])) return false;
    
    //直接返回函数闭包
    RET_VALUE(args[1]);
}
//绑定fn.call的重载
static void bindFnOverloadCall(VM *vm, const char *sign)
{
    uint32_t index = ensureSymbolExist(vm, &vm->allMethodNames, sign, strlen(sign));
    //构造method
    Method method = { MT_FN_CALL, { 0 }};
    bindMethod(vm, vm->fnClass, index, method);
}

void coreFunctionBind(VM *vm, ObjModule *coreModule)
{
    vm->fnClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Fn"));
    PRIM_METHOD_BIND(vm->fnClass->objHeader.class, "new(_)", primFnNew);
    
    //绑定call的重载方法
    bindFnOverloadCall(vm, "call()");
    bindFnOverloadCall(vm, "call(_)");
    bindFnOverloadCall(vm, "call(_,_)");
    bindFnOverloadCall(vm, "call(_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)");
    bindFnOverloadCall(vm, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)");
}