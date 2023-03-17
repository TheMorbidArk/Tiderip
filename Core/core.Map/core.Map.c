//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "core.Map.h"
#include "class.h"
#include "core.h"

//objMap.new():创建map对象
static bool primMapNew(VM *vm, Value *args UNUSED)
{
    RET_OBJ(newObjMap(vm));
}

//objMap[_]:返回map[key]对应的value
static bool primMapSubscript(VM *vm, Value *args)
{
    //校验key的合法性
    if (!validateKey(vm, args[1]))
    {
        return false;  //出错了,切换线程
    }
    
    //获得map对象实例
    ObjMap *objMap = VALUE_TO_OBJMAP(args[0]);
    
    //从map中查找key(args[1])对应的value
    Value value = mapGet(objMap, args[1]);
    
    //若没有相应的key则返回NULL
    if (VALUE_IS_UNDEFINED(value))
    {
        RET_NULL;
    }
    
    RET_VALUE(value);
}

//objMap[_]=(_):map[key]=value
static bool primMapSubscriptSetter(VM *vm, Value *args)
{
    //校验key的合法性
    if (!validateKey(vm, args[1]))
    {
        return false;  //出错了,切换线程
    }
    
    //获得map对象实例
    ObjMap *objMap = VALUE_TO_OBJMAP(args[0]);
    
    //在map中将key和value关联
    //即map[key]=value
    mapSet(vm, objMap, args[1], args[2]);
    
    RET_VALUE(args[2]); //返回value
}

//objMap.addCore_(_,_):编译器编译map字面量时内部使用的,
//在map中添加(key-value)对儿并返回map自身
static bool primMapAddCore(VM *vm, Value *args)
{
    if (!validateKey(vm, args[1]))
    {
        return false;  //出错了,切换线程
    }
    
    //获得map对象实例
    ObjMap *objMap = VALUE_TO_OBJMAP(args[0]);
    
    //在map中将key和value关联
    //即map[key]=value
    mapSet(vm, objMap, args[1], args[2]);
    
    RET_VALUE(args[0]);  //返回map对象自身
}

//objMap.clear():清除map
static bool primMapClear(VM *vm, Value *args)
{
    clearMap(vm, VALUE_TO_OBJMAP(args[0]));
    RET_NULL;
}

//objMap.containsKey(_):判断map即args[0]是否包含key即args[1]
static bool primMapContainsKey(VM *vm, Value *args)
{
    if (!validateKey(vm, args[1]))
    {
        return false;  //出错了,切换线程
    }
    
    //直接去get该key,判断是否get成功
    RET_BOOL(!VALUE_IS_UNDEFINED(mapGet(VALUE_TO_OBJMAP(args[0]), args[1])));
}

//objMap.count:返回map中entry个数
static bool primMapCount(VM *vm UNUSED, Value *args)
{
    RET_NUM(VALUE_TO_OBJMAP(args[0])->count);
}

//objMap.remove(_):删除map[key] map是args[0] key是args[1]
static bool primMapRemove(VM *vm, Value *args)
{
    if (!validateKey(vm, args[1]))
    {
        return false;  //出错了,切换线程
    }
    
    RET_VALUE(removeKey(vm, VALUE_TO_OBJMAP(args[0]), args[1]));
}

//objMap.iterate_(_):迭代map中的entry,
//返回entry的索引供keyIteratorValue_和valueIteratorValue_做迭代器
static bool primMapIterate(VM *vm, Value *args)
{
    //获得map对象实例
    ObjMap *objMap = VALUE_TO_OBJMAP(args[0]);
    
    //map中若空则返回false不可迭代
    if (objMap->count == 0)
    {
        RET_FALSE;
    }
    
    //若没有传入迭代器,迭代默认是从第0个entry开始
    uint32_t index = 0;
    
    //若不是第一次迭代,传进了迭代器
    if (!VALUE_IS_NULL(args[1]))
    {
        //iter必须为整数
        if (!validateInt(vm, args[1]))
        {
            //本线程出错了,返回false是为了切换到下一线
            return false;
        }
        
        //迭代器不能小0
        if (VALUE_TO_NUM(args[1]) < 0)
        {
            RET_FALSE;
        }
        
        index = (uint32_t)VALUE_TO_NUM(args[1]);
        //迭代器不能越界
        if (index >= objMap->capacity)
        {
            RET_FALSE;
        }
        
        index++;  //更新迭代器
    }
    
    //返回下一个正在使用(有效)的entry
    while (index < objMap->capacity)
    {
        //entries是个数组, 元素是哈希槽,
        //哈希值散布在这些槽中并不连续,因此逐个判断槽位是否在用
        if (!VALUE_IS_UNDEFINED(objMap->entries[index].key))
        {
            RET_NUM(index);    //返回entry索引
        }
        index++;
    }
    
    //若没有有效的entry了就返回false,迭代结束
    RET_FALSE;
}

//objMap.keyIteratorValue_(_): key=map.keyIteratorValue(iter)
static bool primMapKeyIteratorValue(VM *vm, Value *args)
{
    ObjMap *objMap = VALUE_TO_OBJMAP(args[0]);
    
    uint32_t index = validateIndex(vm, args[1], objMap->capacity);
    if (index == UINT32_MAX)
    {
        return false;
    }
    
    Entry *entry = &objMap->entries[index];
    if (VALUE_IS_UNDEFINED(entry->key))
    {
        SET_ERROR_FALSE(vm, "invalid iterator!");
    }
    
    //返回该key
    RET_VALUE(entry->key);
}

//objMap.valueIteratorValue_(_):
//value = map.valueIteratorValue_(iter)
static bool primMapValueIteratorValue(VM *vm, Value *args)
{
    ObjMap *objMap = VALUE_TO_OBJMAP(args[0]);
    
    uint32_t index = validateIndex(vm, args[1], objMap->capacity);
    if (index == UINT32_MAX)
    {
        return false;
    }
    
    Entry *entry = &objMap->entries[index];
    if (VALUE_IS_UNDEFINED(entry->key))
    {
        SET_ERROR_FALSE(vm, "invalid iterator!");
    }
    
    //返回该key
    RET_VALUE(entry->value);
}

void coreMapBind(VM *vm, ObjModule *coreModule)
{
    vm->mapClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Map"));
    PRIM_METHOD_BIND(vm->mapClass->objHeader.class, "new()", primMapNew);
    PRIM_METHOD_BIND(vm->mapClass, "[_]", primMapSubscript);
    PRIM_METHOD_BIND(vm->mapClass, "[_]=(_)", primMapSubscriptSetter);
    PRIM_METHOD_BIND(vm->mapClass, "addCore_(_,_)", primMapAddCore);
    PRIM_METHOD_BIND(vm->mapClass, "clear()", primMapClear);
    PRIM_METHOD_BIND(vm->mapClass, "containsKey(_)", primMapContainsKey);
    PRIM_METHOD_BIND(vm->mapClass, "count", primMapCount);
    PRIM_METHOD_BIND(vm->mapClass, "remove(_)", primMapRemove);
    PRIM_METHOD_BIND(vm->mapClass, "iterate_(_)", primMapIterate);
    PRIM_METHOD_BIND(vm->mapClass, "keyIteratorValue_(_)", primMapKeyIteratorValue);
    PRIM_METHOD_BIND(vm->mapClass, "valueIteratorValue_(_)", primMapValueIteratorValue);
}