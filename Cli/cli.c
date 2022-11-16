#include "cli.h"
#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "vm.h"
#include "core.h"
#include<time.h>

//执行脚本文件
static void runFile(const char *path) {
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL) {
        char *root = (char *) malloc(lastSlash - path + 2);
        memcpy(root, path, lastSlash - path + 1);
        root[lastSlash - path + 1] = '\0';
        rootDir = root;
    }

    VM *vm = newVM();
    const char *sourceCode = readFile(path);
    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, path, strlen(path))), sourceCode);
}

void LOGO() {

    char *logWord = LOG_VERSION;
    printf("%s", logWord);

}

int main(int argc, const char **argv) {

    clock_t start_t, finish_t;

    if (argc == 1) {    // Cli 交互式运行
        LOGO();
        printf("Nothing in There!");
    } else if (argc == 2) {      // Cli 程序
        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
            printf("VanTideL Version:%s\r\n", VERSION);
            return 0;
        } else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {

            printf("VanTideL is a script language interpreter based on C language implementation.\r\n"
                   "This interpreter can only be used to compile Tide.\r\n"
                   "This interpreter runs by stack-based Virtual Machines and Top-Down Operator Implementations First(TDOP).\r\n"
                   "Refer to Project <sparrow> for Grammar & VM opcode.\r\n"
                   "\r\n"
                   "Usage: VanTideL [options] <FileName.vt>\r\n"
                   "\r\n"
                   "Options:\r\n"
                   "        -v, --version                   Display compiler version information.\r\n"
                   "        -h, --help                      Display this information.\r\n"
                   "        -d, -D, --debug, --DEBUG        Enter debug mode.\r\n"
                   "\r\n"
                   "Examples:\r\n"
                   "        ./VanTideL samlp.vt             Running the source code from samlp.vt.\r\n"
                   "        ./VanTideL -d samlp.vt          Run the source code for samlp.vt in DEBUG mode.\r\n"
                   "        ./VanTideL -v                   Display compiler version information.\r\n"
                   "        ./VanTideL -h                   Display Help Document & Some information about this compiler.\r\n"
                   "\r\n");

            return 0;
        } else {
            /* 开始编译 */
            LOGO();
            start_t = clock();
            runFile(argv[1]);
            finish_t = clock();
            printf("> CPU Run Time: %lfs <\r\n", (double) (finish_t - start_t) / CLOCKS_PER_SEC);
        }
    } else if (argc == 3) {    // 进入Debug模式
        if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--debug") || !strcmp(argv[1], "-D") || !strcmp(argv[1], "--DEBUG")) {
            /* TODO 进入DEBUG模式进行编译 */
            printf("DEBUG Mode Not Done,You can Update the program\r\n");
            return 0;
        }
    }

    return 0;
}
