
// @description: 定义 VM 相关函数     //

/* ~ INCLUDE ~ */
#include <stdlib.h>
#include "vm.h"
#include "core.h"

/* ~ Functions ~ */
/** InitVM
 * 初始化虚拟机
 * @param vm VM 指针
 */
void InitVM( VM *vm )
{
	vm->allocatedBytes = 0;
	vm->allObjects = NULL;
	vm->curParser = NULL;
	StringBufferInit( &vm->allMethodNames );
	vm->allModules = NewObjMap( vm );
	vm->curParser = NULL;
}

/** NewVM
 * 创建 VM
 * @return VM* VM指针 -> new VM
 */
VM *NewVM( )
{
	VM *vm = ( VM * )malloc( sizeof( VM ));
	if ( vm == NULL)
	{
		MEM_ERROR( "allocate VM failed!" );
	}
	InitVM( vm );
	BuildCore( vm );
	return vm;
}
