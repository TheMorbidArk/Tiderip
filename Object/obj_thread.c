//
// Created by TheMorbidArk on 2022/10/2.
//

#include "obj_thread.h"
#include "vm.h"

/** PrepareFrame
 * 为运行函数准备桢栈
 * @param objThread ObjThread
 * @param objClosure 运行在线程上的闭包对象
 * @param stackStart 闭包函数堆栈起始处
 */
void PrepareFrame( ObjThread *objThread, ObjClosure *objClosure, Value *stackStart )
{
	ASSERT( objThread->frameCapacity > objThread->usedFrameNum, "frame not enough!!" );
	Frame *frame = &( objThread->frames[ objThread->usedFrameNum++ ] );

	// thread中的各个frame是共享thread的stack
	// frame用frame->stackStart指向各自frame在thread->stack中的起始地址
	frame->stackStart = stackStart;
	frame->closure = objClosure;
	frame->ip = objClosure->fn->instrStream.datas;
}

/** ResetThread
 * 重置thread,
 * @param objThread
 * @param objClosure
 */
void ResetThread( ObjThread *objThread, ObjClosure *objClosure )
{
	objThread->esp = objThread->stack;
	objThread->openUpvalues = NULL;
	objThread->caller = NULL;
	objThread->errorObj = VT_TO_VALUE( VT_NULL );
	objThread->usedFrameNum = 0;

	ASSERT( objClosure != NULL, "objClosure is NULL in function resetThread" );
	PrepareFrame( objThread, objClosure, objThread->stack );
}

/** NewObjThread
 * 新建线程
 * @param vm
 * @param objClosure
 * @return
 */
ObjThread *NewObjThread( VM *vm, ObjClosure *objClosure )
{
	ASSERT( objClosure != NULL, "objClosure is NULL!" );

	Frame *frames = ALLOCATE_ARRAY( vm, Frame, INITIAL_FRAME_NUM );

	uint32_t stackCapacity = CeilToPowerOf2( objClosure->fn->maxStackSlotUsedNum + 1 );
	Value *newStack = ALLOCATE_ARRAY( vm, Value, stackCapacity );

	ObjThread *objThread = ALLOCATE( vm, ObjThread );
	InitObjHeader( vm, &objClosure->objHeader, OT_THREAD, vm->threadClass );

	objThread->frames = frames;
	objThread->frameCapacity = INITIAL_FRAME_NUM;
	objThread->stack = newStack;
	objThread->stackCapacity = stackCapacity;

	ResetThread( objThread, objClosure );
	return objThread;
}