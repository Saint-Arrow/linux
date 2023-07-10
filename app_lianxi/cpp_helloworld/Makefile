######### Makefile COMMON ########
#gcc 还是 g++
CC = gcc
LD = g++
#源文件路径 空格 可以继续添如 src src1 src2
SRCS += ./
INC += -I.
#SUBDIR += ./dbg
#INC += -I./dbg
#.o文件存放路径
OBJPATH = obj



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
all :hello_world
    
hello_world:hello_world.o 
	$(LD)  $^ -g -o $@ -lpthread -lm 

clean:
	rm hello_world $(CXX_OBJECTS) $(C_OBJECTS) -rf
