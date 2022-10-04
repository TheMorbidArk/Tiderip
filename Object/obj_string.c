//
// Created by TheMorbidArk on 2022/9/11.
//

#include "obj_string.h"
#include <string.h>
#include "vm.h"
#include "utils.h"
#include "common.h"
#include <stdlib.h>

/** HashString
 * Fnv-La 算法
 * @param str 字符串
 * @param length 字符串长度
 * @return Hash Code -> Hash 值
 */
uint32_t HashString( char *str, uint32_t length )
{
	uint32_t hashCode = 2166136261, index = 0;
	while ( index < length )
	{
		hashCode ^= str[ index ];
		hashCode *= 16777619;
		index++;
	}
	return hashCode;
}

/** HashObjString
 * 为 string 对象计算哈希码并将值存储到 string->hash
 * @param objString 字符串对象
 */
void HashObjString( ObjString *objString )
{
	objString->hashCode = HashString( objString->value.start, objString->value.length );
}

/** NewObjString
 * New Object String
 * 以 str 字符串创建 ObjString 对象,允许空串 ""
 * @param vm VM 指针
 * @param str 字符串 Value
 * @param length 字符串 Str 长度
 * @return Object String 对象指针
 */
ObjString *NewObjString( VM *vm, const char *str, uint32_t length )
{
	//length 为0时 str 必为 NULL, length 不为0时 str 不为 NULL
	ASSERT( length == 0 || str != NULL, "str length don`t match str!" );

	//+1是为了结尾的'\0'
	ObjString *objString = ALLOCATE_EXTRA( vm, ObjString, length + 1 );

	// 支持空字符串: str为null,length为0
	if ( objString != NULL)
	{
		InitObjHeader( vm, &objString->objHeader, OT_STRING, vm->stringClass );
		objString->value.length = length;
		if ( length > 0 )
		{
			memcpy( objString->value.start, str, length );    // 将 str 内容 => objString->value.start
		}
		objString->value.start[ length ] = '\0';
		HashObjString( objString );
	}
	else
	{
		MEM_ERROR( "\"Allocating ObjString failed!\"" );
	}
	return objString;
}
