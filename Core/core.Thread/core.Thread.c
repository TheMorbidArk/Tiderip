//
// Created by MorbidArk on 2023/3/17.
//

#include <string.h>
#include "core.Thread.h"
#include "core.h"

//Thread.new(func):创建一个thread实例
static bool primThreadNew(VM *vm, Value *args)
{
    //代码块为参数必为闭包
    if (!validateFn(vm, args[1]))
    {
        return false;
    }
    
    ObjThread *objThread = newObjThread(vm, VALUE_TO_OBJCLOSURE(args[1]));
    
    //使stack[0]为接收者,保持栈平衡
    objThread->stack[0] = VT_TO_VALUE(VT_NULL);
    objThread->esp++;
    RET_OBJ(objThread);
}

//Thread.abort(err):以错误信息err为参数退出线程
static bool primThreadAbort(VM *vm, Value *args)
{
    //此函数后续未处理,暂时放着
    vm->curThread->errorObj = args[1]; //保存退出参数
    return VALUE_IS_NULL(args[1]);
}

//Thread.current:返回当前的线程
static bool primThreadCurrent(VM *vm, Value *args UNUSED)
{
    RET_OBJ(vm->curThread);
}

//Thread.suspend():挂起线程,退出解析器
static bool primThreadSuspend(VM *vm, Value *args UNUSED)
{
    //目前suspend操作只会退出虚拟机,
    //使curThread为NULL,虚拟机将退出
    vm->curThread = NULL;
    return false;
}

//Thread.yield(arg)带参数让出cpu
static bool primThreadYieldWithArg(VM *vm, Value *args)
{
    ObjThread *curThread = vm->curThread;
    vm->curThread = curThread->caller;   //使cpu控制权回到主调方
    
    curThread->caller = NULL;  //与调用者断开联系
    
    if (vm->curThread != NULL)
    {
        //如果当前线程有主调方,就将当前线程的返回值放在主调方的栈顶
        vm->curThread->esp[-1] = args[1];
        
        //对于"thread.yield(arg)"来说, 回收arg的空间,
        //保留thread参数所在的空间,将来唤醒时用于存储yield结果
        curThread->esp--;
    }
    return false;
}

//Thread.yield() 无参数让出cpu
static bool primThreadYieldWithoutArg(VM *vm, Value *args UNUSED)
{
    ObjThread *curThread = vm->curThread;
    vm->curThread = curThread->caller;   //使cpu控制权回到主调方
    
    curThread->caller = NULL;  //与调用者断开联系
    
    if (vm->curThread != NULL)
    {
        //为保持通用的栈结构,如果当前线程有主调方,
        //就将空值做为返回值放在主调方的栈顶
        vm->curThread->esp[-1] = VT_TO_VALUE(VT_NULL);
    }
    return false;
}

//切换到下一个线程nextThread
static bool switchThread(VM *vm,
    ObjThread *nextThread, Value *args, bool withArg)
{
    //在下一线程nextThread执行之前,其主调线程应该为空
    if (nextThread->caller != NULL)
    {
        RUN_ERROR("thread has been called!");
    }
    nextThread->caller = vm->curThread;
    
    if (nextThread->usedFrameNum == 0)
    {
        //只有已经运行完毕的thread的usedFrameNum才为0
        SET_ERROR_FALSE(vm, "a finished thread can`t be switched to!");
    }
    
    if (!VALUE_IS_NULL(nextThread->errorObj))
    {
        //Thread.abort(arg)会设置errorObj, 不能切换到abort的线程
        SET_ERROR_FALSE(vm, "a aborted thread can`t be switched to!");
    }
    
    //如果call有参数,回收参数的空间,
    //只保留次栈顶用于存储nextThread返回后的结果
    if (withArg)
    {
        vm->curThread->esp--;
    }
    
    ASSERT(nextThread->esp > nextThread->stack, "esp should be greater than stack!");
    //nextThread.call(arg)中的arg做为nextThread.yield的返回值
    //存储到nextThread的栈顶,否则压入null保持栈平衡
    nextThread->esp[-1] = withArg ? args[1] : VT_TO_VALUE(VT_NULL);
    
    //使当前线程指向nextThread,使之成为就绪
    vm->curThread = nextThread;
    
    //返回false以进入vm中的切换线程流程
    return false;
}

//objThread.call()
static bool primThreadCallWithoutArg(VM *vm, Value *args)
{
    return switchThread(vm, VALUE_TO_OBJTHREAD(args[0]), args, false);
}

//objThread.call(arg)
static bool primThreadCallWithArg(VM *vm, Value *args)
{
    return switchThread(vm, VALUE_TO_OBJTHREAD(args[0]), args, true);
}

//objThread.isDone返回线程是否运行完成
static bool primThreadIsDone(VM *vm UNUSED, Value *args)
{
    //获取.isDone的调用者
    ObjThread *objThread = VALUE_TO_OBJTHREAD(args[0]);
    RET_BOOL(objThread->usedFrameNum == 0 || !VALUE_IS_NULL(objThread->errorObj));
}

void coreThreadBind(VM *vm, ObjModule *coreModule)
{
    //Thread类也是在core.script.inc中定义的,
    //将其挂载到vm->threadClass并补充原生方法
    vm->threadClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "Thread"));
    //以下是类方法
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "new(_)", primThreadNew);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "abort(_)", primThreadAbort);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "current", primThreadCurrent);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "suspend()", primThreadSuspend);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "yield(_)", primThreadYieldWithArg);
    PRIM_METHOD_BIND(vm->threadClass->objHeader.class, "yield()", primThreadYieldWithoutArg);
    //以下是实例方法
    PRIM_METHOD_BIND(vm->threadClass, "call()", primThreadCallWithoutArg);
    PRIM_METHOD_BIND(vm->threadClass, "call(_)", primThreadCallWithArg);
    PRIM_METHOD_BIND(vm->threadClass, "isDone", primThreadIsDone);
}