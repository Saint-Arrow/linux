#ifndef _Pthread_Pool_H
#define _Pthread_Pool_H


#include <pthread.h>

typedef struct{
	void * (*function)(void *argv);
	void *argv;
}threadpool_task_t;

typedef struct{
	pthread_mutex_t task_lock;// mutex for the taskpool
	threadpool_task_t *task_queue;//任务队列
	pthread_cond_t queue_not_full;
	pthread_cond_t queue_not_empty;//任务队列非空的信号
	int queue_size;
	int r_index,w_index;
	int queue_max_size;
	
	pthread_mutex_t thread_counter;//mutex for count the busy thread
	pthread_t *threads;//执行任务的线程
	pthread_t adjust_tid;//负责管理线程数目的线程
	
	int max_thread_num;
	int quit;
}threadpool_t;


extern threadpool_t *threadpool_create(int max_thread_num, int max_task_queue_size);
extern int threadpool_destroy(threadpool_t *threadpool);

extern int threadpool_add(threadpool_t *threadpool, void*(*function)(void *arg), void *arg);
#endif