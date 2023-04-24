//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

#include "../../include/utils.h"
#include "obj_list.h"

extern Value getCoreClassValue(ObjModule *objModule, const char *name);
extern bool validateNum(VM *vm, Value arg);
void coreRangeBind(VM *vm, ObjModule *coreModule);