cmake_minimum_required(VERSION 3.10)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

project(fscpp
    VERSION 0.1
    DESCRIPTION "my simple ls command"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(fscpp)

target_sources(fscpp
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/res/resource.rc
)

set_target_properties(fscpp PROPERTIES OUTPUT_NAME "fs")