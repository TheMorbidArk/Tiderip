cmake_minimum_required(VERSION 3.8)
project(MySharedLib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
add_definitions(-fgnu89-inline)
add_definitions(-std=c11)

# 定义源文件 *.c
aux_source_directory(Tiderip/include INCLUDE)
aux_source_directory(Tiderip/VM VM)
aux_source_directory(Tiderip/Core CORE)
aux_source_directory(Tiderip/Parser PARSER)
aux_source_directory(Tiderip/Cli CLI)
aux_source_directory(Tiderip/Object OBJECT)
aux_source_directory(Tiderip/Compiler COMPILER)
aux_source_directory(Tiderip/Core/core.System coreSystem)
aux_source_directory(Tiderip/Core/core.Range coreRange)
aux_source_directory(Tiderip/Core/core.Map coreMap)
aux_source_directory(Tiderip/Core/core.List coreList)
aux_source_directory(Tiderip/Core/core.String coreString)
aux_source_directory(Tiderip/Core/core.Num coreNum)
aux_source_directory(Tiderip/Core/core.Null coreNull)
aux_source_directory(Tiderip/Core/core.Function coreFunction)
aux_source_directory(Tiderip/Core/core.Thread coreThread)
aux_source_directory(Tiderip/Core/core.Bool coreBool)


# 显示源文件路径
set(SCR_SOUCES_LIST
        ${INCLUDE} ${VM} ${CORE} ${PARSER} ${CLI} ${OBJECT} ${COMPILER}     # 核心代码
        # core 标准库
        ${coreSystem} ${coreFunction} ${coreNum} ${coreNull} ${coreBool}
        ${coreRange} ${coreThread} ${coreMap} ${coreList} ${coreString}
        )
message("-- [OK] Read the source file completely!")

# 定义头文件 *.h
file(GLOB_RECURSE SCR_HEADER_PATH "*.hpp" "*.h")
message("-- [OK] Read the header file completely!")

include_directories(Tiderip/include Tiderip/VM Tiderip/Core Tiderip/Parser Tiderip/Cli Tiderip/Object Tiderip/Compiler)
message("-- [OK] Set the header file completely!")

LINK_LIBRARIES(m)

#创建共享库
add_library(LibTiderip SHARED ${SCR_SOUCES_LIST} ${SCR_HEADER_PATH} )