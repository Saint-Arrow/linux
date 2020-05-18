######### Makefile COMMON ########
#c 还是 c++
EXTENSION = c
#gcc 还是 g++
CC = gcc
#生成的可执行文件名
EXE = uvc_test
#源文件路径 空格 可以继续添如 src src1 src2
SUBDIR += .
INC += -I.
#SUBDIR += ./dbg
#INC += -I./dbg
#.o文件存放路径
OBJPATH = obj

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
	$(CC)  $^ -o $@ -lpthread -lm -g
    
clean:
	rm $(EXE) $(OBJPATH) -rf
