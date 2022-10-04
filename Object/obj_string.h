//
// Created by TheMorbidArk on 2022/9/24.
//

#ifndef VANTIDEL_OBJ_STRING_H
#define VANTIDEL_OBJ_STRING_H

#include "header_obj.h"

typedef struct
{
	ObjHeader objHeader;
	uint32_t hashCode;  // String -> Hash值
	CharValue value;    // Char[] => String
} ObjString;

uint32_t HashString( char *str, uint32_t length );
void HashObjString( ObjString *objString );
ObjString *NewObjString( VM *vm, const char *str, uint32_t length );

#endif //VANTIDEL_OBJ_STRING_H
