#ifndef _VM_VM_H
#define _VM_VM_H

#include "common.h"
#include "class.h"
#include "obj_map.h"
#include "obj_thread.h"
#include "parser.h"

//为定义操作码加上前缀"OPCODE_"
#define OPCODE_SLOTS(opcode, effect) OPCODE_##opcode,
typedef enum {
    OPCODE_SLOTS(LOAD_CONSTANT, 1)
    OPCODE_SLOTS(PUSH_NULL, 1)
    OPCODE_SLOTS(PUSH_FALSE, 1)
    OPCODE_SLOTS(PUSH_TRUE, 1)
    OPCODE_SLOTS(LOAD_LOCAL_VAR, 1)
    OPCODE_SLOTS(STORE_LOCAL_VAR, 0)
    OPCODE_SLOTS(LOAD_UPVALUE, 1)
    OPCODE_SLOTS(STORE_UPVALUE, 0)
    OPCODE_SLOTS(LOAD_MODULE_VAR, 1)
    OPCODE_SLOTS(STORE_MODULE_VAR, 0)
    OPCODE_SLOTS(LOAD_THIS_FIELD, 1)
    OPCODE_SLOTS(STORE_THIS_FIELD, 0)
    OPCODE_SLOTS(LOAD_FIELD, 0)
    OPCODE_SLOTS(STORE_FIELD, -1)
    OPCODE_SLOTS(POP, -1)
    OPCODE_SLOTS(CALL0, 0)
    OPCODE_SLOTS(CALL1, -1)
    OPCODE_SLOTS(CALL2, -2)
    OPCODE_SLOTS(CALL3, -3)
    OPCODE_SLOTS(CALL4, -4)
    OPCODE_SLOTS(CALL5, -5)
    OPCODE_SLOTS(CALL6, -6)
    OPCODE_SLOTS(CALL7, -7)
    OPCODE_SLOTS(CALL8, -8)
    OPCODE_SLOTS(CALL9, -9)
    OPCODE_SLOTS(CALL10, -10)
    OPCODE_SLOTS(CALL11, -11)
    OPCODE_SLOTS(CALL12, -12)
    OPCODE_SLOTS(CALL13, -13)
    OPCODE_SLOTS(CALL14, -14)
    OPCODE_SLOTS(CALL15, -15)
    OPCODE_SLOTS(CALL16, -16)
    OPCODE_SLOTS(SUPER0, 0)
    OPCODE_SLOTS(SUPER1, -1)
    OPCODE_SLOTS(SUPER2, -2)
    OPCODE_SLOTS(SUPER3, -3)
    OPCODE_SLOTS(SUPER4, -4)
    OPCODE_SLOTS(SUPER5, -5)
    OPCODE_SLOTS(SUPER6, -6)
    OPCODE_SLOTS(SUPER7, -7)
    OPCODE_SLOTS(SUPER8, -8)
    OPCODE_SLOTS(SUPER9, -9)
    OPCODE_SLOTS(SUPER10, -10)
    OPCODE_SLOTS(SUPER11, -11)
    OPCODE_SLOTS(SUPER12, -12)
    OPCODE_SLOTS(SUPER13, -13)
    OPCODE_SLOTS(SUPER14, -14)
    OPCODE_SLOTS(SUPER15, -15)
    OPCODE_SLOTS(SUPER16, -16)
    OPCODE_SLOTS(JUMP, 0)
    OPCODE_SLOTS(LOOP, 0)
    OPCODE_SLOTS(JUMP_IF_FALSE, -1)
    OPCODE_SLOTS(AND, -1)
    OPCODE_SLOTS(OR, -1)
    OPCODE_SLOTS(CLOSE_UPVALUE, -1)
    OPCODE_SLOTS(RETURN, 0)
    OPCODE_SLOTS(CREATE_CLOSURE, 1)
    OPCODE_SLOTS(CONSTRUCT, 0)
    OPCODE_SLOTS(CREATE_CLASS, -1)
    OPCODE_SLOTS(INSTANCE_METHOD, -2)
    OPCODE_SLOTS(STATIC_METHOD, -2)
    OPCODE_SLOTS(END, 0)
} OpCode;
#undef OPCODE_SLOTS

typedef enum vmResult {
    VM_RESULT_SUCCESS,
    VM_RESULT_ERROR
} VMResult;   //虚拟机执行结果
//如果执行无误,可以将字符码输出到文件缓存,避免下次重新编译

struct vm {
    Class *classOfClass;
    Class *objectClass;
    Class *stringClass;
    Class *mapClass;
    Class *rangeClass;
    Class *listClass;
    Class *nullClass;
    Class *boolClass;
    Class *numClass;
    Class *fnClass;
    Class *threadClass;
    uint32_t allocatedBytes;  //累计已分配的内存量
    ObjHeader *allObjects;  //所有已分配对象链表
    SymbolTable allMethodNames;    //(所有)类的方法名
    ObjMap *allModules;
    ObjThread *curThread;   //当前正在执行的线程
    Parser *curParser;  //当前词法分析器
};

void initVM(VM *vm);

VM *newVM(void);

void ensureStack(VM *vm, ObjThread *objThread, uint32_t neededSlots);

VMResult executeInstruction(VM *vm, register ObjThread *curThread);

#endif
