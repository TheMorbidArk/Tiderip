
// @description: 预定义类型 & DEBUG相关宏   //

#ifndef SPARROW_COMMON_H
#define SPARROW_COMMON_H

/* ~INCLUDE~ */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* ~Predefined(预定义)~ */
typedef struct vm VM;
typedef struct parser Parser;
typedef struct class Class;

/* ~DEFINE~ */
// 定义 BOOL 类型变量
#define bool char
#define true 1
#define false 0

#define UNUSED __attribute__ ((unused)) // 关闭 GCC 不进行 Warning

/** DEGUG
 * 判断是否定义DEBUG宏
 * 若定义，则开启DEBUG功能
 * 若未定义，则返回 (viod)0
 */
#ifdef DEBUG
#define ASSERT(condition, errMsg) \
      do {\
     if (!(condition)) {\
        fprintf(stderr, "ASSERT failed! %s:%d In function %s(): %s\n", \
           __FILE__, __LINE__, __func__, errMsg); \
        abort();\
     }\
      } while (0);
#else
#define ASSERT(condition, errMsg) ((void)0)
#endif

/** NOT_REACHED
 * 不可能到达的程序
 * 某些程序分支不可达，若被执行到则触发宏 NOT_REACHED
 */
#define NOT_REACHED()\
   do {\
      fprintf(stderr, "NOT_REACHED: %s:%d In function %s()\n", \
     __FILE__, __LINE__, __func__);\
      while (1);\
   } while (0);

#endif //SPARROW_COMMON_H
