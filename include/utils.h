
// @description: 声明&定义 相关数据类型 Buffer 宏函数、Struct
//               声明 内存控制相关函数、宏
//               声明 错误&异常处理相关函数、宏、枚举


#ifndef SPARROW_UTILS_H
#define SPARROW_UTILS_H

/* ~ INCLUDE ~ */
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include "vm.h"
#include "parser.h"
#include "common.h"

/* ~ MEM Manager ~ */
void *MemManager(VM *vm, void *ptr, uint32_t oldSize, uint32_t newSize);

// 分配 TYPE 类型的内存块
#define ALLOCATE(vmPtr, type) \
   (type*)MemManager(vmPtr, NULL, 0, sizeof(type))

// 分配 除主类型 mainType 外还需要额外分配的内存块 -> sizeof(mainType) + extraSize
#define ALLOCATE_EXTRA(vmPtr, mainType, extraSize) \
   (mainType*)MemManager(vmPtr, NULL, 0, sizeof(mainType) + extraSize)

// 为数组分配内存
#define ALLOCATE_ARRAY(vmPtr, type, count) \
   (type*)MemManager(vmPtr, NULL, 0, sizeof(type) * count)

// 释放数组的内存
#define DEALLOCATE_ARRAY(vmPtr, arrayPtr, count) \
   MemManager(vmPtr, arrayPtr, sizeof(arrayPtr[0]) * count, 0)

// 释放内存
#define DEALLOCATE(vmPtr, memPtr) \
   MemManager(vmPtr, memPtr, 0, 0)

uint32_t CeilToPowerOf2(uint32_t v);

/* ~DATA BUFFER~ */
// String 类型
typedef struct {
    char *str;
    uint32_t length;
} String;

// 字符串缓冲区
typedef struct {
    uint32_t length;
    char start[];   // 柔性数组
} CharValue;

#define DEFAULT_BUfFER_SIZE 512

// type##Buffer 声明
#define DECLARE_BUFFER_TYPE(type)   \
    typedef struct{                 \
        /* 数据缓冲区 */             \
        type *datas;                \
        /*缓冲区中已使用的元素个数*/   \
        uint32_t count;             \
        /*缓冲区元素总数*/            \
        uint32_t capacity;          \
     }type##Buffer;                 \
    void type##BufferInit(type##Buffer* buf);                                            \
    void type##BufferFillWrite(VM* vm, type##Buffer* buf, type data, uint32_t fillCount);\
    void type##BufferAdd(VM* vm, type##Buffer* buf, type data);                          \
    void type##BufferClear(VM* vm, type##Buffer* buf);

// type##Buffer 定义
#define DEFINE_BUFFER_METHOD(type) \
   /** type##BufferInit
    * 初始化 Buffer
    * @param buf Buffer 指针
    */\
    void type##BufferInit(type##Buffer* buf) {\
        buf->datas = NULL;\
        buf->count = buf->capacity = 0;\
    }\
    \
    /** type##BufferFillWrite
     * 向 BUffer 写入 fillCount 个类型为 type 的数据 data
     * @param vm VM 指针
     * @param buf Buffer 指针
     * @param data 需要被写入的 type 类型数据
     * @param fillCount 数据的个数
     */\
    void type##BufferFillWrite(VM* vm, type##Buffer* buf, type data, uint32_t fillCount) {\
        uint32_t newCounts = buf->count + fillCount;\
        if (newCounts > buf->capacity) {\
            size_t oldSize = buf->capacity * sizeof(type);\
            buf->capacity = CeilToPowerOf2(newCounts);\
            size_t newSize = buf->capacity * sizeof(type);\
            ASSERT(newSize > oldSize, "faint...memory allocate!");\
            buf->datas = (type*)MemManager(vm, buf->datas, oldSize, newSize);\
        }\
        uint32_t cnt = 0;\
        while (cnt < fillCount) {\
            buf->datas[buf->count++] = data;\
            cnt++;\
        }\
    }\
    \
    /** type##BufferAdd
     * 向 Buffer 写入一个 type 类型的 data
     * @param vm VM 指针
     * @param buf Buffer 指针
     * @param data 需要被写入的 type 类型数据
     */\
    void type##BufferAdd(VM* vm, type##Buffer* buf, type data) {\
        type##BufferFillWrite(vm, buf, data, 1);\
    }\
    \
    /** type##BufferClear
     * 清空 Buffer
     * @param vm VM 指针
     * @param buf Buffer 指针
     */\
    void type##BufferClear(VM* vm, type##Buffer* buf) {\
        size_t oldSize = buf->capacity * sizeof(buf->datas[0]);\
        MemManager(vm, buf->datas, oldSize, 0);\
        type##BufferInit(buf);\
    }

// 声明&定义相关 DATA Buffer TYPE
#define SymbolTable StringBuffer
typedef uint8_t Byte;
typedef char Char;
typedef int Int;

DECLARE_BUFFER_TYPE(String)

DECLARE_BUFFER_TYPE(Int)

DECLARE_BUFFER_TYPE(Char)

DECLARE_BUFFER_TYPE(Byte)

/* ~ ERROR ~ */

typedef enum {
    ERROR_IO,
    ERROR_MEM,
    ERROR_LEX,
    ERROR_COMPILE,
    ERROR_RUNTIME
} ErrorType;

void ErrorReport(void *parser, ErrorType errorType, const char *fmt, ...);

void SymbolTableClear(VM *, SymbolTable *buffer);

// 相关报错的宏定义
#define IO_ERROR(...)\
    ErrorReport(NULL, ERROR_IO, __VA_ARGS__)

#define MEM_ERROR(...)\
    ErrorReport(NULL, ERROR_MEM, __VA_ARGS__)

#define LEX_ERROR(parser, ...)\
    ErrorReport(parser, ERROR_LEX, __VA_ARGS__)

#define COMPILE_ERROR(parser, ...)\
    ErrorReport(parser, ERROR_COMPILE, __VA_ARGS__)

#define RUN_ERROR(...)\
    ErrorReport(NULL, ERROR_RUNTIME, __VA_ARGS__)


#endif //SPARROW_UTILS_H
