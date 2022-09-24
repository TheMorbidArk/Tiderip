cmake_minimum_required(VERSION 3.13)
project(Sparrow C)

set(CMAKE_C_STANDARD 11)

add_executable(Sparrow include/common.h include/utils.h include/utils.c include/unicodeUtf8.c include/unicodeUtf8.h VM/vm.c VM/vm.h VM/core.c VM/core.h Parser/parser.c Parser/parser.h Cli/cli.c Cli/cli.h)

include_directories(include VM Parser Cli)