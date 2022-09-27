//
// Created by TheMorbidArk on 2022/9/25.
//

#include "obj_range.h"
#include "utils.h"
#include "class.h"
#include "vm.h"

/** NewObjRange
 * 新建range对象
 * @param vm VM 指针
 * @param from
 * @param to
 * @return ObjRange -> Range 对象
 */
ObjRange *NewObjRange(VM *vm, int from, int to) {
    ObjRange* objRange = ALLOCATE(vm, ObjRange);
    InitObjHeader(vm, &objRange->objHeader, OT_RANGE, vm->rangeClass);
    objRange->from = from;
    objRange->to = to;
    return objRange;
}
