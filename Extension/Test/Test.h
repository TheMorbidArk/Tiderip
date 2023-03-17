//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

extern Value getCoreClassValue(ObjModule *objModule, const char *name);

bool primTestParse(VM *vm, Value *args);
void extenTestBind(VM *vm, ObjModule *coreModule);