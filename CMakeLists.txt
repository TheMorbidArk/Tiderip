cmake_minimum_required(VERSION 3.13)
project(Tiderip C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
add_definitions(-fgnu89-inline)
add_definitions(-std=c11)

# 定义源文件 *.c
aux_source_directory(include INCLUDE)
aux_source_directory(VM VM)
aux_source_directory(Core CORE)
aux_source_directory(Parser PARSER)
aux_source_directory(Cli CLI)
aux_source_directory(Object OBJECT)
aux_source_directory(Compiler COMPILER)
aux_source_directory(Core/core.System coreSystem)
aux_source_directory(Core/core.Range coreRange)
aux_source_directory(Core/core.Map coreMap)
aux_source_directory(Core/core.List coreList)
aux_source_directory(Core/core.String coreString)
aux_source_directory(Core/core.Num coreNum)
aux_source_directory(Core/core.Null coreNull)
aux_source_directory(Core/core.Function coreFunction)
aux_source_directory(Core/core.Thread coreThread)
aux_source_directory(Core/core.Bool coreBool)
aux_source_directory(Extra/Linenoise ExtraLinenoise)
aux_source_directory(Extra/Tuibox ExtraTuibox)

# 显示源文件路径
set(SCR_SOUCES_LIST
        ${ExtraLinenoise} ${ExtraTuibox}    # 第三方库,框架
        ${INCLUDE} ${VM} ${CORE} ${PARSER} ${CLI} ${OBJECT} ${COMPILER}     # 核心代码
        # core 标准库
        ${coreSystem} ${coreFunction} ${coreNum} ${coreNull} ${coreBool}
        ${coreRange} ${coreThread} ${coreMap} ${coreList} ${coreString}
        )
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

include_directories(Extra/Linenoise Extra/Tuibox include VM Core Parser Cli Object Compiler Extension)

# No.subdirectory
add_subdirectory(Extension/Regex)
add_subdirectory(Extension/Test)

add_executable(Tiderip ${SCR_SOUCES_LIST} ${SCR_HEADER_PATH})

# No.libraries
target_link_libraries(Tiderip Regex Test)

# 加载自动补全信息
add_custom_target(
        autoCompleteMode
        COMMAND python3 ../Config/CliConfig/cliScript.py
)
add_dependencies(Tiderip autoCompleteMode)

# 运行插件系统
add_custom_target(
        PluginSystem
        COMMAND python3 ../Config/ExtenConfig/extenScript.py
)
add_dependencies(Tiderip PluginSystem)