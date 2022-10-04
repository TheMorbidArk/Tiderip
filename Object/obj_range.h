//
// Created by TheMorbidArk on 2022/9/25.
//

#ifndef VANTIDEL_OBJ_RANGE_H
#define VANTIDEL_OBJ_RANGE_H

#include "class.h"

typedef struct
{
	ObjHeader objHeader;
	int from;
	int to;
} ObjRange;

ObjRange *NewObjRange( VM *vm, int from, int to );

#endif //VANTIDEL_OBJ_RANGE_H
