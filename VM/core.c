
// @description:                           //

/* ~ INCLUDE ~ */
#include <string.h>
#include <sys/stat.h>
#include "utils.h"
#include "vm.h"
#include "core.h"

char* rootDir = NULL;   //根目录

/** ReadFile
 * 读取文件中源代码
 * @param path
 * @return
 */
char* ReadFile(const char* path){
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        IO_ERROR("Could`t open file \"%s\".\n", path);
    }

    // 检查文件
    struct stat fileStat;
    stat(path, &fileStat);

    size_t fileSize = fileStat.st_size;     // 文件大小
    char* fileContent = (char*)malloc(fileSize + 1);    // 存放文件内容的变量
    if (fileContent == NULL) {
        MEM_ERROR("Could`t allocate memory for reading file \"%s\".\n", path);
    }

    // 将文件内容写入 fileContent
    size_t numRead = fread(fileContent, sizeof(char), fileSize, file);
    if (numRead < fileSize) {
        IO_ERROR("Could`t read file \"%s\".\n", path);
    }
    fileContent[fileSize] = '\0';

    fclose(file);
    return fileContent;
}
