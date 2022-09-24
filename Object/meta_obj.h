//
// Created by TheMorbidArk on 2022/9/24.
//

#ifndef VANTIDEL_META_OBJ_H
#define VANTIDEL_META_OBJ_H

#include "obj_string.h"

typedef struct {
    ObjHeader  objHeader;
    SymbolTable moduleVarName;   //模块中的模块变量名
    ValueBuffer moduleVarValue;  //模块中的模块变量值
    ObjString*  name;   //模块名
} ObjModule;   //模块对象

typedef struct {
    ObjHeader objHeader;
    //具体的字段
    Value fields[0];
} ObjInstance;	//对象实例

ObjModule* NewObjModule(VM* vm, const char* modName);
ObjInstance* NewObjInstance(VM* vm, Class* class);

#endif //VANTIDEL_META_OBJ_H
