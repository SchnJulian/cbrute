cmake_minimum_required(VERSION 3.15)
project(cbrute)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -fprofile-instr-generate -fcoverage-mapping")
add_executable(cbrute main.cpp Generator.cpp Generator.h)