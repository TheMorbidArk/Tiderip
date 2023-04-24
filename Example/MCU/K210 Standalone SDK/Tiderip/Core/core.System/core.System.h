//
// Created by MorbidArk on 2023/3/16.
//

#pragma once

#include <string.h>
#include "../../include/utils.h"
#include "class.h"
#include "obj_list.h"
#include "core.h"

extern Value getCoreClassValue(ObjModule *objModule, const char *name);
extern ObjThread *loadModule(VM *vm, Value moduleName, const char *moduleCode);
extern ObjModule *getModule(VM *vm, Value moduleName);
extern char *readModule(const char *moduleName);
extern bool validateString(VM *vm, Value arg);
extern bool validateIntValue(VM *vm, double value);

void coreSystemBind(VM *vm, ObjModule *coreModule);

