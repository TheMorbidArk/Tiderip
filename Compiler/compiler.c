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

typedef enum
{
	VAR_SCOPE_INVALID,
	VAR_SCOPE_LOCAL,    //局部变量
	VAR_SCOPE_UPVALUE,  //upvalue
	VAR_SCOPE_MODULE    //模块变量
} VarScopeType;   //标识变量作用域

typedef struct
{
	VarScopeType scopeType;   //变量的作用域
	//根据scodeType的值,
	//此索引可能指向局部变量或upvalue或模块变量
	int index; // -> VarScopeType
} Variable;

typedef enum
{
	BP_NONE,      //无绑定能力
	
	//从上往下,优先级越来越高
	BP_LOWEST,    //最低绑定能力
	BP_ASSIGN,    // =
	BP_CONDITION,   // ?:
	BP_LOGIC_OR,    // ||
	BP_LOGIC_AND,   // &&
	BP_EQUAL,      // == !=
	BP_IS,        // is
	BP_CMP,       // < > <= >=
	BP_BIT_OR,    // |
	BP_BIT_AND,   // &
	BP_BIT_SHIFT, // << >>
	BP_RANGE,       // ..
	BP_TERM,      // + -
	BP_FACTOR,      // * / %
	BP_UNARY,    // - ! ~
	BP_CALL,     // . () []
	BP_HIGHEST
} BindPower;   //定义了操作符的绑定权值,即优先级

//指示符函数指针 -> 指向各种符号的 nud 和 led 方法
typedef void (*DenotationFn)( CompileUnit *cu, bool canAssign );

//签名函数指针 -> 指向不同方法
typedef void (*methodSignatureFn)( CompileUnit *cu, Signature *signature );

typedef struct
{
	const char *id;          //符号
	//左绑定权值,不关注左边操作数的符号此值为0
	BindPower lbp;
	//字面量,变量,前缀运算符等不关注左操作数的Token调用的方法
	DenotationFn nud;
	//中缀运算符等关注左操作数的Token调用的方法
	DenotationFn led;
	//表示本符号在类中被视为一个方法,
	//为其生成一个方法签名
	methodSignatureFn methodSign;
} SymbolBindRule;   //符号绑定规则

/** DefineModuleVar
 * 在模块objModule中定义名为name,值为value的 <模块变量>
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

/** Sign2String
 * 把Signature转换为字符串,返回字符串长度
 * @param sign
 * @param buf
 * @return Str_Length
 */
static uint32_t Sign2String( Signature *sign, char *buf )
{
	uint32_t pos = 0;
	
	//复制方法名xxx
	memcpy( buf + pos, sign->name, sign->length );
	pos += sign->length;
	
	//下面单独处理方法名之后的部分
	switch ( sign->type )
	{
		//SIGN_GETTER形式:xxx,无参数,上面memcpy已完成
	case SIGN_GETTER:
		break;
		
		//SIGN_SETTER形式: xxx=(_),之前已完成xxx
	case SIGN_SETTER:
		buf[ pos++ ] = '=';
		//下面添加=右边的赋值,只支持一个赋值
		buf[ pos++ ] = '(';
		buf[ pos++ ] = '_';
		buf[ pos++ ] = ')';
		break;
		
		//SIGN_METHOD和SIGN_CONSTRUCT形式:xxx(_,...)
	case SIGN_CONSTRUCT:
	case SIGN_METHOD:
	{
		buf[ pos++ ] = '(';
		uint32_t idx = 0;
		while ( idx < sign->argNum )
		{
			buf[ pos++ ] = '_';
			buf[ pos++ ] = ',';
			idx++;
		}
		
		if ( idx == 0 )
		{ //说明没有参数
			buf[ pos++ ] = ')';
		}
		else
		{ //用rightBracket覆盖最后的','
			buf[ pos - 1 ] = ')';
		}
		break;
	}
		
		//SIGN_SUBSCRIPT形式:xxx[_,...]
	case SIGN_SUBSCRIPT:
	{
		buf[ pos++ ] = '[';
		uint32_t idx = 0;
		while ( idx < sign->argNum )
		{
			buf[ pos++ ] = '_';
			buf[ pos++ ] = ',';
			idx++;
		}
		if ( idx == 0 )
		{ //说明没有参数
			buf[ pos++ ] = ']';
		}
		else
		{ //用rightBracket覆盖最后的','
			buf[ pos - 1 ] = ']';
		}
		break;
	}
		
		//SIGN_SUBSCRIPT_SETTER形式:xxx[_,...]=(_)
	case SIGN_SUBSCRIPT_SETTER:
	{
		buf[ pos++ ] = '[';
		uint32_t idx = 0;
		//argNum包括了等号右边的1个赋值参数,
		//这里是在处理等号左边subscript中的参数列表,因此减1.
		//后面专门添加该参数
		while ( idx < sign->argNum - 1 )
		{
			buf[ pos++ ] = '_';
			buf[ pos++ ] = ',';
			idx++;
		}
		if ( idx == 0 )
		{ //说明没有参数
			buf[ pos++ ] = ']';
		}
		else
		{ //用rightBracket覆盖最后的','
			buf[ pos - 1 ] = ']';
		}
		
		//下面为等号右边的参数构造签名部分
		buf[ pos++ ] = '=';
		buf[ pos++ ] = '(';
		buf[ pos++ ] = '_';
		buf[ pos++ ] = ')';
	}
	}
	
	buf[ pos ] = '\0';
	return pos;   //返回签名串的长度
}

/** AddLocalVar
 * 添加局部变量到cu
 * @param cu
 * @param name
 * @param length
 * @return cu->localVars -> 局部变量索引(The Last)
 */
static uint32_t AddLocalVar( CompileUnit *cu, const char *name, uint32_t length )
{
	LocalVar *var = &( cu->localVars[ cu->localVarNum ] );
	var->name = name;
	var->length = length;
	var->scopeDepth = cu->scopeDepth;
	var->isUpvalue = false;
	return cu->localVarNum++;
}

/** DeclareLocalVar
 * 声明局部变量
 * @param cu
 * @param name
 * @param length
 * @return Var 的索引 Index
 */
static int DeclareLocalVar( CompileUnit *cu, const char *name, uint32_t length )
{
	if ( cu->localVarNum >= MAX_LOCAL_VAR_NUM )
	{
		COMPILE_ERROR( cu->curParser, "the max length of local variable of one scope is %d", MAX_LOCAL_VAR_NUM );
	}
	
	for ( int idx = ( int )( cu->localVarNum - 1 ); idx >= 0; idx-- )
	{
		LocalVar *var = &cu->localVars[ idx ];
		// 如果到了父作用域就退出,减少没必要的遍历 -> 剪枝
		if ( var->scopeDepth < cu->scopeDepth )
		{
			break;
		}
		// 如果重复定义则报错
		if ( var->length == length && memcmp( var->name, name, length ) == 0 )
		{
			char id[MAX_ID_LEN] = { '\0' };
			memcpy( id, name, length );
			COMPILE_ERROR( cu->curParser, "identifier \"%s\" redefinition!", id );
		}
	}
	return ( int )AddLocalVar( cu, name, length );
}

/** DeclareVariable
 * 根据作用域声明变量 -> 全局变量
 * @param cu
 * @param name
 * @param length
 * @return
 */
static int DeclareVariable( CompileUnit *cu, const char *name, uint32_t length )
{
	if ( cu->scopeDepth == -1 )
	{
		int index = DefineModuleVar( cu->curParser->vm, cu->curParser->curModule, name, length, VT_TO_VALUE( VT_NULL ));
		// 若是重复定义则报错
		if ( index == -1 )
		{
			char id[MAX_ID_LEN] = { '\0' };
			memcpy( id, name, length );
			COMPILE_ERROR( cu->curParser, "identifier \"%s\" redefinition!", id );
		}
		return index;
	}
	//否则是局部作用域,声明局部变量
	return DeclareLocalVar( cu, name, length );
}

//把opcode定义到数组opCodeSlotsUsed中
#define OPCODE_SLOTS( opCode, effect ) effect,
static const int opCodeSlotsUsed[] = {
#include "opcode.inc"
};
#undef OPCODE_SLOTS

//初始化CompileUnit
static void InitCompileUnit( Parser *parser, CompileUnit *cu, CompileUnit *enclosingUnit, bool isMethod )
{
	// 初始化 CompileUnit
	parser->curCompileUnit = cu;
	cu->curParser = parser;
	cu->enclosingUnit = enclosingUnit;
	cu->curLoop = NULL;
	cu->enclosingClassBK = NULL;
	
	if ( enclosingUnit == NULL)     // 若没有外层,说明当前属于模块作用域 -> 模块作用域初始化
	{
		// 编译代码时是从上到下从最外层的模块作用域开始,模块作用域设为-1
		cu->scopeDepth = -1;
		// 模块级作用域中没有局部变量
		cu->localVarNum = 0;
	}
	else     // 若是内层单元,属局部作用域 -> 局部作用域编译单元的初始化
	{
		if ( isMethod )     // 若是类中的方法
		{
			// 如果是类的方法就设定隐式"this"为第0个局部变量,即实例对象,
			// 它是方法(消息)的接收者.this这种特殊对象被处理为局部变量
			cu->localVars[ 0 ].name = "this";
			cu->localVars[ 0 ].length = 4;
		}
		else    // 若为普通函数
		{
			// 空出第0个局部变量,保持统一
			cu->localVars[ 0 ].name = NULL;
			cu->localVars[ 0 ].length = 0;
		}
		
		// 第0个局部变量的特殊性使其作用域为模块级别
		cu->localVars[ 0 ].scopeDepth = -1;
		cu->localVars[ 0 ].isUpvalue = false;
		cu->localVarNum = 1;  // localVars[0]被分配
		// 对于函数和方法来说,初始作用域是局部作用域
		// 0表示局部作用域的最外层
		cu->scopeDepth = 0;
	}
	//局部变量保存在栈中,初始时栈中已使用的slot数量等于局部变量的数量
	cu->stackSlotNum = cu->localVarNum;
	// 存储编译单元的指令流
	cu->fn = NewObjFn( cu->curParser->vm, cu->curParser->curModule, cu->localVarNum );
}

//往函数的指令流中写入1字节,返回其索引
static int WriteByte( CompileUnit *cu, int byte )
{
	//若在调试状态,额外在debug->lineNo中写入当前token行号
#if DEBUG
	IntBufferAdd(cu->curParser->vm,
	 &cu->fn->debug->lineNo, cu->curParser->preToken.lineNo);
#endif
	ByteBufferAdd( cu->curParser->vm,
		&cu->fn->instrStream, ( uint8_t )byte );
	return cu->fn->instrStream.count - 1;
}

//写入操作码
static void WriteOpCode( CompileUnit *cu, OpCode opCode )
{
	WriteByte( cu, opCode );
	//累计需要的运行时空间大小
	cu->stackSlotNum += opCodeSlotsUsed[ opCode ];
	if ( cu->stackSlotNum > cu->fn->maxStackSlotUsedNum )
	{
		cu->fn->maxStackSlotUsedNum = cu->stackSlotNum;
	}
}

//写入1个字节的操作数
static int WriteByteOperand( CompileUnit *cu, int operand )
{
	return WriteByte( cu, operand );
}

//写入2个字节的操作数 按大端字节序写入参数,低地址写高位,高地址写低位
inline static void WriteShortOperand( CompileUnit *cu, int operand )
{
	WriteByte( cu, ( operand >> 8 ) & 0xff ); //先写高8位
	WriteByte( cu, operand & 0xff );        //再写低8位
}

//写入操作数为1字节大小的指令
static int WriteOpCodeByteOperand( CompileUnit *cu, OpCode opCode, int operand )
{
	WriteOpCode( cu, opCode );
	return WriteByteOperand( cu, operand );
}

//写入操作数为2字节大小的指令
static void WriteOpCodeShortOperand( CompileUnit *cu, OpCode opCode, int operand )
{
	WriteOpCode( cu, opCode );
	WriteShortOperand( cu, operand );
}

//为单运算符方法创建签名
static void UnaryMethodSignature( CompileUnit *cu UNUSED, Signature *sign UNUSED)
{
	//名称部分在调用前已经完成,只修改类型
	sign->type = SIGN_GETTER;
}

//为中缀运算符创建签名
static void InfixMethodSignature( CompileUnit *cu, Signature *sign )
{
	//在类中的运算符都是方法,类型为SIGN_METHOD
	sign->type = SIGN_METHOD;
	
	// 中缀运算符只有一个参数,故初始为1
	sign->argNum = 1;
	ConsumeCurToken( cu->curParser, TOKEN_LEFT_PAREN, "expect '(' after infix operator!" );
	ConsumeCurToken( cu->curParser, TOKEN_ID, "expect variable name!" );
	DeclareVariable( cu, cu->curParser->preToken.start, cu->curParser->preToken.length );
	ConsumeCurToken( cu->curParser, TOKEN_RIGHT_PAREN, "expect ')' after parameter!" );
}

//为既做单运算符又做中缀运算符的符号方法创建签名
static void MixMethodSignature( CompileUnit *cu, Signature *sign )
{
	//假设是单运算符方法,因此默认为getter
	sign->type = SIGN_GETTER;
	
	//若后面有'(',说明其为中缀运算符,那就置其类型为SIGN_METHOD
	if ( MatchToken( cu->curParser, TOKEN_LEFT_PAREN ))
	{
		sign->type = SIGN_METHOD;
		sign->argNum = 1;
		ConsumeCurToken( cu->curParser, TOKEN_ID, "expect variable name!" );
		DeclareVariable( cu, cu->curParser->preToken.start, cu->curParser->preToken.length );
		ConsumeCurToken( cu->curParser, TOKEN_RIGHT_PAREN, "expect ')' after parameter!" );
	}
}

//添加常量并返回其索引
static uint32_t AddConstant( CompileUnit *cu, Value constant )
{
	ValueBufferAdd( cu->curParser->vm, &cu->fn->constants, constant );
	return cu->fn->constants.count - 1;
}

//生成加载常量的指令
static void EmitLoadConstant( CompileUnit *cu, Value value )
{
	int index = AddConstant( cu, value );
	WriteOpCodeShortOperand( cu, OPCODE_LOAD_CONSTANT, index );
}

//数字和字符串.nud() 编译字面量
static void Literal( CompileUnit *cu, bool canAssign UNUSED)
{
	//literal是常量(数字和字符串)的nud方法,用来返回字面值.
	EmitLoadConstant( cu, cu->curParser->preToken.value );
}

//不关注左操作数的符号称为前缀符号
//用于如字面量,变量名,前缀符号等非运算符
#define PREFIX_SYMBOL( nud ) {NULL, BP_NONE, nud, NULL, NULL}

//前缀运算符,如'!'
#define PREFIX_OPERATOR( id ) {id, BP_NONE, unaryOperator, NULL, unaryMethodSignature}

//关注左操作数的符号称为中缀符号
//数组'[',函数'(',实例与方法之间的'.'等
#define INFIX_SYMBOL( lbp, led ) {NULL, lbp, NULL, led, NULL}

//中棳运算符
#define INFIX_OPERATOR( id, lbp ) {id, lbp, NULL, infixOperator, infixMethodSignature}

//既可做前缀又可做中缀的运算符,如'-'
#define MIX_OPERATOR( id ) {id, BP_TERM, unaryOperator, infixOperator, mixMethodSignature}

//占位用的
#define UNUSED_RULE {NULL, BP_NONE, NULL, NULL, NULL}

SymbolBindRule Rules[] = {
	/* TOKEN_INVALID*/            UNUSED_RULE,
	/* TOKEN_NUM	*/            PREFIX_SYMBOL( Literal ),
	/* TOKEN_STRING */            PREFIX_SYMBOL( Literal ),
};

/** Expression
 * TDOP 语法分析的核心 -> 计算表达式的结果
 * @param cu
 * @param rbp
 */
static void Expression( CompileUnit *cu, BindPower rbp )
{
	//以中缀运算符表达式"aSwTe"为例,
	//大写字符表示运算符,小写字符表示操作数
	
	//进入expression时,curToken是操作数w, preToken是运算符S
	DenotationFn nud = Rules[ cu->curParser->curToken.type ].nud;
	
	//表达式开头的要么是操作数要么是前缀运算符,必然有nud方法
	ASSERT( nud != NULL, "nud is NULL!" );
	
	GetNextToken( cu->curParser );  //执行后curToken为运算符T
	
	bool canAssign = rbp < BP_ASSIGN;
	nud( cu, canAssign );   //计算操作数w的值
	
	while ( rbp < Rules[ cu->curParser->curToken.type ].lbp )
	{
		DenotationFn led = Rules[ cu->curParser->curToken.type ].led;
		GetNextToken( cu->curParser );  //执行后curToken为操作数e
		led( cu, canAssign );  //计算运算符T.led方法
	}
}

/** EmitCallBySignature
 * 通过签名编译方法调用 包括callX和superX指令
 * @param cu
 * @param sign
 * @param opcode
 */
static void EmitCallBySignature( CompileUnit *cu, Signature *sign, OpCode opcode )
{
	char signBuffer[MAX_SIGN_LEN];
	uint32_t length = Sign2String( sign, signBuffer );
	
	// 确保录入
	int symbolIndex = EnsureSymbolExist( cu->curParser->vm, &cu->curParser->vm->allMethodNames, signBuffer, length );
	WriteOpCodeShortOperand( cu, opcode + sign->argNum, symbolIndex );
	
	//此时在常量表中预创建一个空slot占位,将来绑定方法时再装入基类
	if ( opcode == OPCODE_SUPER0 )
	{
		WriteShortOperand( cu, AddConstant( cu, VT_TO_VALUE( VT_NULL )));
	}
}

/** DeclareModuleVar
 * 声明模块变量,与defineModuleVar的区别是不做重定义检查,默认为声明
 * @param vm
 * @param objModule
 * @param name
 * @param length
 * @param value
 * @return
 */
static int DeclareModuleVar( VM *vm, ObjModule *objModule, const char *name, uint32_t length, Value value )
{
	ValueBufferAdd( vm, &objModule->moduleVarValue, value );
	return AddSymbol( vm, &objModule->moduleVarName, name, length );
}

//返回包含cu->enclosingClassBK的最近的CompileUnit
static CompileUnit *GetEnclosingClassBKUnit( CompileUnit *cu )
{
	while ( cu != NULL)
	{
		if ( cu->enclosingClassBK != NULL)
		{
			return cu;
		}
		cu = cu->enclosingUnit;
	}
	return NULL;
}

//返回包含cu最近的ClassBookKeep
static ClassBookKeep *GetEnclosingClassBK( CompileUnit *cu )
{
	CompileUnit *ncu = GetEnclosingClassBKUnit( cu );
	if ( ncu != NULL)
	{
		return ncu->enclosingClassBK;
	}
	return NULL;
}

static void ProcessArgList( CompileUnit *cu, Signature *sign )
{
	//由主调方保证参数不空
	ASSERT( cu->curParser->curToken.type != TOKEN_RIGHT_PAREN &&
	        cu->curParser->curToken.type != TOKEN_RIGHT_BRACKET, "empty argument list!" );
	do
	{
		if ( ++sign->argNum > MAX_ARG_NUM )
		{
			COMPILE_ERROR( cu->curParser, "the max number of argument is %d!", MAX_ARG_NUM );
		}
		Expression( cu, BP_LOWEST );  //加载实参
	} while ( MatchToken( cu->curParser, TOKEN_COMMA ));
}

//声明形参列表中的各个形参
static void ProcessParaList( CompileUnit *cu, Signature *sign )
{
	ASSERT( cu->curParser->curToken.type != TOKEN_RIGHT_PAREN &&
	        cu->curParser->curToken.type != TOKEN_RIGHT_BRACKET, "empty argument list!" );
	do
	{
		if ( ++sign->argNum > MAX_ARG_NUM )
		{
			COMPILE_ERROR( cu->curParser, "the max number of argument is %d!", MAX_ARG_NUM );
		}
		ConsumeCurToken( cu->curParser, TOKEN_ID, "expect variable name!" );
		DeclareVariable( cu, cu->curParser->preToken.start, cu->curParser->preToken.length );
	} while ( MatchToken( cu->curParser, TOKEN_COMMA ));
}

//尝试编译setter
static bool TrySetter( CompileUnit *cu, Signature *sign )
{
	if ( !MatchToken( cu->curParser, TOKEN_ASSIGN ))
	{
		return false;
	}
	
	if ( sign->type == SIGN_SUBSCRIPT )
	{
		sign->type = SIGN_SUBSCRIPT_SETTER;
	}
	else
	{
		sign->type = SIGN_SETTER;
	}
	
	//读取等号右边的形参左边的'('
	ConsumeCurToken( cu->curParser, TOKEN_LEFT_PAREN, "expect '(' after '='!" );
	//读取形参
	ConsumeCurToken( cu->curParser, TOKEN_ID, "expect ID!" );
	//声明形参
	DeclareVariable( cu, cu->curParser->preToken.start, cu->curParser->preToken.length );
	
	//读取等号右边的形参右边的')'
	ConsumeCurToken( cu->curParser, TOKEN_RIGHT_PAREN, "expect ')' after argument list!" );
	sign->argNum++;
	return true;
	
}

//标识符的签名函数
static void IdMethodSignature( CompileUnit *cu, Signature *sign )
{
	//刚识别到id,默认为getter
	sign->type = SIGN_GETTER;
	
	//new方法为构造函数
	if ( sign->length == 3 && memcmp( sign->name, "new", 3 ) == 0 )
	{
		
		//构造函数后面不能接'=',即不能成为setter
		if ( MatchToken( cu->curParser, TOKEN_ASSIGN ))
		{
			COMPILE_ERROR( cu->curParser, "constructor shouldn`t be setter!" );
		}
		
		//构造函数必须是标准的method,即new(_,...),new后面必须接'('
		if ( !MatchToken( cu->curParser, TOKEN_LEFT_PAREN ))
		{
			COMPILE_ERROR( cu->curParser, "constructor must be method!" );
		}
		
		sign->type = SIGN_CONSTRUCT;
		
		//无参数就直接返回
		if ( MatchToken( cu->curParser, TOKEN_RIGHT_PAREN ))
		{
			return;
		}
		
	}
	else
	{  //若不是构造函数
		
		if ( TrySetter( cu, sign ))
		{
			//若是setter,此时已经将type改为了setter,直接返回
			return;
		}
		
		if ( !MatchToken( cu->curParser, TOKEN_LEFT_PAREN ))
		{
			//若后面没有'('说明是getter,type已在开头置为getter,直接返回
			return;
		}
		
		//至此type应该为一般形式的SIGN_METHOD,形式为name(paralist)
		sign->type = SIGN_METHOD;
		//直接匹配到')'，说明形参为空
		if ( MatchToken( cu->curParser, TOKEN_RIGHT_PAREN ))
		{
			return;
		}
	}
	
	//下面处理形参
	ProcessParaList( cu, sign );
	ConsumeCurToken( cu->curParser, TOKEN_RIGHT_PAREN, "expect ')' after parameter list!" );
	
}

//查找局部变量
static int FindLocal( CompileUnit *cu, const char *name, uint32_t length )
{
	for ( int index = cu->localVarNum - 1; index >= 0; index-- )
	{
		if ( cu->localVars[ index ].length == length && memcmp( cu->localVars[ index ].name, name, length ) == 0 )
		{
			return index;
		}
	}
	return -1;
}

//添加upvalue到cu->upvalues,返回其索引.若已存在则只返回索引
static int AddUpvalue( CompileUnit *cu, bool isEnclosingLocalVar, uint32_t index )
{
	for ( uint32_t idx = 0; idx < cu->fn->upvalueNum; idx++ )
	{
		if ( cu->upvalues[ idx ].index == index &&
		     cu->upvalues[ idx ].isEnclosingLocalVar == isEnclosingLocalVar )
		{
			return idx;
		}
	}
	
	//若没找到则将其添加
	cu->upvalues[ cu->fn->upvalueNum ].isEnclosingLocalVar = isEnclosingLocalVar;
	cu->upvalues[ cu->fn->upvalueNum ].index = index;
	return cu->fn->upvalueNum++;
	
}

//查找name指代的upvalue后添加到cu->upvalues,返回其索引,否则返回-1
static int FindUpvalue( CompileUnit *cu, const char *name, uint32_t length )
{
	if ( cu->enclosingUnit == NULL)
	{ //如果已经到了最外层仍未找到,返回-1.
		return -1;
	}
	//进入了方法的cu并且查找的不是静态域,即不是方法的Upvalue,那就没必要再往上找了
	if ( !strchr( name, ' ' ) && cu->enclosingUnit->enclosingClassBK != NULL)
	{
		return -1;
	}
	
	//查看name是否为直接外层的局部变量
	int directOuterLocalIndex = FindLocal( cu->enclosingUnit, name, length );
	
	//若是,将该外层局部变量置为upvalue,
	if ( directOuterLocalIndex != -1 )
	{
		cu->enclosingUnit->localVars[ directOuterLocalIndex ].isUpvalue = true;
		return AddUpvalue( cu, true, ( uint32_t )directOuterLocalIndex );
	}
	
	//向外层递归查找
	int directOuterUpvalueIndex = FindUpvalue( cu->enclosingUnit, name, length );
	if ( directOuterUpvalueIndex != -1 )
	{
		return AddUpvalue( cu, false, ( uint32_t )directOuterUpvalueIndex );
	}
	
	//执行到此说明没有该upvalue对应的局部变量,返回-1
	return -1;
}

//从局部变量和upvalue中查找符号name
static Variable GetVarFromLocalOrUpvalue( CompileUnit *cu, const char *name, uint32_t length )
{
	Variable var;
	//默认为无效作用域类型,查找到后会被更正
	var.scopeType = VAR_SCOPE_INVALID;
	
	var.index = FindLocal( cu, name, length );
	if ( var.index != -1 )
	{
		var.scopeType = VAR_SCOPE_LOCAL;
		return var;
	}
	
	var.index = FindUpvalue( cu, name, length );
	if ( var.index != -1 )
	{
		var.scopeType = VAR_SCOPE_UPVALUE;
	}
	return var;
}

//生成把变量var加载到栈的指令
static void EmitLoadVariable( CompileUnit *cu, Variable var )
{
	switch ( var.scopeType )
	{
	case VAR_SCOPE_LOCAL:
		//生成加载局部变量入栈的指令
		WriteOpCodeByteOperand( cu, OPCODE_LOAD_LOCAL_VAR, var.index );
		break;
	case VAR_SCOPE_UPVALUE:
		//生成加载upvalue到栈的指令
		WriteOpCodeByteOperand( cu, OPCODE_LOAD_UPVALUE, var.index );
		break;
	case VAR_SCOPE_MODULE:
		//生成加载模块变量到栈的指令
		WriteOpCodeShortOperand( cu, OPCODE_LOAD_MODULE_VAR, var.index );
		break;
	default:
		// 不可能抵达的代码块
		NOT_REACHED( );
	}
}

//为变量var生成存储的指令
static void EmitStoreVariable( CompileUnit *cu, Variable var )
{
	switch ( var.scopeType )
	{
	case VAR_SCOPE_LOCAL:
		//生成存储局部变量的指令
		WriteOpCodeByteOperand( cu, OPCODE_STORE_LOCAL_VAR, var.index );
		break;
	case VAR_SCOPE_UPVALUE:
		//生成存储upvalue的指令
		WriteOpCodeByteOperand( cu, OPCODE_STORE_UPVALUE, var.index );
		break;
	case VAR_SCOPE_MODULE:
		//生成存储模块变量的指令
		WriteOpCodeByteOperand( cu, OPCODE_STORE_MODULE_VAR, var.index );
		break;
	default:
		NOT_REACHED( );
	}
}

//生成加载或存储变量的指令
static void EmitLoadOrStoreVariable( CompileUnit *cu, bool canAssign, Variable var )
{
	if ( canAssign && MatchToken( cu->curParser, TOKEN_ASSIGN ))
	{
		Expression( cu, BP_LOWEST );   //计算'='右边表达式的值
		EmitStoreVariable( cu, var );  //为var生成赋值指令
	}
	else
	{
		EmitLoadVariable( cu, var );   //为var生成读取指令
	}
}

//生成把实例对象this加载到栈的指令
static void EmitLoadThis( CompileUnit *cu )
{
	Variable var = GetVarFromLocalOrUpvalue( cu, "this", 4 );
	ASSERT( var.scopeType != VAR_SCOPE_INVALID, "get variable failed!" );
	EmitLoadVariable( cu, var );
}

//编译代码块
static void CompileBlock( CompileUnit *cu )
{
	//进入本函数前已经读入了'{'
	while ( !MatchToken( cu->curParser, TOKEN_RIGHT_BRACE ))
	{
		if ( PEEK_TOKEN( cu->curParser ) == TOKEN_EOF )
		{
			COMPILE_ERROR( cu->curParser, "expect '}' at the end of block!" );
		}
		CompileProgram( cu );
	}
}

//编译函数或方法体
static void CompileBody( CompileUnit *cu, bool isConstruct )
{
	//进入本函数前已经读入了'{'
	CompileBlock( cu );
	if ( isConstruct )
	{
		//若是构造函数就加载"this对象"做为下面OPCODE_RETURN的返回值
		WriteOpCodeByteOperand( cu, OPCODE_LOAD_LOCAL_VAR, 0 );
	}
	else
	{
		//否则加载null占位
		WriteOpCode( cu, OPCODE_PUSH_NULL );
	}
	
	//返回编译结果,若是构造函数就返回this,否则返回null
	WriteOpCode( cu, OPCODE_RETURN );
}

//结束cu的编译工作,在其外层编译单元中为其创建闭包
#if DEBUG
static ObjFn *EndCompileUnit( CompileUnit *cu, const char *debugName, uint32_t debugNameLen )
{
	BindDebugFnName( cu->curParser->vm, cu->fn->debug, debugName, debugNameLen );
#else
static ObjFn *EndCompileUnit( CompileUnit *cu )
{
#endif
	//标识单元编译结束
	WriteOpCode( cu, OPCODE_END );
	if ( cu->enclosingUnit != NULL)
	{
		//把当前编译的objFn做为常量添加到父编译单元的常量表
		uint32_t index = AddConstant( cu->enclosingUnit, OBJ_TO_VALUE( cu->fn ));
		
		//内层函数以闭包形式在外层函数中存在,
		//在外层函数的指令流中添加"为当前内层函数创建闭包的指令"
		WriteOpCodeShortOperand( cu->enclosingUnit, OPCODE_CREATE_CLOSURE, index );
		
		//为vm在创建闭包时判断引用的是局部变量还是upvalue,
		//下面为每个upvalue生成参数.
		index = 0;
		while ( index < cu->fn->upvalueNum )
		{
			WriteByte( cu->enclosingUnit,
				cu->upvalues[ index ].isEnclosingLocalVar ? 1 : 0 );
			WriteByte( cu->enclosingUnit,
				cu->upvalues[ index ].index );
			index++;
		}
	}
	
	///下掉本编译单元,使当前编译单元指向外层编译单元
	cu->curParser->curCompileUnit = cu->enclosingUnit;
	return cu->fn;
}

//生成getter或一般method调用指令
static void EmitGetterMethodCall( CompileUnit *cu, Signature *sign, OpCode opCode )
{
	Signature newSign;
	newSign.type = SIGN_GETTER;  //默认为getter,假设下面的两个if不执行
	newSign.name = sign->name;
	newSign.length = sign->length;
	newSign.argNum = 0;
	
	//如果是method,有可能有参数列表  在生成调用方法的指令前必须把参数入栈  否则运行方法时除了会获取到错误的参数(即栈中已有数据)外,g还会在从方法返回时,错误地回收参数空间而导致栈失衡
	
	//下面调用的processArgList是把实参入栈,供方法使用
	if ( MatchToken( cu->curParser, TOKEN_LEFT_PAREN ))
	{    //判断后面是否有'('
		newSign.type = SIGN_METHOD;
		
		//若后面不是')',说明有参数列表
		if ( !MatchToken( cu->curParser, TOKEN_RIGHT_PAREN ))
		{
			ProcessArgList( cu, &newSign );
			ConsumeCurToken( cu->curParser, TOKEN_RIGHT_PAREN, "expect ')' after argument list!" );
		}
	}
	
	//对method来说可能还传入了块参数
	if ( MatchToken( cu->curParser, TOKEN_LEFT_BRACE ))
	{
		newSign.argNum++;
		//进入本if块时,上面的if块未必执行过,
		//此时newSign.type也许还是GETTER,下面要将其设置为METHOD
		newSign.type = SIGN_METHOD;
		CompileUnit fnCU;
		InitCompileUnit( cu->curParser, &fnCU, cu, false );
		
		Signature tmpFnSign = { SIGN_METHOD, "", 0, 0 }; //临时用于编译函数
		if ( MatchToken( cu->curParser, TOKEN_BIT_OR ))
		{  //若块参数也有参数
			ProcessParaList( &fnCU, &tmpFnSign );    //将形参声明为函数的局部变量
			ConsumeCurToken( cu->curParser, TOKEN_BIT_OR, "expect '|' after argument list!" );
		}
		fnCU.fn->argNum = tmpFnSign.argNum;
		
		//编译函数体,将指令流写进该函数自己的指令单元fnCu
		CompileBody( &fnCU, false );

#if DEBUG
		//以此函数被传给的方法来命名这个函数, 函数名=方法名+" block arg"
	  char fnName[MAX_SIGN_LEN + 10] = {'\0'}; //"block arg\0"
	  uint32_t len = sign2String(&newSign, fnName);
	  memmove(fnName + len, " block arg", 10);
	  endCompileUnit(&fnCU, fnName, len + 10);
#else
		EndCompileUnit( &fnCU );
#endif
	}
	
	//如果是在子类构造函数中
	if ( sign->type == SIGN_CONSTRUCT )
	{
		if ( newSign.type != SIGN_METHOD )
		{
			COMPILE_ERROR( cu->curParser, "the form of supercall is super() or super(arguments)" );
		}
		newSign.type = SIGN_CONSTRUCT;
	}
	
	//根据签名生成调用指令.如果上面的三个if都未执行,此处就是getter调用
	EmitCallBySignature( cu, &newSign, opCode );
	
}

//生成方法调用指令,包括getter和setter
static void EmitMethodCall( CompileUnit *cu, const char *name, uint32_t length, OpCode opCode, bool canAssign )
{
	Signature sign;
	sign.type = SIGN_GETTER;
	sign.name = name;
	sign.length = length;
	
	//若是setter则生成调用setter的指令
	if ( MatchToken( cu->curParser, TOKEN_ASSIGN ) && canAssign )
	{
		sign.type = SIGN_SETTER;
		sign.argNum = 1;   //setter只接受一个参数
		
		//载入实参(即'='右边所赋的值),为下面方法调用传参
		Expression( cu, BP_LOWEST );
		
		EmitCallBySignature( cu, &sign, opCode );
	}
	else
	{
		EmitGetterMethodCall( cu, &sign, opCode );
	}
}

/** EmitCall
 * 生成方法调用的指令,仅限callX指令
 * @param cu
 * @param numArgs
 * @param name
 * @param length
 */
static void EmitCall( CompileUnit *cu, int numArgs, const char *name, int length )
{
	int symbolIndex = EnsureSymbolExist( cu->curParser->vm, &cu->curParser->vm->allMethodNames, name, length );
	WriteOpCodeShortOperand( cu, OPCODE_CALL0 + numArgs, symbolIndex );
}

//小写字符开头便是局部变量
static bool isLocalName( const char *name )
{
	return ( name[ 0 ] >= 'a' && name[ 0 ] <= 'z' );
}

//标识符.nud():变量名或方法名
static void id( CompileUnit *cu, bool canAssign )
{
	Token name = cu->curParser->preToken;
	ClassBookKeep *classBK = GetEnclosingClassBK( cu );
	
	//标识符可以是任意符号,按照此顺序处理:
	//函数调用->局部变量和upvalue->实例域->静态域->类getter方法调用->模块变量
	
	if ( cu->enclosingUnit == NULL && MatchToken( cu->curParser, TOKEN_LEFT_PAREN ))
	{
		char id[MAX_ID_LEN] = { '\0' };
		//函数名加上"Fn "前缀做为模块变量名
		//检查前面是否已有此函数的定义
		memmove( id, "Fn ", 3 );
		memmove( id + 3, name.start, name.length );
		Variable var;
		var.scopeType = VAR_SCOPE_MODULE;
		var.index = GetIndexFromSymbolTable( &cu->curParser->curModule->moduleVarName, id, strlen( id ));
		if ( var.index == -1 )
		{
			memmove( id, name.start, name.length );
			id[ name.length ] = '\0';
			COMPILE_ERROR( cu->curParser, "Undefined function: '%s'!", id );
		}
		
		// 1.把模块变量即函数闭包加载到栈
		EmitLoadVariable( cu, var );
		Signature sign;
		//函数调用的形式和method类似,只不过method有一个可选的块参数
		sign.type = SIGN_METHOD;
		//把函数调用编译为"闭包.call"的形式,故name为call
		sign.name = "call";
		sign.length = 4;
		sign.argNum = 0;
		
		//若后面不是')',说明有参数列表
		if ( !MatchToken( cu->curParser, TOKEN_RIGHT_PAREN ))
		{
			// 2 压入实参
			ProcessArgList( cu, &sign );
			ConsumeCurToken( cu->curParser, TOKEN_RIGHT_PAREN, "expect ')' after argument list!" );
		}
		
		// 3 生成调用指令以调用函数
		EmitCallBySignature( cu, &sign, OPCODE_CALL0 );
	}
	else //否则按照各种变量来处理
	{
		//按照局部变量和upvalue来处理
		Variable var = GetVarFromLocalOrUpvalue( cu, name.start, name.length );
		if ( var.index != -1 )
		{
			EmitLoadOrStoreVariable( cu, canAssign, var );
			return;
		}
		
		//按照实例域来处理
		if ( classBK != NULL)
		{
			int fieldIndex = GetIndexFromSymbolTable( &classBK->fields, name.start, name.length );
			if ( fieldIndex != -1 )
			{
				bool isRead = true;
				if ( canAssign && MatchToken( cu->curParser, TOKEN_ASSIGN ))
				{
					isRead = false;
					Expression( cu, BP_LOWEST );
				}
				
				//如果当前正在编译类方法,则直接在该实例对象中加载field
				if ( cu->enclosingUnit != NULL)
				{
					WriteOpCodeByteOperand( cu, isRead ? OPCODE_LOAD_THIS_FIELD : OPCODE_STORE_THIS_FIELD, fieldIndex );
				}
				else
				{
					EmitLoadThis( cu );
					WriteOpCodeByteOperand( cu, isRead ? OPCODE_LOAD_FIELD : OPCODE_STORE_FIELD, fieldIndex );
				}
				return;
			}
		}
		
		//按照静态域查找
		if ( classBK != NULL)
		{
			char *staticFieldId = ALLOCATE_ARRAY( cu->curParser->vm, char, MAX_ID_LEN );
			memset( staticFieldId, 0, MAX_ID_LEN );
			uint32_t staticFieldIdLen;
			char *clsName = classBK->name->value.start;
			uint32_t clsLen = classBK->name->value.length;
			
			//各类中静态域的名称以"Cls类名 静态域名"来命名
			memmove(staticFieldId, "Cls", 3);
			memmove(staticFieldId + 3, clsName, clsLen);
			memmove(staticFieldId + 3 + clsLen, " ", 1);
			
			
		}
		
	}
}

/** InfixOperator
 * 中缀运算符.led方法
 * @param cu
 * @param canAssign
 */
static void InfixOperator( CompileUnit *cu, bool canAssign UNUSED)
{
	SymbolBindRule *rule = &Rules[ cu->curParser->preToken.type ];
	
	//中缀运算符对左右操作数的绑定权值一样
	BindPower rbp = rule->lbp;
	Expression( cu, rbp );  //解析右操作数
	
	//生成1个参数的签名
	Signature sign = { SIGN_METHOD, rule->id, strlen( rule->id ), 1 };
	EmitCallBySignature( cu, &sign, OPCODE_CALL0 );
}

/** UnaryOperator
 * 前缀运算符.nud方法, 如'-','!'等
 * @param cu
 * @param canAssign
 */
static void UnaryOperator( CompileUnit *cu, bool canAssign UNUSED)
{
	SymbolBindRule *rule = &Rules[ cu->curParser->preToken.type ];
	
	//BP_UNARY做为rbp去调用expression解析右操作数
	Expression( cu, BP_UNARY );
	
	//生成调用前缀运算符的指令
	//0个参数,前缀运算符都是1个字符,长度是1
	EmitCall( cu, 0, rule->id, 1 );
}

//编译程序
static void CompileProgram( CompileUnit *cu )
{
	;
}

//编译模块
ObjFn *CompileModule( VM *vm, ObjModule *objModule, const char *moduleCode )
{
	Parser parser;
	parser.parent = vm->curParser;
	vm->curParser = &parser;
	
	if ( objModule->name == NULL)
	{
		// 核心模块是core.script.inc
		InitParser( vm, &parser, "core.script.inc", moduleCode, objModule );
	}
	else
	{
		InitParser( vm, &parser, ( const char * )objModule->name->value.start, moduleCode, objModule );
	}
	
	CompileUnit moduleCU;
	InitCompileUnit( &parser, &moduleCU, NULL, false );
	
	//记录现在模块变量的数量,后面检查预定义模块变量时可减少遍历
	uint32_t moduleVarNumBefor = objModule->moduleVarValue.count;
	
	//初始的parser->curToken.type为TOKEN_UNKNOWN,下面使其指向第一个合法的token
	GetNextToken( &parser );
	
	//此时compileProgram为桩函数,并不会读进token,因此是死循环.
	while ( !MatchToken( &parser, TOKEN_EOF ))
	{
		CompileProgram( &moduleCU );
	}
	
	/* TODO */
	printf( "There is something to do...\n" );
	exit( 0 );
	
}

