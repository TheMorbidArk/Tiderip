
// @description:                           //

#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "vm.h"
#include "core.h"
#include "cli.h"

void RunFile(const char *path) {
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash != NULL) {
        char *root = (char *) malloc(lastSlash - path + 2);
        memcpy(root, path, lastSlash - path + 1);
        root[lastSlash - path + 1] = '\0';
        rootDir = root;
    }

    VM *vm = NewVM();
    const char *sourceCode = ReadFile(path);
    struct parser parser;
    InitParser(vm, &parser, path, sourceCode);

#include "token.list"

    while (parser.curToken.type != TOKEN_EOF) {
        GetNextToken(&parser);
        printf("%dL: %s [", parser.curToken.lineNo, tokenArray[parser.curToken.type]);
        uint32_t idx = 0;
        while (idx < parser.curToken.length) {
            printf("%c", *(parser.curToken.start + idx++));
        }
        printf("]\n");
    }

}

int main(int argc, const char **argv) {
    if (argc == 1) { ;
    } else {
        RunFile(argv[1]);
    }
    return 0;
}
