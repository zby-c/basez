cmake_minimum_required(VERSION 3.30)

project(basez LANGUAGES CXX)

# 只是想用 std::print
set(CMAKE_CXX_STANDARD 23)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

# MSVC 静态编译, 防止部分人不会装 VCRuntime
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/MT>")

add_executable(basez main.cpp)
