//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

#include "utils.h"
#include "obj_list.h"

extern Value getCoreClassValue(ObjModule *objModule, const char *name);
extern uint32_t validateIndex(VM *vm, Value index, uint32_t length);
extern bool validateKey(VM *vm, Value arg);
extern bool validateInt(VM *vm, Value arg);
void coreMapBind(VM *vm, ObjModule *coreModule);