//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

#include "utils.h"
#include "obj_list.h"
#include "obj_range.h"

extern Value getCoreClassValue(ObjModule *objModule, const char *name);
extern uint32_t validateIndex(VM *vm, Value index, uint32_t length);
extern bool validateInt(VM *vm, Value arg);
extern uint32_t calculateRange(VM *vm, ObjRange *objRange, uint32_t *countPtr, int *directionPtr);
void coreListBind(VM *vm, ObjModule *coreModule);
