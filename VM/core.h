
// @description:                           //

#ifndef SPARROW_CORE_H
#define SPARROW_CORE_H
#include "vm.h"

extern char *rootDir;   // extern -> Cli.c 根目录
char *ReadFile( const char *sourceFile );
VMResult ExecuteModule(VM* vm, Value moduleName, const char* moduleCode);
int GetIndexFromSymbolTable(SymbolTable* table, const char* symbol, uint32_t length);
int AddSymbol(VM* vm, SymbolTable* table, const char* symbol, uint32_t length);

void BuildCore(VM* vm);
void BindMethod(VM* vm, Class* class, uint32_t index, Method method);
void BindSuperClass(VM* vm, Class* subClass, Class* superClass);

int EnsureSymbolExist(VM* vm, SymbolTable* table, const char* symbol, uint32_t length);

#endif //SPARROW_CORE_H
