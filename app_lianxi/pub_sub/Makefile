CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -I.
LDFLAGS = -lpthread

# 源文件
PUBSUB_SRC = pubsub.c
LWRB_SRC = lwrb/lwrb.c
TEST_SRC = test_pubsub.c

# 目标文件
PUBSUB_OBJ = $(PUBSUB_SRC:.c=.o)
LWRB_OBJ = $(LWRB_SRC:.c=.o)
TEST_OBJ = $(TEST_SRC:.c=.o)

# 最终可执行文件
TEST_TARGET = test_pubsub

.PHONY: all clean test

all: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJ) $(PUBSUB_OBJ) $(LWRB_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(PUBSUB_OBJ) $(LWRB_OBJ) $(TEST_OBJ) $(TEST_TARGET)
	echo "Cleaned"
