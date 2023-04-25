#include "cli.h"
#include <string.h>
#include "parser.h"
#include "vm.h"
#include "core.h"
#include <time.h>

Value moduleName;
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
    moduleName = OBJ_TO_VALUE(newObjString(vm, path, strlen(path)));
    executeModule(vm, moduleName, sourceCode);
}

void runText(){
    printf("Tiderip Version:%s\r\n", VERSION);
    VM *vm = newVM();
       
    char *line = "System.println(\"Hello World !!!\")";

    executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), line);

    return ;
}

//运行命令行
static void runCli(void)
{
    VM *vm = newVM();
    printf("Tiderip Version:%s\r\n", VERSION);
    
    char *line  = (char*)malloc(sizeof(char) * 255);
    // 命令动态监控
    while (line != NULL)
    {
        printf(">>> ");   
        if (!strncmp(line, "exit", 4))
        {
            exit(0);
        }
        
        executeModule(vm, OBJ_TO_VALUE(newObjString(vm, "cli", 3)), line);
        free(line);
    }
}
