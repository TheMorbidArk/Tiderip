
// @description: 定义 内存控制相关函数和宏
//               定义 错误&异常处理相关函数和宏

/* ~INCLUDE~ */

#include "utils.h"
#include "vm.h"
#include "parser.h"
#include <stdlib.h>
#include <stdarg.h>

/* ~ Functions ~ */

/** MemManager
 * 内存管理:
 * 输入期望分配内存块大小，已分配内存块大小，内存块地址
 * 进行内存块大小重新分配
 * 返回 realloc 后的内存块指针
 * @param vm 虚拟机
 * @param ptr 指向被修改内存块的指针
 * @param oldSize 之前分配的内存空间大小
 * @param newSize 期望分配的内存大小
 * @return 指向被修改内存块的指针
 */
void *MemManager(VM *vm, void *ptr, uint32_t oldSize, uint32_t newSize) {
    vm->allocatedBytes += newSize - oldSize;
    if (newSize == 0) {
        free(ptr);
        return NULL;
    }
    return realloc(ptr, newSize);
}

/** CeilToPowerOf2
 * 返回大于等于v最近的2次幂
 * v |= v >> n （n = 1,2,4,8,16）
 * 最终 v.bit(v.Value -> 二进制) = VB = 1111(VB的每一位均为1)
 * 2^n = VB+1
 * @param v
 * @return (x = 2^n) >= v
 */

uint32_t CeilToPowerOf2(uint32_t v) {
    v += (v == 0);
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

DEFINE_BUFFER_METHOD(String)

DEFINE_BUFFER_METHOD(Int)

DEFINE_BUFFER_METHOD(Char)

DEFINE_BUFFER_METHOD(Byte)

/** SymbolTableClear
 *
 * @param vm VM 指针
 * @param buffer Buffer 指针
 */
void SymbolTableClear(VM *vm, SymbolTable *buffer) {
    uint32_t idx = 0;
    while (idx < buffer->count) {
        MemManager(vm, buffer->datas[idx++].str, 0, 0);
    }
    StringBufferClear(vm, buffer);
}

/** ErrorReport
 * 通用错误类型处理
 * 输入 错误类型, 错误信息
 * 根据错误类型, 输出相应错误信息
 * @param parser 词法分析器
 * @param errorType 错误类型
 * @param fmt buffer 参数 -> 错误信息
 * @return void
 */
void ErrorReport(void *parser, ErrorType errorType, const char *fmt, ...) {
    // buffer 存储 fmt ... 错误信息
    char buffer[DEFAULT_BUfFER_SIZE] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, DEFAULT_BUfFER_SIZE, fmt, ap);
    va_end(ap);

    switch (errorType) {
        case ERROR_IO:
            // 文件IO ERROR
        case ERROR_MEM:
            // 内存分配 ERROR
            fprintf(stderr, "%s:%d In function %s():%s\n",
                    __FILE__, __LINE__, __func__, buffer);
            break;
        case ERROR_LEX:
            // 词法分析器 ERROR
        case ERROR_COMPILE:
            // 编译 ERROR
            ASSERT(parser != NULL, "Parser is null!");
            fprintf(stderr, "%s:%d \"%s\"\n", ((Parser *) parser)->file,
                    ((Parser *) parser)->preToken.lineNo, buffer);
            break;
        case ERROR_RUNTIME:
            // VM运行时 ERROR
            fprintf(stderr, "%s\n", buffer);
            break;
        default:
            NOT_REACHED();
    }
    exit(1);
}

