######### Makefile COMMON ########
#c 还是 c++
EXTENSION = c
#gcc 还是 g++
CXX = aarch64-linux-gnu-c++
LD = aarch64-linux-gnu-c++
#生成的可执行文件名
EXE = dbus_test
#源文件路径 空格 可以继续添如 src src1 src2
SRCS += ./
INC += -I.
#SUBDIR += ./dbg
#INC += -I./dbg
#.o文件存放路径
OBJPATH = obj
sysroot=/data/cwj/data/vhd_gitlab/cv5_project/sdk/cooper_linux_sdk_1.5_cv5x/ambarella/out/amba_out/cv5_timn/sysroot
CFLAGS += -I$(sysroot)/usr/include/dbus-1.0 
LINK_LIBS += -ldbus-1 -lsystemd -llzma -lz -L$(sysroot)/usr/lib/
#后面的内容都不需要修改
CFLAGS += -g
CXX_SOURCES = $(foreach dir,$(SRCS), $(wildcard $(dir)/*.cpp))
CXX_OBJECTS = $(patsubst  %.cpp, %.o, $(CXX_SOURCES))
C_SOURCES = $(foreach dir,$(SRCS), $(wildcard $(dir)/*.c))
C_OBJECTS = $(patsubst  %.c, %.o, $(C_SOURCES))
CXXFLAGS += $(CFLAGS)
CXXFLAGS += -std=c++11
#CXX_OBJECTS like obj/ucam_dbgout.o
#OBJ like obj/./dbg/ucam_dbgout.o
# Target
.PHONY: all clean
all :$(CXX_OBJECTS) $(C_OBJECTS) $(EXE)
    
$(EXE): $(CXX_OBJECTS) $(C_OBJECTS)
	$(LD)  $^ -g -o $@ $(LINK_LIBS) -lpthread -lm 
	cp $@ ~/nfs
clean:
	rm $(EXE) $(CXX_OBJECTS) $(C_OBJECTS) -rf
