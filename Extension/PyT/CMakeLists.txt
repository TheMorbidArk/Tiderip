aux_source_directory(. DIR_HELLO_SRCS)

# 如果使用的是非系统目录下的 Python 可以通过指定 Python3_ROOT_DIR 改变查找路径
# set(Python3_ROOT_DIR "${CMAKE_SOURCE_DIR}/venv")
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_library(PyT ${DIR_HELLO_SRCS})

target_include_directories(PyT PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(PyT ${Python3_LIBRARIES})

message("-- Link PyT Libraries...")
