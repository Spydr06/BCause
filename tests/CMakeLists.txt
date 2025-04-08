cmake_minimum_required(VERSION 3.12)

project(btests VERSION 0.1 DESCRIPTION "Tests for B compiler")
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Werror -Wshadow)

#
# Download GoogleTest
#
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)
enable_testing()

#
# Common definitions and libraries for all tests.
#
add_definitions(
    -DBUILD_DIR="${CMAKE_BINARY_DIR}"
    -DTEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
link_libraries(gtest_main)

#
# Unit tests.
#
add_executable(btest EXCLUDE_FROM_ALL
    fixture.cpp
    libb_test.cpp
    globals_test.cpp
    func_test.cpp
    expr_test.cpp
    string_test.cpp
    hello_test.cpp
    e2_test.cpp
    fibonacci_test.cpp
    fizzbuzz_test.cpp
    precedence_test.cpp
    assignment_test.cpp
)
gtest_discover_tests(btest EXTRA_ARGS --gtest_repeat=1 PROPERTIES TIMEOUT 120)
