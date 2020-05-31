######### Makefile COMMON ########
#c 还是 c++
EXTENSION = c
#gcc 还是 g++
#CC = gcc
CC = arm-none-linux-gnueabi-gcc
#CC = arm-himix200-linux-gcc
#生成的可执行文件名
EXE = sqlite_blob
#源文件路径 空格 可以继续添如 src src1 src2
SUBDIR += .
INC += -I.
INC += -I./itop4412_install/include
#SUBDIR += ./dbg
#INC += -I./dbg
#.o文件存放路径
OBJPATH = obj
DEP_LIB += ./itop4412_install/lib/libsqlite3.a


#后面的内容都不需要修改
CXX_SOURCES = $(foreach dir,$(SUBDIR), $(wildcard $(dir)/*.$(EXTENSION)))
CXX_OBJECTS = $(patsubst  %.$(EXTENSION), $(OBJPATH)/%.o, $(notdir $(CXX_SOURCES)))
OBJ = $(patsubst %.c,$(OBJPATH)/%.o,$(CXX_SOURCES))
#CXX_OBJECTS like obj/ucam_dbgout.o
#OBJ like obj/./dbg/ucam_dbgout.o
# Target
.PHONY: all clean
all :$(OBJ) $(EXE)
$(OBJPATH)/%.o:%.c
	@[ -e $(OBJPATH) ] || mkdir -p $(OBJPATH)
	$(CC)  $(CFLAGS) $(DFLAGS) -g -c $^  -o $(OBJPATH)/$(notdir $@)  $(INC) 
    
$(EXE): $(CXX_OBJECTS)
	$(CC)  $^ -g -o $@ -lpthread -lm -ldl $(DEP_LIB)
	cp $@ ~/nfs
clean:
	rm $(EXE) $(OBJPATH) -rf
