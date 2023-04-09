//
// Created by MorbidArk on 2023/3/16.
//

#pragma once

extern Value getCoreClassValue(ObjModule *objModule, const char *name);

void extenRegexBind(VM *vm, ObjModule *coreModule);