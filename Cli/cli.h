#ifndef _CLI_CLI_H
#define _CLI_CLI_H

#include <stdio.h>
#include <string.h>
#include "linenoise.h"

#define VERSION "0.1.1"
#define MAX_LINE_LEN 1024

void LOGO() {

    char *logWord = "                         11111111111111111111\r\n"
                    "                    111111i                1111111\r\n"
                    "       :1        1111i                          11111\r\n"
                    "       11     1111                                 :111i\r\n"
                    "\r\n"
                    "  111111111111     111,       111,            111   11111,  111,   11111111111,\r\n"
                    "      1111                    111,  111111111  11 11,               11      11,\r\n"
                    "      1111         111, ;11111111,  11     11  111,         111,    1111111111,\r\n"
                    "      1111         111, 111,  111,  111111111  11,          111,    11,\r\n"
                    "      1111         111, 111,  111,  111        11,          111,    11,\r\n"
                    "      ;;;;         111,  11111111,  i11111111  11,          111,    11,\r\n"
                    "\r\n"
                    "  1, ,111111111111:                                     .:i11111111111i,\r\n"
                    "1111111             111.                 .;11111111111111111111111111111111111i\r\n"
                    "                       i111111111111111111111111111111111111111111111\r\n"
                    "                                 .....                           ,:\r\n";
    printf("%s", logWord);

}

/* 添加自动补全信息 */
void completion(const char *buf, linenoiseCompletions *lc) {
    #include "AutoCom_KeyWord.inc"
}

/* 命令提示内容显示设置 */
char *hints(const char *buf, int *color, int *bold) {
    #include "AutoCom_Hint.inc"
    return NULL;
}

#endif
