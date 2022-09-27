//
// Created by TheMorbidArk on 2022/9/25.
//

#include "obj_list.h"

/** NewObjList
 * 新建一个List
 * @param vm VM 指针
 * @param elementNum List 容量
 * @return ObjList -> List对象
 */
ObjList *NewObjList(VM *vm, uint32_t elementNum) {
    Value *elementArray = NULL;
    if(elementNum > 0){
        elementArray = ALLOCATE_ARRAY(vm, Value, elementNum);
    }
    ObjList * objList = ALLOCATE(vm, ObjList);

    objList->elements.datas = elementArray;
    objList->elements.capacity = objList->elements.count = elementNum;
    InitObjHeader(vm, &objList->objHeader, OT_LIST, vm->listClass);
    return objList;
}

/** ShrinkList
 * 调整list容量
 * @param vm VM 指针
 * @param objList 需要被调整的 List 指针
 * @param newCapacity 调整后的容量
 */
static void ShrinkList(VM* vm, ObjList* objList, uint32_t newCapacity) {
    uint32_t oldSize = objList->elements.capacity * sizeof(Value);
    uint32_t newSize = newCapacity * sizeof(Value);
    MemManager(vm, objList->elements.datas, oldSize, newSize);
    objList->elements.capacity = newCapacity;
}

/** RemoveElement
 * 删除list中索引为index处的元素,即删除list[index]
 * @param vm VM 指针
 * @param objList 需要被调整的 List 指针
 * @param index 索引
 * @return 被删除的 value
 */
Value RemoveElement(VM *vm, ObjList *objList, uint32_t index) {
    Value valueRemoved = objList->elements.datas[index];

    //使index后面的元素前移一位,覆盖index处的元素
    uint32_t idx = index;
    while (idx < objList->elements.count) {
        objList->elements.datas[idx] = objList->elements.datas[idx + 1];
        idx++;
    }

    //若容量利用率过低就减小容量
    uint32_t _capacity = objList->elements.capacity / CAPACITY_GROW_FACTOR;
    if (_capacity > objList->elements.count) {
        ShrinkList(vm, objList, _capacity);
    }

    objList->elements.count--;
    return valueRemoved;
}

/** InsertElement
 * 在objlist中索引为index处插入value, 类似于list[index] = value
 * @param vm VM 指针
 * @param objList 需要被调整的 List 指针
 * @param index 索引
 * @param value 需要插入的Value
 */
void InsertElement(VM *vm, ObjList *objList, uint32_t index, Value value) {
    if(index > objList->elements.count-1){
        RUN_ERROR("index out bounded!");
    }

    //准备一个Value的空间以容纳新元素产生的空间波动
    //即最后一个元素要后移1个空间
    ValueBufferAdd(vm, &objList->elements, VT_TO_VALUE(VT_NULL));

    //使index后面的元素整体后移一位
    uint32_t idx = objList->elements.count - 1;
    while (idx > index) {
        objList->elements.datas[idx] = objList->elements.datas[idx - 1];
        idx--;
    }

    //在index处插入数值
    objList->elements.datas[index] = value;
}
