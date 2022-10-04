//
// Created by TheMorbidArk on 2022/9/24.
//

#include "header_obj.h"
#include "class.h"
#include "vm.h"

DEFINE_BUFFER_METHOD( Value );

/** InitObjHeader
 * 初始化对象头 -> ObjectHeader
 * @param vm VM 指针
 * @param objHeader Objct头
 * @param objType Object 类型
 * @param class Class 指针
 */
void InitObjHeader( VM *vm, ObjHeader *objHeader, ObjType objType, Class *class )
{
	objHeader->type = objType;
	objHeader->isDark = false;
	objHeader->class = class;    //设置meta类
	objHeader->next = vm->allObjects;
	vm->allObjects = objHeader;
}
