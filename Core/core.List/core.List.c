//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "core.List.h"
#include "class.h"
#include "core.h"

//objList.new():创建1个新的liist
static bool primListNew(VM *vm, Value *args UNUSED)
{
    RET_OBJ(newObjList(vm, 0));
}

//objList[_]:索引list元素
static bool primListSubscript(VM *vm, Value *args)
{
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    
    //数字和objRange都可以做索引,分别判断
    //若索引是数字,就直接索引1个字符,这是最简单的subscript
    if (VALUE_IS_NUM(args[1]))
    {
        uint32_t index = validateIndex(vm, args[1], objList->elements.count);
        if (index == UINT32_MAX)
        {
            return false;
        }
        RET_VALUE(objList->elements.datas[index]);
    }
    
    //索引要么为数字要么为ObjRange,若不是数字就应该为objRange
    if (!VALUE_IS_OBJRANGE(args[1]))
    {
        SET_ERROR_FALSE(vm, "subscript should be integer or range!");
    }
    
    int direction;
    
    uint32_t count = objList->elements.count;
    
    //返回的startIndex是objRange.from在objList.elements.data中的下标
    uint32_t startIndex = calculateRange(vm, VALUE_TO_OBJRANGE(args[1]), &count, &direction);
    
    //新建一个list 存储该range在原来list中索引的元素
    ObjList *result = newObjList(vm, count);
    uint32_t idx = 0;
    while (idx < count)
    {
        //direction为-1表示从后往前倒序赋值
        //如var l = [a,b,c,d,e,f,g]; l[5..3]表示[f,e,d]
        result->elements.datas[idx] = objList->elements.datas[startIndex + idx * direction];
        idx++;
    }
    RET_OBJ(result);
}

//objList[_]=(_):只支持数字做为subscript
static bool primListSubscriptSetter(VM *vm UNUSED, Value *args)
{
    //获取对象
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    
    //获取subscript
    uint32_t index = validateIndex(vm, args[1], objList->elements.count);
    if (index == UINT32_MAX)
    {
        return false;
    }
    
    //直接赋值
    objList->elements.datas[index] = args[2];
    
    RET_VALUE(args[2]); //把参数2做为返回值
}

//objList.add(_):直接追加到list中
static bool primListAdd(VM *vm, Value *args)
{
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    ValueBufferAdd(vm, &objList->elements, args[1]);
    RET_VALUE(args[1]); //把参数1做为返回值
}

//objList.addCore_(_):编译内部使用的,用于编译列表直接量
static bool primListAddCore(VM *vm, Value *args)
{
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    ValueBufferAdd(vm, &objList->elements, args[1]);
    RET_VALUE(args[0]); //返回列表自身
}

//objList.clear():清空list
static bool primListClear(VM *vm, Value *args)
{
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    ValueBufferClear(vm, &objList->elements);
    RET_NULL;
}

//objList.count:返回list中元素个数
static bool primListCount(VM *vm UNUSED, Value *args)
{
    RET_NUM(VALUE_TO_OBJLIST(args[0])->elements.count);
}

//objList.insert(_,_):插入元素
static bool primListInsert(VM *vm, Value *args)
{
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    //+1确保可以在最后插入
    uint32_t index = validateIndex(vm, args[1], objList->elements.count + 1);
    if (index == UINT32_MAX)
    {
        return false;
    }
    insertElement(vm, objList, index, args[2]);
    RET_VALUE(args[2]);  //参数2做为返回值
}

//objList.iterate(_):迭代list
static bool primListIterate(VM *vm, Value *args)
{
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    
    //如果是第一次迭代 迭代索引肯定为空 直接返回索引0
    if (VALUE_IS_NULL(args[1]))
    {
        if (objList->elements.count == 0)
        {
            RET_FALSE;
        }
        RET_NUM(0);
    }
    
    //确保迭代器是整数
    if (!validateInt(vm, args[1]))
    {
        return false;
    }
    
    double iter = VALUE_TO_NUM(args[1]);
    //如果迭代完了就终止
    if (iter < 0 || iter >= objList->elements.count - 1)
    {
        RET_FALSE;
    }
    
    RET_NUM(iter + 1);   //返回下一个
}

//objList.iteratorValue(_):返回迭代值
static bool primListIteratorValue(VM *vm, Value *args)
{
    //获取实例对象
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    
    uint32_t index = validateIndex(vm, args[1], objList->elements.count);
    if (index == UINT32_MAX)
    {
        return false;
    }
    
    RET_VALUE(objList->elements.datas[index]);
}

//objList.removeAt(_):删除指定位置的元素
static bool primListRemoveAt(VM *vm, Value *args)
{
    //获取实例对象
    ObjList *objList = VALUE_TO_OBJLIST(args[0]);
    
    uint32_t index = validateIndex(vm, args[1], objList->elements.count);
    if (index == UINT32_MAX)
    {
        return false;
    }
    
    RET_VALUE(removeElement(vm, objList, index));
}

void coreListBind(VM *vm, ObjModule *coreModule)
{
    vm->listClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "List"));
    PRIM_METHOD_BIND(vm->listClass->objHeader.class, "new()", primListNew);
    PRIM_METHOD_BIND(vm->listClass, "[_]", primListSubscript);
    PRIM_METHOD_BIND(vm->listClass, "[_]=(_)", primListSubscriptSetter);
    PRIM_METHOD_BIND(vm->listClass, "add(_)", primListAdd);
    PRIM_METHOD_BIND(vm->listClass, "addCore_(_)", primListAddCore);
    PRIM_METHOD_BIND(vm->listClass, "clear()", primListClear);
    PRIM_METHOD_BIND(vm->listClass, "count", primListCount);
    PRIM_METHOD_BIND(vm->listClass, "insert(_,_)", primListInsert);
    PRIM_METHOD_BIND(vm->listClass, "iterate(_)", primListIterate);
    PRIM_METHOD_BIND(vm->listClass, "iteratorValue(_)", primListIteratorValue);
    PRIM_METHOD_BIND(vm->listClass, "removeAt(_)", primListRemoveAt);
}