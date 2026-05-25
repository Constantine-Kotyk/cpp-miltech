cmake_minimum_required(VERSION 3.20)
project(hw08 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hw08 src/main.cpp src/ConfigLoaderJSON.cpp src/AnalyticalSolver.cpp src/MissionProcessor.cpp)
target_include_directories(hw08 PUBLIC include)
target_compile_options(hw08 PRIVATE -Wall -Wextra)
