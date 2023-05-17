
#ifndef LIB_IPC_UTILS_H_INCLUDE
#define LIB_IPC_UTILS_H_INCLUDE

#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>

//#define __IPC_UTILS_DEBUG

#ifdef __IPC_UTILS_DEBUG
#define __D(fmt, args...) fprintf(stderr,  fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#ifdef __IPC_UTILS_DEBUG
#define __E(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#else
#define __E(fmt, args...)
#endif

//Message

int IpcMsgInit( int msgKey );
int IpcMsgKill( int qid );
int IpcMsgSend( int qid, void *pdata , int size );
int IpcMsgRecv( int qid, int msg_type, void *pdata , int size );
int IpcMsgSendRecv( int qid, int msg_type, void *pdata , int size );


//Sem
int IpcSemaphore_Init(key_t sem_key);
int IpcSemaphore_SetValue(int qid, int value);
int IpcSemaPhore_Wait(int qid);
int IpcSemaPhore_Post(int qid);
int IpcSemaphore_kill(int qid);

//ShareMemory

int IpcShareMemInit(int key, int size); ///< Initial shared memory.
void IpcShareMemRead(int mid, int offset,void *buf, int length); ///< Read shared memory.
void IpcShareMemWrite(int mid, int offset,void *buf, int length); ///< Write shared memory.



#endif

