//
// Created by MorbidArk on 2023/3/17.
//

#include <math.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "core.Num.h"
#include "core.h"
#include "obj_range.h"

//将字符串转换为数字
static bool primNumFromString(VM *vm, Value *args)
{
    if (!validateString(vm, args[1]))
    {
        return false;
    }
    
    ObjString *objString = VALUE_TO_OBJSTR(args[1]);
    
    //空字符串返回RETURN_NULL
    if (objString->value.length == 0)
    {
        RET_NULL;
    }
    
    ASSERT(objString->value.start[objString->value.length] == '\0', "objString don`t teminate!");
    
    errno = 0;
    char *endPtr;
    
    //将字符串转换为double型, 它会自动跳过前面的空白
    double num = strtod(objString->value.start, &endPtr);
    
    //以endPtr是否等于start+length来判断不能转换的字符之后是否全是空白
    while (*endPtr != '\0' && isspace((unsigned char)*endPtr))
    {
        endPtr++;
    }
    
    if (errno == ERANGE)
    {
        RUN_ERROR("string too large!");
    }
    
    //如果字符串中不能转换的字符不全是空白,字符串非法,返回NULL
    if (endPtr < objString->value.start + objString->value.length)
    {
        RET_NULL;
    }
    
    //至此,检查通过,返回正确结果
    RET_NUM(num);
}

//返回圆周率
static bool primNumPi(VM *vm UNUSED, Value *args UNUSED)
{
    RET_NUM(3.14159265358979323846);
}

#define PRIM_NUM_INFIX(name, operator, type) \
   static bool name(VM* vm, Value* args) {\
      if (!validateNum(vm, args[1])) {\
     return false; \
      }\
      RET_##type(VALUE_TO_NUM(args[0]) operator VALUE_TO_NUM(args[1]));\
   }

PRIM_NUM_INFIX(primNumPlus, +, NUM);

PRIM_NUM_INFIX(primNumMinus, -, NUM);

PRIM_NUM_INFIX(primNumMul, *, NUM);

PRIM_NUM_INFIX(primNumDiv, /, NUM);

PRIM_NUM_INFIX(primNumGt, >, BOOL);

PRIM_NUM_INFIX(primNumGe, >=, BOOL);

PRIM_NUM_INFIX(primNumLt, <, BOOL);

PRIM_NUM_INFIX(primNumLe, <=, BOOL);
#undef PRIM_NUM_INFIX

#define PRIM_NUM_BIT(name, operator) \
   static bool name(VM* vm UNUSED, Value* args) {\
      if (!validateNum(vm, args[1])) {\
     return false;\
      }\
      uint32_t leftOperand = VALUE_TO_NUM(args[0]); \
      uint32_t rightOperand = VALUE_TO_NUM(args[1]); \
      RET_NUM(leftOperand operator rightOperand);\
   }

PRIM_NUM_BIT(primNumBitAnd, &);

PRIM_NUM_BIT(primNumBitOr, |);

PRIM_NUM_BIT(primNumBitShiftRight, >>);

PRIM_NUM_BIT(primNumBitShiftLeft, <<);
#undef PRIM_NUM_BIT

//使用数学库函数
#define PRIM_NUM_MATH_FN(name, mathFn) \
   static bool name(VM* vm UNUSED, Value* args) {\
      RET_NUM(mathFn(VALUE_TO_NUM(args[0]))); \
   }

PRIM_NUM_MATH_FN(primNumAbs, fabs);

PRIM_NUM_MATH_FN(primNumAcos, acos);

PRIM_NUM_MATH_FN(primNumAsin, asin);

PRIM_NUM_MATH_FN(primNumAtan, atan);

PRIM_NUM_MATH_FN(primNumCeil, ceil);

PRIM_NUM_MATH_FN(primNumCos, cos);

PRIM_NUM_MATH_FN(primNumFloor, floor);

PRIM_NUM_MATH_FN(primNumNegate, -);

PRIM_NUM_MATH_FN(primNumSin, sin);

PRIM_NUM_MATH_FN(primNumSqrt, sqrt);  //开方
PRIM_NUM_MATH_FN(primNumTan, tan);
#undef PRIM_NUM_MATH_FN

//这里用fmod实现浮点取模
static bool primNumMod(VM *vm UNUSED, Value *args)
{
    if (!validateNum(vm, args[1]))
    {
        return false;
    }
    RET_NUM(fmod(VALUE_TO_NUM(args[0]), VALUE_TO_NUM(args[1])));
}

//数字取反
static bool primNumBitNot(VM *vm UNUSED, Value *args)
{
    RET_NUM(~(uint32_t)VALUE_TO_NUM(args[0]));
}

//[数字from..数字to]
static bool primNumRange(VM *vm UNUSED, Value *args)
{
    if (!validateNum(vm, args[1]))
    {
        return false;
    }
    
    double from = VALUE_TO_NUM(args[0]);
    double to = VALUE_TO_NUM(args[1]);
    RET_OBJ(newObjRange(vm, from, to));
}

//atan2(args[1])
static bool primNumAtan2(VM *vm UNUSED, Value *args)
{
    if (!validateNum(vm, args[1]))
    {
        return false;
    }
    
    RET_NUM(atan2(VALUE_TO_NUM(args[0]), VALUE_TO_NUM(args[1])));
}

//返回小数部分
static bool primNumFraction(VM *vm UNUSED, Value *args)
{
    double dummyInteger;
    RET_NUM(modf(VALUE_TO_NUM(args[0]), &dummyInteger));
}

//判断数字是否无穷大,不区分正负无穷大
static bool primNumIsInfinity(VM *vm UNUSED, Value *args)
{
    RET_BOOL(isinf(VALUE_TO_NUM(args[0])));
}

//判断是否为数字
static bool primNumIsInteger(VM *vm UNUSED, Value *args)
{
    double num = VALUE_TO_NUM(args[0]);
    //如果是nan(不是一个数字)或无限大的数字就返回false
    if (isnan(num) || isinf(num))
    {
        RET_FALSE;
    }
    RET_BOOL(trunc(num) == num);
}

//判断数字是否为nan
static bool primNumIsNan(VM *vm UNUSED, Value *args)
{
    RET_BOOL(isnan(VALUE_TO_NUM(args[0])));
}

//数字转换为字符串
static bool primNumToString(VM *vm UNUSED, Value *args)
{
    RET_OBJ(num2str(vm, VALUE_TO_NUM(args[0])));
}

//取数字的整数部分
static bool primNumTruncate(VM *vm UNUSED, Value *args)
{
    double integer;
    modf(VALUE_TO_NUM(args[0]), &integer);
    RET_NUM(integer);
}

//判断两个数字是否相等
static bool primNumEqual(VM *vm UNUSED, Value *args)
{
    if (!validateNum(vm, args[1]))
    {
        RET_FALSE;
    }
    
    RET_BOOL(VALUE_TO_NUM(args[0]) == VALUE_TO_NUM(args[1]));
}

//判断两个数字是否不等
static bool primNumNotEqual(VM *vm UNUSED, Value *args)
{
    if (!validateNum(vm, args[1]))
    {
        RET_TRUE;
    }
    RET_BOOL(VALUE_TO_NUM(args[0]) != VALUE_TO_NUM(args[1]));
}

void coreNumBind(VM *vm, ObjModule *coreModule)
{
    vm->numClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Num"));
    //类方法
    PRIM_METHOD_BIND(vm->numClass->objHeader.class, "toNum(_)", primNumFromString);
    PRIM_METHOD_BIND(vm->numClass->objHeader.class, "pi", primNumPi);
    //实例方法
    PRIM_METHOD_BIND(vm->numClass, "+(_)", primNumPlus);
    PRIM_METHOD_BIND(vm->numClass, "-(_)", primNumMinus);
    PRIM_METHOD_BIND(vm->numClass, "*(_)", primNumMul);
    PRIM_METHOD_BIND(vm->numClass, "/(_)", primNumDiv);
    PRIM_METHOD_BIND(vm->numClass, ">(_)", primNumGt);
    PRIM_METHOD_BIND(vm->numClass, ">=(_)", primNumGe);
    PRIM_METHOD_BIND(vm->numClass, "<(_)", primNumLt);
    PRIM_METHOD_BIND(vm->numClass, "<=(_)", primNumLe);
    
    //位运算
    PRIM_METHOD_BIND(vm->numClass, "&(_)", primNumBitAnd);
    PRIM_METHOD_BIND(vm->numClass, "|(_)", primNumBitOr);
    PRIM_METHOD_BIND(vm->numClass, ">>(_)", primNumBitShiftRight);
    PRIM_METHOD_BIND(vm->numClass, "<<(_)", primNumBitShiftLeft);
    //以上都是通过rules中INFIX_OPERATOR来解析的
    
    //下面大多数方法是通过rules中'.'对应的led(callEntry)来解析,
    //少数符号依然是INFIX_OPERATOR解析
    PRIM_METHOD_BIND(vm->numClass, "abs", primNumAbs);
    PRIM_METHOD_BIND(vm->numClass, "acos", primNumAcos);
    PRIM_METHOD_BIND(vm->numClass, "asin", primNumAsin);
    PRIM_METHOD_BIND(vm->numClass, "atan", primNumAtan);
    PRIM_METHOD_BIND(vm->numClass, "ceil", primNumCeil);
    PRIM_METHOD_BIND(vm->numClass, "cos", primNumCos);
    PRIM_METHOD_BIND(vm->numClass, "floor", primNumFloor);
    PRIM_METHOD_BIND(vm->numClass, "-", primNumNegate);
    PRIM_METHOD_BIND(vm->numClass, "sin", primNumSin);
    PRIM_METHOD_BIND(vm->numClass, "sqrt", primNumSqrt);
    PRIM_METHOD_BIND(vm->numClass, "tan", primNumTan);
    PRIM_METHOD_BIND(vm->numClass, "%(_)", primNumMod);
    PRIM_METHOD_BIND(vm->numClass, "~", primNumBitNot);
    PRIM_METHOD_BIND(vm->numClass, "..(_)", primNumRange);
    PRIM_METHOD_BIND(vm->numClass, "atan(_)", primNumAtan2);
    PRIM_METHOD_BIND(vm->numClass, "fraction", primNumFraction);
    PRIM_METHOD_BIND(vm->numClass, "isInfinity", primNumIsInfinity);
    PRIM_METHOD_BIND(vm->numClass, "isInteger", primNumIsInteger);
    PRIM_METHOD_BIND(vm->numClass, "isNan", primNumIsNan);
    PRIM_METHOD_BIND(vm->numClass, "toString", primNumToString);
    PRIM_METHOD_BIND(vm->numClass, "truncate", primNumTruncate);
    PRIM_METHOD_BIND(vm->numClass, "==(_)", primNumEqual);
    PRIM_METHOD_BIND(vm->numClass, "!=(_)", primNumNotEqual);
}