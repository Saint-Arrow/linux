

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
PROJECT(hello_world)

SET(SRC_LIST hello_world.cpp)
ADD_EXECUTABLE(hello_world ${SRC_LIST})

set_target_properties(hello_world PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)