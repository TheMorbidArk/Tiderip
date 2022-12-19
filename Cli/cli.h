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
    /* TODO 添加关键字及内置函数自动补全信息 */
    if (buf[0] == 'T') {
        linenoiseAddCompletion(lc, "Tide");
    } else if (buf[0] == 'f') {
        linenoiseAddCompletion(lc, "false");
        linenoiseAddCompletion(lc, "for");
        linenoiseAddCompletion(lc, "fun");
    } else if (buf[0] == 'i') {
        linenoiseAddCompletion(lc, "if");
        linenoiseAddCompletion(lc, "is");
        linenoiseAddCompletion(lc, "import");
    } else if (buf[0] == 's') {
        linenoiseAddCompletion(lc, "static");
        linenoiseAddCompletion(lc, "super");
    } else if (buf[0] == 'e') {
        linenoiseAddCompletion(lc, "elif");
        linenoiseAddCompletion(lc, "else");
    } else if (buf[0] == 't') {
        linenoiseAddCompletion(lc, "true");
        linenoiseAddCompletion(lc, "this");
    } else if (buf[0] == 'w') {
        linenoiseAddCompletion(lc, "while");
    } else if (buf[0] == 'b') {
        linenoiseAddCompletion(lc, "break");
    } else if (buf[0] == 'c') {
        linenoiseAddCompletion(lc, "continue");
        linenoiseAddCompletion(lc, "class");
    } else if (buf[0] == 'r') {
        linenoiseAddCompletion(lc, "return");
    } else if (buf[0] == 'n') {
        linenoiseAddCompletion(lc, "null");
    }
    /* TODO 添加变量及函数实时自动补全 */
}

/* 命令提示内容显示设置 */
char *hints(const char *buf, int *color, int *bold) {
    /* TODO 添加命令提示信息 */

    /**
     * Tide <Name> = <Value>
     * if (E) {S}
     * elif
     * else {}
     * for <IndexName> (ValueName) {}
     * fun <FunName>(Arguments){}
     * import <ImportName>
     * class <ClassName> {}
     */

    // 如果命令为 hello
    if (!strcasecmp(buf, "Tide")) {
        // 命令字体颜色
        *color = 35;
        // 命令字体样式
        *bold = 0;
        // 提示内容
        return " <Name> = <Value>";
    } else if (!strcasecmp(buf, "fun")) {
        // 命令字体颜色
        *color = 35;
        // 命令字体样式
        *bold = 0;
        // 提示内容
        return " <FunName>(Arguments){}";
    }
    return NULL;
}

#endif
