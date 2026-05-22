cmake_minimum_required(VERSION 3.20)
project(hw07 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hw07 src/main.cpp)
target_include_directories(hw07 PUBLIC include)
target_compile_options(hw07 PRIVATE -Wall -Wextra)
