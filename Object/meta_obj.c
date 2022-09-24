//
// Created by TheMorbidArk on 2022/9/18.
//

#include "obj_fn.h"
#include "class.h"
#include "vm.h"
#include <string.h>


ObjModule *NewObjModule(VM *vm, const char *modName) {
    ObjModule *objModule = ALLOCATE(vm, ObjModule);
    if (objModule == NULL) {
        MEM_ERROR("allocate ObjModule failed!");
    }

    //ObjModule是元信息对象,不属于任何一个类
    InitObjHeader(vm, &objModule->objHeader, OT_MODULE, NULL);

    StringBufferInit(&objModule->moduleVarName);
    ValueBufferInit(&objModule->moduleVarValue);

    objModule->name = NULL;
    if(modName != NULL){
        objModule->name = NewObjString(vm, modName, strlen(modName));
    }

    return objModule;
}

ObjInstance *NewObjInstance(VM *vm, Class *class) {
    ObjInstance *objInstance = ALLOCATE_EXTRA(vm, ObjInstance, sizeof(Value) * class->fieldNum);

    //在此关联对象的类为参数class
    InitObjHeader(vm, &objInstance->objHeader, OT_INSTANCE, class);

    //初始化field为NULL
    uint32_t idx = 0;
    while (idx < class->fieldNum) {
        objInstance->fields[idx++] = VT_TO_VALUE(VT_NULL);
    }

    return objInstance;
}
