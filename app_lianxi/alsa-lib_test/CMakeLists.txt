cmake_minimum_required(VERSION 3.8)

# 设置交叉编译链
set(CMAKE_SYSTEM_NAME Linux) # 指定目标操作系统为 Linux
set(CMAKE_SYSTEM_PROCESSOR aarch64) # 指定目标处理器架构为 ARM
# 指定交叉编译器
set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc") # 指定 C 编译器
set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++") # 指定 C++ 编译器


set(CMAKE_CXX_STANDARD 11)
PROJECT(alsa_test)

SET(SRC_LIST test.cpp)
ADD_EXECUTABLE(alsa_test ${SRC_LIST})

set(SYSROOT_DIR /data/cwj/data/vhd_gitlab/cv5_trunk/cooper_linux_sdk_1.5_cv5x/ambarella/out/amba_out/cv5_timn/sysroot)
set(CMAKE_EXE_LINKER_FLAGS "-L${SYSROOT_DIR}/usr/lib/")
include_directories(${SYSROOT_DIR}/usr/include)
target_link_libraries(alsa_test libasound.so)

set_target_properties(alsa_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)