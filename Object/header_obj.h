//
// Created by TheMorbidArk on 2022/9/24.
//

#ifndef VANTIDEL_HEADER_OBJ_H
#define VANTIDEL_HEADER_OBJ_H

#include "utils.h"

typedef enum ObjType {
    OT_CLASS,   // 此项是class类型,以下都是object类型
    OT_LIST,    // Object List 类型
    OT_MAP,     // Object Map 类型
    OT_MODULE,  // Object Module 类型 -> 模块作用域
    OT_RANGE,   // Object Range 类型 -> 步进为 1 的数字范围
    OT_STRING,  // Object String 类型
    OT_UPVALUE, // Object Upvalue 类型 -> 上值/自由变量
    OT_FUNCTION,// Object Function 类型 -> 函数
    OT_CLOSURE, // Object Closeure 类型 -> 闭包
    OT_INSTANCE,// Object Instance 类型 -> 实例，对象实例
    OT_THREAD   // Object Thread 类型 -> 线程类型
} ObjType;  // 对象类型


typedef struct objHeader {
    ObjType type;       // Object 类型
    bool isDark;	   // 对象是否可达
    Class* class;   // 对象所属的类
    struct objHeader* next;   // 用于链接所有已分配对象
} ObjHeader;	  // 对象头,用于记录元信息和垃圾回收

typedef enum {
    VT_UNDEFINED,   // 未定义
    VT_NULL,        // 空值
    VT_FALSE,       // Bool False
    VT_TRUE,        // Bool True
    VT_NUM,         // 数字
    VT_OBJ   //值为对象,指向对象头
} ValueType;     // value类型

typedef struct value{
    ValueType type;     // 类型
    union {             // 数值 -> 数字&对象
        double num;
        ObjHeader* objHeader;
    };
} Value;   // 通用的值结构

DECLARE_BUFFER_TYPE(Value);

void InitObjHeader(VM *vm, ObjHeader *objHeader, ObjType objType, Class *class);

#endif //VANTIDEL_HEADER_OBJ_H
