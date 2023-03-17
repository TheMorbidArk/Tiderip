#include "cli.h"
#include <string.h>
#include "parser.h"
#include "vm.h"
#include "core.h"
#include <time.h>

#include "linenoise.h"

/* 添加自动补全信息 */
void completion(const char *buf, linenoiseCompletions *lc)
{
    #include "AutoCom_KeyWord.inc"
}

/* 命令提示内容显示设置 */
char *hints(const char *buf, int *color, int *bold)
{
    #include "AutoCom_Hint.inc"
    return NULL;
}

//执行脚本文件
static void runFile(const char *path)
{
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL)
    {
        char *root = (char *)malloc(lastSlash - path + 2);
        memcpy(root, path, lastSlash - path + 1);
        root[lastSlash - path + 1] = '\0';
        rootDir = root;
    }
    
    VM *vm = newVM();
    const char *sourceCode = readFile(path);
    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, path, strlen(path))), sourceCode);
}

//运行命令行
static void runCli(void)
{
    VM *vm = newVM();
    printf("Tiderip Version:%s\r\n", VERSION);
    
    char *line;
    // 设置信息自动补全回调函数
    linenoiseSetCompletionCallback(completion);
    // 设置命令提示内容以及显示样式回调函数
    linenoiseSetHintsCallback(hints);
    // 历史命令加载
    linenoiseHistoryLoad("history.vt");
    // 命令动态监控
    while ((line = linenoise(">>> ")) != NULL)
    {
        
        if (!strncmp(line, "exit", 4))
        {
            exit(0);
        }
        
        executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), line);
        // 添加命令至历史列表
        linenoiseHistoryAdd(line);
        // 保存命令至历史文件
        linenoiseHistorySave("history.vt");
        free(line);
    }
}

int main(int argc, const char **argv)
{
    
    clock_t start_t, finish_t;
    
    if (argc == 1)
    {    // Cli 交互式运行
        LOGO();
        runCli();
    }
    else if (argc == 2)
    {      // Cli 程序
        if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))
        {
            printf("Tiderip Version:%s\r\n", VERSION);
            return 0;
        }
        else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))
        {
            
            printf("Tiderip is a script language interpreter based on C language implementation.\r\n"
                   "This interpreter can only be used to compile Tide.\r\n"
                   "This interpreter runs by stack-based Virtual Machines and Top-Down Operator Implementations First(TDOP).\r\n"
                   "Refer to Project <sparrow> for Grammar & VM opcode.\r\n"
                   "\r\n"
                   "Usage: VanTideL [options] <FileName.vt>\r\n"
                   "\r\n"
                   "Options:\r\n"
                   "        -v, --version                   Display compiler version information.\r\n"
                   "        -h, --help                      Display this information.\r\n"
                   "        -l, --logo                      Display compiler version and LOGO.\r\n"
                   "        -t, --time                      Display compiler result and running times.\r\n"
                   "        -d, -D, --debug, --DEBUG        Enter debug mode.\r\n"
                   "\r\n"
                   "Examples:\r\n"
                   "        ./Tiderip samlp.vt             Running the source code from samlp.vt.\r\n"
                   "        ./Tiderip -d samlp.vt          Run the source code for samlp.vt in DEBUG mode.\r\n"
                   "        ./Tiderip -v                   Display compiler version information.\r\n"
                   "        ./Tiderip -h                   Display Help Document & Some information about this compiler.\r\n"
                   "\r\n");
            
            return 0;
        }
        else if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--logo"))
        {
            LOGO();
        }
        else
        {
            /* 开始编译 */
            runFile(argv[1]);
        }
    }
    else if (argc == 3)
    {    // 进入Debug模式
        if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--debug") || !strcmp(argv[1], "-D") ||
            !strcmp(argv[1], "--DEBUG"))
        {
            /* TODO 进入DEBUG模式进行编译 */
            printf("DEBUG Mode Not Done,You can Update the program\r\n");
            return 0;
        }
        else if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--logo"))
        {
            LOGO();
            /* 开始编译 */
            runFile(argv[2]);
        }
        else if (!strcmp(argv[1], "-t") || !strcmp(argv[1], "--time"))
        {
            /* 开始编译 */
            start_t = clock();
            runFile(argv[2]);
            finish_t = clock();
            printf("> CPU Run Time: %lfs <\r\n", (double)(finish_t - start_t) / CLOCKS_PER_SEC);
        }
    }
    
    return 0;
}
