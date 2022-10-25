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
		char *root = (char *)malloc(lastSlash - path + 2);
		memcpy(root, path, lastSlash - path + 1);
		root[lastSlash - path + 1] = '\0';
		rootDir = root;
	}

	VM *vm = newVM();
	const char *sourceCode = readFile(path);
	executeModule(vm, OBJ_TO_VALUE(newObjString(vm, path, strlen(path))), sourceCode);
}

int main(int argc, const char **argv) {

	clock_t start_t,finish_t;

	if (argc == 1) { ;
	} else {

		printf("-----------------------------\r\n");
		printf(" Welcome to VanTideL v_0.1.0 \r\n");
		printf("-----------------------------\r\n");

		start_t = clock();

		runFile(argv[1]);

		finish_t = clock();

		printf("CPU 占用的总时间：%f\n", (double)(finish_t - start_t) / CLOCKS_PER_SEC);
	}
	return 0;
}
