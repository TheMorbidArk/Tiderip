
#include <stdio.h>
#include <string.h>
#include "cli.h"
#include "parser.h"
#include "vm.h"
#include "core.h"

//执行脚本文件, 输出Token信息
static void RunFile_Token(const char *path) {
	const char *lastSlash = strrchr(path, '/');
	if (lastSlash != NULL) {
		char *root = (char *)malloc(lastSlash - path + 2);
		memcpy(root, path, lastSlash - path + 1);
		root[lastSlash - path + 1] = '\0';
		rootDir = root;
	}

	VM *vm = NewVM();
	const char *sourceCode = ReadFile(path);

	struct parser parser;
	InitParser(vm, &parser, path, sourceCode, NULL);  //此NULL是临时的

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

void RunFile(const char *path) {
	const char *lastSlash = strrchr(path, '/');
	if (lastSlash != NULL) {
		char *root = (char *)malloc(lastSlash - path + 2);
		memcpy(root, path, lastSlash - path + 1);
		root[lastSlash - path + 1] = '\0';
		rootDir = root;
	}

	VM *vm = NewVM();
	const char *sourceCode = ReadFile(path);
	ExecuteModule(vm, OBJ_TO_VALUE(NewObjString(vm, path, strlen(path))), sourceCode);

}

int main(int argc, const char **argv) {
	if (argc == 1) {
		;
	} else {
		RunFile(argv[1]);
	}
	return 0;
}
