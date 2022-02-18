
CC := gcc


#INSTALLDIR = $(APP_LIB_DIR)

IPC_LIB = libIpcUtils.a


#CFLAGS += -I$(APP_HEADER_DIR)

CFLAGS += -fPIC

IPC_OBJ = IpcMsgUtils.o IpcSemUtils.o IpcShareMemUtils.o

%.o: %.c
	$(CC) $(CFLAGS) -c  $< -o $@

#all:  $(IPC_LIB) 
all:server client

$(IPC_LIB): $(IPC_OBJ)
	$(AR) $(AR_FLAGS) $(IPC_LIB) $(IPC_OBJ) 

server:server.o ${IPC_OBJ}
	$(CC) -o $@ $^ -lm	
	
client:client.o ${IPC_OBJ}
	$(CC) -o $@ $^ -lm	
	
clean:
	rm -rf *.a *.o $(IPC_OBJ) $(IPC_LIB) server client
	
