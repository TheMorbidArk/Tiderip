cmake_minimum_required(VERSION 3.13)
project(VanTideL C)

set(CMAKE_C_STANDARD 11)

# 定义源文件 *.c
aux_source_directory(include INCLUDE)
aux_source_directory(VM VM)
aux_source_directory(Parser PARSER)
aux_source_directory(Cli CLI)
aux_source_directory(Object OBJECT)
aux_source_directory(Compiler COMPILER)

set(SCR_SOUCES_LIST ${INCLUDE} ${VM} ${PARSER} ${CLI} ${OBJECT} ${COMPILER} )
message("\r\n-----[SCR_SOUCES_LIST]-----")
foreach(SOURCE_PATH ${SCR_SOUCES_LIST})
    message("--> ${SOURCE_PATH}")
endforeach()
message("-----[END_MESSAGES]-----\r\n")

# 定义头文件 *.h
file(GLOB_RECURSE SCR_HEADER_PATH "*.hpp" "*.h")
message("-----[SCR_HEADER_PATH]-----")
foreach(HEADER_PATH ${SCR_HEADER_PATH})
    message("--> ${HEADER_PATH}")
endforeach()
message("-----[END_MESSAGES]-----\r\n")
include_directories(include VM Parser Cli Object)

add_executable(VanTideL ${SCR_SOUCES_LIST} ${SCR_HEADER_PATH})