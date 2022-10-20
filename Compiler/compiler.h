//
// Created by TheMorbidArk on 2022/10/6.
//

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "obj_fn.h"

#define MAX_LOCAL_VAR_NUM 128
#define MAX_UPVALUE_NUM 128
#define MAX_ID_LEN 128   // 变量名最大长度
#define MAX_METHOD_NAME_LEN MAX_ID_LEN
#define MAX_ARG_NUM 16
// 函数名长度+'('+n个参数+(n-1)个参数分隔符','+')'
#define MAX_SIGN_LEN MAX_METHOD_NAME_LEN + MAX_ARG_NUM * 2 + 1
#define MAX_FIELD_NUM 128

typedef struct
{
	// 如果此upvalue是直接外层函数的局部变量就置为true,
	// 否则置为false
	bool isEnclosingLocalVar;
	
	// 外层函数中局部变量的索引或者外层函数中upvalue的索引
	// 这取决于isEnclosingLocalVar的值
	uint32_t index;
} Upvalue;  // upvalue结构

typedef struct
{
	const char *name;   // 局部变量名
	uint32_t length;    // 变量名长度
	int scopeDepth;    //局部变量作用域 -> 范围深度
	
	// 表示本函数中的局部变量是否是其内层函数所引用的upvalue,
	// 当其内层函数引用此变量时,由其内层函数来设置此项为true.
	bool isUpvalue;
} LocalVar;    // 局部变量

typedef enum
{
	SIGN_CONSTRUCT, // 构造函数
	SIGN_METHOD,    // 普通方法
	SIGN_GETTER,    // getter方法
	SIGN_SETTER,    // setter方法
	SIGN_SUBSCRIPT, // getter形式的下标
	SIGN_SUBSCRIPT_SETTER   // setter形式的下标
} SignatureType;    // 方法的签名

typedef struct
{
	SignatureType type; // 签名类型
	const char *name;    // 签名
	uint32_t length;    // 签名长度
	uint32_t argNum;    // 参数个数
} Signature;            // 签名

typedef struct loop
{
	int condStartIndex;   // 循环中条件的地址
	int bodyStartIndex;   // 循环体起始地址
	int scopeDepth;  // 循环中若有break,告诉它需要退出的作用域深度
	int exitIndex;   // 循环条件不满足时跳出循环体的目标地址
	struct loop *enclosingLoop;   // 外层循环
} Loop;   //loop结构

typedef struct
{
	ObjString *name;          //类名
	SymbolTable fields;          //类属性符号表
	bool inStatic;          //若当前编译静态方法就为真
	IntBuffer instantMethods;  //实例方法
	IntBuffer staticMethods;   //静态方法
	Signature *signature;      //当前正在编译的签名
} ClassBookKeep;    //用于记录类编译时的信息

typedef struct compileUnit CompileUnit;    //提前声明 struct compileUnit 定义在 <compile.c>
typedef struct Variable Variable;

int DefineModuleVar( VM *vm, ObjModule *objModule, const char *name, uint32_t length, Value value );
ObjFn *CompileModule( VM *vm, ObjModule *objModule, const char *moduleCode );

static ClassBookKeep *GetEnclosingClassBK( CompileUnit *cu );
static void EmitLoadThis( CompileUnit *cu );
static void InfixOperator( CompileUnit *cu, bool canAssign UNUSED);
static void UnaryMethodSignature( CompileUnit *cu UNUSED, Signature *sign UNUSED);
static void UnaryOperator( CompileUnit *cu, bool canAssign UNUSED);

static uint32_t AddConstant( CompileUnit *cu, Value constant );
static void CompileProgram( CompileUnit *cu );
static void CompileStatment( CompileUnit *cu );
static void CompileLoopBody( CompileUnit *cu );
static void CompileIfStatment( CompileUnit *cu );
static void CompileWhileStatment( CompileUnit *cu );
inline static void CompileReturn( CompileUnit *cu );
inline static void CompileBreak( CompileUnit *cu );
static void CompileForStatment( CompileUnit *cu );
static void CompileMethod( CompileUnit *cu, Variable classVar, bool isStatic );
static void compileClassDefinition( CompileUnit *cu );

uint32_t GetBytesOfOperands( Byte *instrStream, Value *constants, int ip );

static void IdMethodSignature( CompileUnit *cu, Signature *sign );
static void id( CompileUnit *cu, bool canAssign );
static void StringInterpolation( CompileUnit *cu, bool canAssign UNUSED);
static void Boolean( CompileUnit *cu, bool canAssign UNUSED);
static void Null( CompileUnit *cu, bool canAssign UNUSED);
static void This( CompileUnit *cu, bool canAssign UNUSED);
static void super( CompileUnit *cu, bool canAssign );
static void Parentheses( CompileUnit *cu, bool canAssign UNUSED);
static void ListLiteral( CompileUnit *cu, bool canAssign UNUSED);
static void Subscript( CompileUnit *cu, bool canAssign );
static void SubscriptMethodSignature( CompileUnit *cu, Signature *sign );
static void CallEntry( CompileUnit *cu, bool canAssign );
static void MapLiteral( CompileUnit *cu, bool canAssign UNUSED);
static void LogicOr( CompileUnit *cu, bool canAssign UNUSED);
static void LogicAnd( CompileUnit *cu, bool canAssign UNUSED);
static void Condition( CompileUnit *cu, bool canAssign UNUSED);

#endif //_COMPILER_H_
