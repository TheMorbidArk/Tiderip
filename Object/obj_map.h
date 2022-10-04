//
// Created by TheMorbidArk on 2022/9/26.
//

#ifndef VANTIDEL_OBJ_MAP_H
#define VANTIDEL_OBJ_MAP_H

#include "header_obj.h"

#define MAP_LOAD_PERCENT 0.8

typedef struct
{
	Value key;
	Value value;
} Entry;        // Key-Value 对 -> 键值对

typedef struct
{
	ObjHeader objHeader;
	uint32_t capacity;  // Entry的容量(即总数),包括已使用和未使用Entry的数量
	uint32_t count;     // map中使用的Entry的数量
	Entry *entries;     // Entry数组
} ObjMap;               // Map 对象

ObjMap *NewObjMap( VM *vm );
void MapSet( VM *vm, ObjMap *objMap, Value key, Value value );
Value MapGet( ObjMap *objMap, Value key );
void ClearMap( VM *vm, ObjMap *objMap );
Value RemoveKey( VM *vm, ObjMap *objMap, Value key );

#endif //VANTIDEL_OBJ_MAP_H
