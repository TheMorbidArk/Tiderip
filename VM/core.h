
// @description:                           //

#ifndef SPARROW_CORE_H
#define SPARROW_CORE_H
#include "vm.h"

extern char *rootDir;   // extern -> Cli.c 根目录
char *ReadFile( const char *sourceFile );
VMResult ExecuteModule(VM* vm, Value moduleName, const char* moduleCode);
void BuildCore(VM* vm);

#endif //SPARROW_CORE_H
