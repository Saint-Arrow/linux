# ============================================================
# Mongoose CGI Server - Makefile
#
# 用法:
#   make              # 编译 (Linux / MinGW)
#   make clean        # 清理
#   make run          # 编译并运行
#
# Windows MSVC (在 Developer Command Prompt 中):
#   nmake /f Makefile.msvc
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c99 -I. -D_GNU_SOURCE
LDFLAGS = -lpthread   # Linux 下使用 pthread, Windows 需要 -lws2_32

# 源文件
SRCS = main.c \
       mongoose.c \
       router.c \
       modules/mod_user.c \
       modules/mod_device.c \
       modules/mod_system.c

OBJS = $(SRCS:.c=.o)
TARGET = cgi_server.exe

# ---- 规则 ----
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build OK -> $(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Clean done."

.PHONY: all run clean
