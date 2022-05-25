cmake_minimum_required(VERSION 3.1.1)
project(bc)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# emit warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE source src/*.c)
add_executable(bc ${source})

install(PROGRAMS ${PROJECT_SOURCE_DIR}/bin/bc 
        DESTINATION bin
        RENAME "bc"
)