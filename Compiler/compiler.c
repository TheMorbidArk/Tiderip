//
// Created by TheMorbidArk on 2022/10/6.
//

#include "compiler.h"
#include "parser.h"
#include "core.h"
#include <string.h>
#if DEBUG
#include "debug.h"
#endif

struct compileUnit
{
	// 所编译的函数
	ObjFn *fn;
	//作用域中允许的局部变量的个量上限
	LocalVar localVars[MAX_LOCAL_VAR_NUM];
	//已分配的局部变量个数
	uint32_t localVarNum;
	//记录本层函数所引用的upvalue
	Upvalue upvalues[MAX_UPVALUE_NUM];
	//此项表示当前正在编译的代码所处的作用域,
	int scopeDepth;
	//当前使用的slot个数
	uint32_t stackSlotNum;
	//当前正在编译的循环层
	Loop *curLoop;
	//当前正编译的类的编译信息
	ClassBookKeep *enclosingClassBK;
	//包含此编译单元的编译单元,即直接外层
	struct compileUnit *enclosingUnit;
	//当前parser
	Parser *curParser;
};  //编译单元

int DefineModuleVar( VM *vm, ObjModule *objModule, const char *name, uint32_t length, Value value )
{

	return 0;
}
