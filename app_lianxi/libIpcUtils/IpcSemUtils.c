#include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/types.h>
 #include <unistd.h>
 #include <sys/sem.h>
 #include <sys/ipc.h>


//#define __DEBUG

#ifdef __DEBUG
#define __D(fmt, args...) fprintf(stderr,  fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#ifdef __DEBUG
#define __E(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#else
#define __E(fmt, args...)
#endif

union semun {
      int val;
      struct semid_ds *buf;
      unsigned short *array;
 };


int IpcSemaphore_Init(key_t sem_key)
{
	int qid;

	qid = semget(sem_key, 1, 0666|IPC_CREAT);
	if(qid < 0)
	{
		return -1;
	}

	__D("Semaphore id:%d\n",qid);

	return qid;
}

int IpcSemaphore_SetValue(int qid, int value)
{
	union semun sem_union;
	
	sem_union.val = value;
	
	if (semctl(qid, 0, SETVAL, sem_union) == -1) 
	{
		perror("Sem init");
		return -1;
	}

	__D("Semaphore Set Value:%d\n", value);
	
	return 0;
}

// P操作
int IpcSemaPhore_Wait(int qid)
{
	struct sembuf sem_buf;
	
	sem_buf.sem_num = 0;
	sem_buf.sem_op = -1;	
	sem_buf.sem_flg = SEM_UNDO;	

	if (semop(qid, &sem_buf, 1) == -1)
	{
		perror("Sem Wait operation");
		return -1;
	}

	__D("Semaphore Wait\n");
	
	return 0;
}

// V操作
int IpcSemaPhore_Post(int qid)
{
	struct sembuf sem_buf;
	
	sem_buf.sem_num = 0;
	sem_buf.sem_op = 1;	
	sem_buf.sem_flg = SEM_UNDO;
	
	if (semop(qid, &sem_buf, 1) == -1) {
		perror("Sem Post operation");
		return -1;
	}

	__D("Semaphore Post\n");
	
	return 0;
}

// 删除信号量
int IpcSemaphore_kill(int qid)
{
	union semun sem_union;
	
	if (semctl(qid, 0, IPC_RMID, sem_union) == -1) 
	{
		perror("Sem kill");
		return -1;
	}
	
	__D("Kill Semaphore id:%d\n",qid);
	
	return 0;		
}

