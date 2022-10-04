//
// Created by TheMorbidArk on 2022/10/2.
//

#ifndef VANTIDEL_OBJ_THREAD_H
#define VANTIDEL_OBJ_THREAD_H

#include "obj_fn.h"

typedef struct objThread
{
	ObjHeader objHeader;

	Value *stack;  //运行时栈的栈底
	Value *esp;    //运行时栈的栈顶
	uint32_t stackCapacity;  //栈容量

	Frame *frames;   //调用框架 -> 调用堆栈 -> 调用闭包
	uint32_t usedFrameNum;   //已使用的frame数量
	uint32_t frameCapacity;  //frame容量

	//"打开的upvalue"的链表首结点
	ObjUpvalue *openUpvalues;

	//当前thread的调用者
	struct objThread *caller;

	//导致运行时错误的对象会放在此处,否则为空
	Value errorObj;
} ObjThread;    //线程对象

void PrepareFrame( ObjThread *objThread, ObjClosure *objClosure, Value *stackStart );
ObjThread *NewObjThread( VM *vm, ObjClosure *objClosure );
void ResetThread( ObjThread *objThread, ObjClosure *objClosure );

#endif //VANTIDEL_OBJ_THREAD_H
