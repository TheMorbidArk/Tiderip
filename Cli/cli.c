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

void LOGO(){
	char *i = "  ------------------------------------------------------------\n";
	char *a = "||____   ____           ___________.__    .___     .____      ||\n";
	char *b	= "||\\   \\ /   /____    ___\\__    ___/|__| __| _/____ |    |     ||\n";
	char *c = "|| \\   Y   /\\__  \\  /    \\|    |   |  |/ __ |/ __ \\|    |     ||\n";
	char *d = "||  \\     /  / __ \\|   |  \\    |   |  / /_/ \\  ___/|    |___  ||\n";
	char *e = "||   \\___/  (____  /___|  /____|   |__\\____ |\\___  >_______ \\ ||\n";
	char *f = "||               \\/     \\/                 \\/    \\/        \\/ ||\n";
	char *g = "  ------------------------version 0.1.0-----------------------\n";

	printf("%s",i);
	printf("%s",a);
	printf("%s",b);
	printf("%s",c);
	printf("%s",d);
	printf("%s",e);
	printf("%s",f);
	printf("%s",g);
}

int main(int argc, const char **argv) {

	clock_t start_t,finish_t;

	LOGO();

	if (argc == 1) { ;
	} else {

		start_t = clock();

		runFile(argv[1]);

		finish_t = clock();

		printf("> CPU Run Time：%lfs\r\n", (double)(finish_t - start_t) / CLOCKS_PER_SEC);
	}
	return 0;
}
