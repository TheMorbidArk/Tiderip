//
// Created by TheMorbidArk on 2022/9/26.
//

#include "obj_map.h"
#include "class.h"
#include "vm.h"
#include "obj_string.h"
#include "obj_range.h"

/** NewMap
 * 创建 Map 对象
 * @param vm VM 指针
 * @return ObjMap* -> ObjMap 指针 -> Map对象指针
 */
ObjMap *NewObjMap( VM *vm )
{
	ObjMap *objMap = ALLOCATE( vm, ObjMap );
	InitObjHeader( vm, &objMap->objHeader, OT_MAP, vm->mapClass );
	objMap->capacity = objMap->count = 0;
	objMap->entries = NULL;
	return objMap;
}

//计算数字的哈希码
static uint32_t HashNum( double num )
{
	Bits64 bits64;
	bits64.num = num;
	return bits64.bits32[ 0 ] ^ bits64.bits32[ 1 ];
}

//计算对象的哈希码
static uint32_t HashObj( ObjHeader *objHeader )
{
	switch ( objHeader->type )
	{
	case OT_CLASS:  //计算class的哈希值
		return HashString((( Class * )objHeader )->name->value.start, (( Class * )objHeader )->name->value.length );

	case OT_RANGE:
	{ //计算range对象哈希码
		ObjRange *objRange = ( ObjRange * )objHeader;
		return HashNum( objRange->from ) ^ HashNum( objRange->to );
	}
	case OT_STRING:  //对于字符串,直接返回其hashCode
		return (( ObjString * )objHeader )->hashCode;
	default:
		RUN_ERROR( "the hashable are objstring, objrange and class." );
	}
	return 0;
}

//根据value的类型调用相应的哈希函数
static uint32_t HashValue( Value value )
{
	switch ( value.type )
	{
	case VT_FALSE:
		return 0;
	case VT_NULL:
		return 1;
	case VT_NUM:
		return HashNum( value.num );
	case VT_TRUE:
		return 2;
	case VT_OBJ:
		return HashObj( value.objHeader );
	default:
		RUN_ERROR( "unsupport type hashed!" );
	}
	return 0;
}

/** AddEntry
 * 在entries中添加entry,如果是新的key则返回true
 * @param entries Key-Value 键值对
 * @param capacity entries 容量
 * @param key Key
 * @param value Value
 * @return bool -> 若是新的key则返回true
 */
static bool AddEntry( Entry *entries, uint32_t capacity, Value key, Value value )
{
	uint32_t index = HashValue( key ) % capacity;

	//通过开放探测法去找可用的slot
	while ( true )
	{
		//找到空闲的slot,说明目前没有此key,直接赋值返回
		if ( entries[ index ].key.type == VT_UNDEFINED )
		{
			entries[ index ].key = key;
			entries[ index ].value = value;
			return true;       //新的key就返回true
		}
		else if ( ValueIsEqual( entries[ index ].key, key ))
		{ //key已经存在,仅仅更新值就行
			entries[ index ].value = value;
			return false;    // 未增加新的key就返回false
		}

		//开放探测定址,尝试下一个slot
		index = ( index + 1 ) % capacity;
	}
}

/** ResizeMap
 * 使对象objMap的容量调整到newCapacity
 * @param vm VM 指针
 * @param objMap 需要调整的Map对象
 * @param newCapacity 调整后的容量
 */
static void ResizeMap( VM *vm, ObjMap *objMap, uint32_t newCapacity )
{
	// 创建新Entry数组
	Entry *newEntries = ALLOCATE_ARRAY( vm, Entry, newCapacity );
	for ( uint32_t index = 0; index < newCapacity; index++ )
	{
		newEntries[ index ].key = VT_TO_VALUE( VT_UNDEFINED );
		newEntries[ index ].value = VT_TO_VALUE( VT_FALSE );
	}

	// 将旧数组的值转移至新数组
	if ( objMap->capacity > 0 )
	{
		Entry *entryArr = objMap->entries;
		for ( uint32_t index = 0; index < objMap->capacity; index++ )
		{
			if ( entryArr[ index ].key.type != VT_UNDEFINED )
			{
				AddEntry( newEntries, newCapacity, entryArr[ index ].key, entryArr[ index ].value );
			}
		}
	}

	// 回收旧数组内存空间
	DEALLOCATE_ARRAY( vm, objMap->entries, objMap->count );
	objMap->entries = newEntries;
	objMap->capacity = newCapacity;

}

static Entry *FindEntry( ObjMap *objMap, Value key )
{
	if ( objMap->capacity == 0 )
	{
		return NULL;
	}

	uint32_t index = HashValue( key ) % objMap->capacity;
	Entry *entry;
	while ( true )
	{
		entry = &objMap->entries[ index ];

		// 若该slot中的entry正好是该key的entry,找到返回
		if ( ValueIsEqual( entry->key, key ))
		{
			return entry;
		}

		// key为VT_UNDEFINED且value为VT_TRUE表示探测链未断,可继续探测.
		// key为VT_UNDEFINED且value为VT_FALSE表示探测链结束,探测结束.
		if ( VALUE_IS_UNDEFINED( entry->key ) && VALUE_IS_FALSE( entry->value ))
		{
			return NULL;    //未找到
		}

		//继续向下探测
		index = ( index + 1 ) % objMap->capacity;
	}

}

//在objMap中实现key与value的关联:objMap[key]=value
void MapSet( VM *vm, ObjMap *objMap, Value key, Value value )
{
	//当容量利用率达到80%时扩容
	if ( objMap->count + 1 > objMap->capacity * MAP_LOAD_PERCENT )
	{
		uint32_t newCapacity = objMap->capacity * CAPACITY_GROW_FACTOR;
		if ( newCapacity < MIN_CAPACITY )
		{
			newCapacity = MIN_CAPACITY;
		}
		ResizeMap( vm, objMap, newCapacity );
	}

	//若创建了新的key则使objMap->count加1
	if ( AddEntry( objMap->entries, objMap->capacity, key, value ))
	{
		objMap->count++;
	}
}

//从map中查找key对应的value: map[key]
Value MapGet( ObjMap *objMap, Value key )
{
	Entry *entry = FindEntry( objMap, key );
	if ( entry == NULL)
	{
		return VT_TO_VALUE( VT_UNDEFINED );
	}
	return entry->value;
}

//回收objMap.entries占用的空间
void ClearMap( VM *vm, ObjMap *objMap )
{
	DEALLOCATE_ARRAY( vm, objMap->entries, objMap->count );
	objMap->entries = NULL;
	objMap->capacity = objMap->count = 0;
}

//删除objMap中的key,返回map[key]
Value RemoveKey( VM *vm, ObjMap *objMap, Value key )
{
	Entry *entry = FindEntry( objMap, key );

	if ( entry == NULL)
	{
		return VT_TO_VALUE( VT_NULL );
	}

//设置开放定址的伪删除
	Value value = entry->value;
	entry->key = VT_TO_VALUE( VT_UNDEFINED );
	entry->value = VT_TO_VALUE( VT_TRUE );   //值为真,伪删除

	objMap->count--;
	if ( objMap->count == 0 )
	{ //若删除该entry后map为空就回收该空间
		ClearMap( vm, objMap );
	}
	else if ( objMap->count < objMap->capacity / ( CAPACITY_GROW_FACTOR ) * MAP_LOAD_PERCENT &&
			  objMap->count > MIN_CAPACITY )
	{   //若map容量利用率太低,就缩小map空间
		uint32_t newCapacity = objMap->capacity / CAPACITY_GROW_FACTOR;
		if ( newCapacity < MIN_CAPACITY )
		{
			newCapacity = MIN_CAPACITY;
		}
		ResizeMap( vm, objMap, newCapacity );
	}

	return value;
}
