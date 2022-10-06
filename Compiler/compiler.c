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

// 编译单元 -> 指令流
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
	int scopeDepth;        // 0 -> 没有嵌套, -1 -> 模块作用域
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

/** DefineModuleVar
 * 在模块objModule中定义名为name,值为value的模块变量
 * @param vm
 * @param objModule
 * @param name
 * @param length
 * @param value
 * @return symbolIndex 模块变量Index
 */
int DefineModuleVar( VM *vm, ObjModule *objModule, const char *name, uint32_t length, Value value )
{
	
	// 判断模块变量名是否合法
	if ( length > MAX_ID_LEN )
	{
		// 新建 [id] 存储模块变量名
		char id[MAX_ID_LEN] = { '\0' };
		memcpy( id, name, length );
		
		if ( vm->curParser != NULL)
		{   //编译源码文件
			COMPILE_ERROR( vm->curParser, "length of identifier [ %s ] should be no more than %d", id, MAX_ID_LEN );
		}
		else
		{   // 编译源码前调用,比如加载核心模块时会调用本函数
			MEM_ERROR( "length of identifier [ %s ] should be no more than %d", id, MAX_ID_LEN );
		}
		
	}
	
	// 从模块变量名中查找变量,若不存在就添加
	int symbolIndex = GetIndexFromSymbolTable( &objModule->moduleVarName, name, length );
	
	if ( symbolIndex == -1 )
	{    // 符号 name 未定义 -> 添加进moduleVar Name和 modulVarValue
		//添加变量名
		symbolIndex = AddSymbol( vm, &objModule->moduleVarName, name, length );
		//添加变量值
		ValueBufferAdd( vm, &objModule->moduleVarValue, value );
	}
	else if ( VALUE_IS_NUM( objModule->moduleVarValue.datas[ symbolIndex ] ))
	{
		// 若是预先声明的模块变量的定义,在此为其赋予正确的值
		// 即: var a; -> a = 2;
		objModule->moduleVarValue.datas[ symbolIndex ] = value;
	}
	else
	{
		symbolIndex = -1;  //已定义则返回-1,用于判断重定义
	}
	
	return symbolIndex;
}

//把opcode定义到数组opCodeSlotsUsed中
#define OPCODE_SLOTS( opCode, effect ) effect,
static const int opCodeSlotsUsed[] = {
#include "opcode.inc"
};
#undef OPCODE_SLOTS

//初始化CompileUnit
static void initCompileUnit( Parser *parser, CompileUnit *cu, CompileUnit *enclosingUnit, bool isMethod )
{
	// 初始化 CompileUnit
	parser->curCompileUnit = cu;
	cu->curParser = parser;
	cu->enclosingUnit = enclosingUnit;
	cu->curLoop = NULL;
	cu->enclosingClassBK = NULL;
	
	
	
}

//编译模块(目前是桩函数)
ObjFn *CompileModule( VM *vm, ObjModule *objModule, const char *moduleCode )
{
	;
}

