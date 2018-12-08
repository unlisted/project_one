cmake_minimum_required (VERSION 2.6)
project (node)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
add_executable(node ${SOURCES})

