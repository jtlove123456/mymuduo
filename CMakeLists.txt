cmake_minimum_required(VERSION 2.5)

project(mymuduo)

# 设置编译选项
set(CMAKE_CXX_COMPILE_FEATURES "usr/local/gcc/bin/g++")
# -fPIC 编译动态库需要的
add_compile_options(-Wall -std=c++11 -g -fPIC)

# 添加头文件搜索路径 ,之前写错了mymudo,导致找不到头文件
include_directories(${PROJECT_SOURCE_DIR}/mymuduo/base)
include_directories(${PROJECT_SOURCE_DIR}/mymuduo/net)

# 设置编译生成库的目录,根目录的lib下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# # 设置编译生成可执行文件路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 去找子目录的cmakelists.txt
add_subdirectory(mymuduo)

