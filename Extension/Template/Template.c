// @description:                           //

#include <string.h>
#include "utils.h"
#include "class.h"
#include "core.h"

#include "Template.h"

bool primTemplate(VM *vm, Value *args){
    printf("example No.2 in C");
    RET_NULL
}

void extenTemplateBind(VM *vm, ObjModule *coreModule){
    Class *templatClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Template"));
    PRIM_METHOD_BIND(templatClass->objHeader.class, "example_()", primTemplate);
}