

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
PROJECT(cpp_lianxi)

SET(SRC_LIST hello_world/hello_world.cpp)
ADD_EXECUTABLE(hello ${SRC_LIST})