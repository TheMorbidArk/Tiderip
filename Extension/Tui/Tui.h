//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

extern Value getCoreClassValue(ObjModule *objModule, const char *name);

void extenTuiBind(VM *vm, ObjModule *coreModule);