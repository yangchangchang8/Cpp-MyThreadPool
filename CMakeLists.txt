cmake_minimum_required(VERSION 3.10)

# 项目名与语言
project(MyThreadPoolProject LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找源文件
file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 添加可执行文件
add_executable(my_thread_pool ${SOURCES})
