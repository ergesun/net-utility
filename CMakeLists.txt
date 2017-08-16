cmake_minimum_required(VERSION 3.5)
project(net_utility)

set(CMAKE_CXX_FLAGS "-g -O0 -Wno-attributes")
#set(CMAKE_CXX_FLAGS "-O3 -Wall")
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(.. SOURCE_FILES)
message(STATUS "PROJECT_SOURCE_DIR = " .)

set(LINK_DIRS
        ${PROJECT_BINARY_DIR}/common
        ${PROJECT_BINARY_DIR}/net
        )
link_directories(${LINK_DIRS})

add_subdirectory(common)
add_subdirectory(net)
add_subdirectory(test)
