

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
PROJECT(STL_demo)

SET(SRC_LIST main.cpp)
ADD_EXECUTABLE(STL_demo ${SRC_LIST})

set_target_properties(STL_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)