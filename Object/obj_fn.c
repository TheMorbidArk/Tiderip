//
// Created by TheMorbidArk on 2022/9/18.
//

#include "meta_obj.h"
#include "class.h"
#include "vm.h"


/** NewObjFn
 * 创建一个空函数
 * @param vm VM指针
 * @param objModule 所属模块
 * @param slotNum 栈帧使用数
 * @return ObjFn* 函数对象指针
 */
ObjFn *NewObjFn(VM *vm, ObjModule *objModule, uint32_t maxStackSlotUsedNum) {
    ObjFn *objFn = ALLOCATE(vm, ObjFn);
    if (objFn == NULL) {
        MEM_ERROR("allocate ObjFn failed!");
    }
    InitObjHeader(vm, &objFn->objHeader, OT_FUNCTION, vm->fnClass);
    ByteBufferInit(&objFn->instrStream);
    ValueBufferInit(&objFn->constants);
    objFn->module = objModule;
    objFn->maxStackSlotUsedNum = maxStackSlotUsedNum;
    objFn->upvalueNum = objFn->argNum = 0;
#ifdef DEBUG
    objFn->debug = ALLOCATE(vm, FnDebug);
    objFn->debug->fnName = NULL;
    IntBufferInit(&objFn->debug->lineNo);
#endif
    return objFn;
}

/** NewObjClosure
 * 以函数fn创建一个闭包
 * @param vm VM指针
 * @param objFn ObjFn指针
 * @return ObjClosure* 闭包对象指针
 */
ObjClosure *NewObjClosure(VM *vm, ObjFn *objFn) {
    ObjClosure *objClosure = ALLOCATE_EXTRA(vm, ObjClosure, sizeof(ObjUpvalue *) * objFn->upvalueNum);
    InitObjHeader(vm, &objClosure->objHeader, OT_CLOSURE, vm->fnClass);
    objClosure->fn = objFn;

    // 清除upvalue数组 以避免在填充upvalue数组之前触发GC
    uint32_t index = 0;
    while (index < objFn->upvalueNum) {
        objClosure->upvalues[index] = NULL;
        index++;
    }

    return objClosure;
}

/** NewObjUpvalue
 * 创建upvalue对象
 * @param vm VM指针
 * @param localVarPtr 指向upvalue所关联的局部变量
 * @return ObjUpvalue* upvalue对象指针
 */
ObjUpvalue *NewObjUpvalue(VM *vm, Value *localVarPtr) {
    ObjUpvalue* objUpvalue = ALLOCATE(vm, ObjUpvalue);
    InitObjHeader(vm, &objUpvalue->objHeader, OT_UPVALUE, NULL);
    objUpvalue->localVarPtr = localVarPtr;
    objUpvalue->closedUpvalue = VT_TO_VALUE(VT_NULL);
    objUpvalue->next = NULL;
    return objUpvalue;

}
