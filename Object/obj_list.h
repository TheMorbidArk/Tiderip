//
// Created by TheMorbidArk on 2022/9/25.
//

#ifndef VANTIDEL_OBJ_LIST_H
#define VANTIDEL_OBJ_LIST_H

#include "class.h"
#include "vm.h"

typedef struct {
    ObjHeader objHeader;    // Object 头指针
    ValueBuffer elements;   // List 中元素
}ObjList;                   // List 对象

ObjList *NewObjList(VM *vm, uint32_t elementNum);
Value RemoveElement(VM *vm, ObjList *objList, uint32_t index);
void InsertElement(VM *vm, ObjList *objList, uint32_t index, Value value);

#endif //VANTIDEL_OBJ_LIST_H
