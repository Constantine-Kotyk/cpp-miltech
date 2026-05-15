cmake_minimum_required(VERSION 3.20)
project(ballistics_check CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ballistics src/ballistics.cpp)
target_include_directories(ballistics PUBLIC include)
target_compile_options(ballistics PRIVATE -Wall -Wextra -Werror -Wpedantic)

add_executable(ballistics_check src/main.cpp)
target_link_libraries(ballistics_check PRIVATE ballistics)
target_compile_options(ballistics_check PRIVATE -Wall -Wextra -Werror -Wpedantic)

if(NOT CMAKE_CROSSCOMPILING)
    include(FetchContent)
    include(GoogleTest)
    FetchContent_Declare(
        googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
                       DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_executable(ballistics_tests tests/ballistics_tests.cpp)
    target_link_libraries(ballistics_tests PRIVATE ballistics GTest::gtest_main)
    gtest_discover_tests(ballistics_tests DISCOVERY_MODE PRE_TEST)
    target_compile_definitions(ballistics_tests
                               PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
    add_test(NAME ballistics_tests COMMAND ballistics_tests)
endif()
