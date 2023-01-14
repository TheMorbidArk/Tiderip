cmake_minimum_required(VERSION 3.13)
project(Tiderip C)

set(CMAKE_C_STANDARD 11)
add_definitions(-fgnu89-inline)
add_definitions(-std=c11)

# 定义源文件 *.c
aux_source_directory(include INCLUDE)
aux_source_directory(VM VM)
aux_source_directory(Parser PARSER)
aux_source_directory(Cli CLI)
aux_source_directory(Object OBJECT)
aux_source_directory(Compiler COMPILER)
aux_source_directory(Linenoise LINENOISE)
# 显示源文件路径
set(SCR_SOUCES_LIST ${INCLUDE} ${VM} ${PARSER} ${CLI} ${OBJECT} ${COMPILER} ${LINENOISE})
message("\r\n-----[SCR_SOUCES_PATH]-----")
foreach (SOURCE_PATH ${SCR_SOUCES_LIST})
    message("--> ${SOURCE_PATH}")
endforeach ()
message("-----[END_MESSAGES]-----\r\n")

# 定义头文件 *.h
file(GLOB_RECURSE SCR_HEADER_PATH "*.hpp" "*.h")
message("-----[SCR_HEADER_PATH]-----")
foreach (HEADER_PATH ${SCR_HEADER_PATH})
    message("--> ${HEADER_PATH}")
endforeach ()
message("-----[END_MESSAGES]-----\r\n")

LINK_LIBRARIES(m)

include_directories(include VM Parser Cli Object Compiler Linenoise)

add_executable(Tiderip ${SCR_SOUCES_LIST} ${SCR_HEADER_PATH})

# 加载自动补全信息
add_custom_target(
        autoCompleteMode
        COMMAND python3 ../Util/Script/cliScript.py
)
add_dependencies(Tiderip autoCompleteMode)

# 运行插件系统
#add_custom_target(
#        PluginSystem
#        COMMAND python3 ../Util/Script/coreScript.py
#)
#add_dependencies(Tiderip PluginSystem)