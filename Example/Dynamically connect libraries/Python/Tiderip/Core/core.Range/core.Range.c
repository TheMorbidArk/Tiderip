//
// Created by MorbidArk on 2023/3/17.
//
#include <math.h>
#include <string.h>
#include "core.Range.h"
#include "class.h"
#include "core.h"
#include "obj_range.h"

//objRange.from: 返回range的from
static bool primRangeFrom(VM *vm UNUSED, Value *args)
{
    RET_NUM(VALUE_TO_OBJRANGE(args[0])->from);
}

//objRange.to: 返回range的to
static bool primRangeTo(VM *vm UNUSED, Value *args)
{
    RET_NUM(VALUE_TO_OBJRANGE(args[0])->to);
}

//objRange.min: 返回range中from和to较小的值
static bool primRangeMin(VM *vm UNUSED, Value *args)
{
    ObjRange *objRange = VALUE_TO_OBJRANGE(args[0]);
    RET_NUM(fmin(objRange->from, objRange->to));
}

//objRange.max: 返回range中from和to较大的值
static bool primRangeMax(VM *vm UNUSED, Value *args)
{
    ObjRange *objRange = VALUE_TO_OBJRANGE(args[0]);
    RET_NUM(fmax(objRange->from, objRange->to));
}

//objRange.iterate(_): 迭代range中的值,并不索引
static bool primRangeIterate(VM *vm, Value *args)
{
    ObjRange *objRange = VALUE_TO_OBJRANGE(args[0]);
    
    //若未提供iter说明是第一次迭代,因此返回range->from
    if (VALUE_IS_NULL(args[1]))
    {
        RET_NUM(objRange->from);
    }
    
    //迭代器必须是数字
    if (!validateNum(vm, args[1]))
    {
        return false;
    }
    
    //获得迭代器
    double iter = VALUE_TO_NUM(args[1]);
    
    //若是正方向
    if (objRange->from < objRange->to)
    {
        iter++;
        if (iter > objRange->to)
        {
            RET_FALSE;
        }
    }
    else
    {  //若是反向迭代
        iter--;
        if (iter < objRange->to)
        {
            RET_FALSE;
        }
    }
    
    RET_NUM(iter);
}

//objRange.iteratorValue(_): range的迭代就是range中从from到to之间的值
//因此直接返回迭代器就是range的值
static bool primRangeIteratorValue(VM *vm UNUSED, Value *args)
{
    ObjRange *objRange = VALUE_TO_OBJRANGE(args[0]);
    double value = VALUE_TO_NUM(args[1]);
    
    //确保args[1]在from和to的范围中
    //若是正方向
    if (objRange->from < objRange->to)
    {
        if (value >= objRange->from && value <= objRange->to)
        {
            RET_VALUE(args[1]);
        }
    }
    else
    {  //若是反向迭代
        if (value <= objRange->from && value >= objRange->to)
        {
            RET_VALUE(args[1]);
        }
    }
    RET_FALSE;
}

void coreRangeBind(VM *vm, ObjModule *coreModule)
{
    vm->rangeClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Range"));
    PRIM_METHOD_BIND(vm->rangeClass, "from", primRangeFrom);
    PRIM_METHOD_BIND(vm->rangeClass, "to", primRangeTo);
    PRIM_METHOD_BIND(vm->rangeClass, "min", primRangeMin);
    PRIM_METHOD_BIND(vm->rangeClass, "max", primRangeMax);
    PRIM_METHOD_BIND(vm->rangeClass, "iterate(_)", primRangeIterate);
    PRIM_METHOD_BIND(vm->rangeClass, "iteratorValue(_)", primRangeIteratorValue);
}