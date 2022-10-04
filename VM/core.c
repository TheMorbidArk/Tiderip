
// @description:                           //

/* ~ INCLUDE ~ */
#include "core.h"
#include <string.h>
#include <sys/stat.h>
#include "utils.h"
#include "vm.h"

char *rootDir = NULL;   //根目录
#define CORE_MODULE VT_TO_VALUE(VT_NULL)

/**
 * 执行模块 -> 一个源文件被视为一个模块
 * @param vm
 * @param moduleName
 * @param moduleCode
 * @return
 */
VMResult ExecuteModule( VM *vm, Value moduleName, const char *moduleCode )
{
	return VM_RESULT_ERROR;
}

void BuildCore( VM *vm )
{
	// 创建核心模块,录入到vm->allModules
	ObjModule* coreModule = NewObjModule(vm, NULL); // NULL为核心模块.name
	MapSet(vm, vm->allModules, CORE_MODULE, OBJ_TO_VALUE(coreModule));
}

/** ReadFile
 * 读取文件中源代码
 * @param path
 * @return
 */
char *ReadFile( const char *path )
{
	FILE *file = fopen( path, "r" );
	if ( file == NULL)
	{
		IO_ERROR( "Could`t open file \"%s\".\n", path );
	}

	// 检查文件
	struct stat fileStat;
	stat( path, &fileStat );

	size_t fileSize = fileStat.st_size;     // 文件大小
	char *fileContent = ( char * )malloc( fileSize + 1 );    // 存放文件内容的变量
	if ( fileContent == NULL)
	{
		MEM_ERROR( "Could`t allocate memory for reading file \"%s\".\n", path );
	}

	// 将文件内容写入 fileContent
	size_t numRead = fread( fileContent, sizeof( char ), fileSize, file );
	if ( numRead < fileSize )
	{
		IO_ERROR( "Could`t read file \"%s\".\n", path );
	}
	fileContent[ fileSize ] = '\0';

	fclose( file );
	return fileContent;
}
