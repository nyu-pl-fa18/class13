cmake_minimum_required(VERSION 3.0)
project(pl-class13)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

set(SOURCE_FILES1 src/ptr_test.cpp)
add_executable(ptr_test ${SOURCE_FILES1})
