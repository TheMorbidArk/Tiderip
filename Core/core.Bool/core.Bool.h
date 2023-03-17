//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

#include "utils.h"
#include "obj_list.h"

extern Value getCoreClassValue(ObjModule *objModule, const char *name);
void coreBoolBind(VM *vm, ObjModule *coreModule);