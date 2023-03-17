//
// Created by MorbidArk on 2023/3/17.
//

#pragma once

#include "utils.h"
#include "obj_list.h"
#include "obj_range.h"

extern Value getCoreClassValue(ObjModule *objModule, const char *name);
extern bool validateInt(VM *vm, Value arg);
extern uint32_t validateIndex(VM *vm, Value index, uint32_t length);
extern uint32_t calculateRange(VM *vm, ObjRange *objRange, uint32_t *countPtr, int *directionPtr);
extern bool validateString(VM *vm, Value arg);
extern ObjString *newObjStringFromSub(VM *vm, ObjString *sourceStr, int startIndex, uint32_t count, int direction);
extern Value makeStringFromCodePoint(VM *vm, int value);
extern int findString(ObjString *haystack, ObjString *needle);
extern Value stringCodePointAt(VM *vm, ObjString *objString, uint32_t index);
void coreStringBind(VM *vm, ObjModule *coreModule);
