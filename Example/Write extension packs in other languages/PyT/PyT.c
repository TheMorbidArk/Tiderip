// @description:                           //

#include <string.h>
#include "utils.h"
#include "class.h"
#include "core.h"

#include "PyT.h"

#include <Python.h>

bool primPyT(VM *vm, Value *args){
    Py_Initialize();
    PyRun_SimpleString("print('Hello World')");
    Py_Finalize();
    RET_NULL
}

void extenPyTBind(VM *vm, ObjModule *coreModule){
    Class *pytClass = VALUE_TO_CLASS(getCoreClassValue(coreModule, "PyT"));
    PRIM_METHOD_BIND(pytClass->objHeader.class, "pyHello_()", primPyT);
}