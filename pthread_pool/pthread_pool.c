#include "../header_file.h"
#include "dbgout.h"

#include "pthread_pool.h"

#define DEFAULT_TIME 100 // 领导定时检查队列、线程状态的时间间隔
void *pthreadpool_adjust(void *argv);
void *pthreadpool_work(void *argv);

threadpool_t *threadpool_create(int max_thread_num, int max_task_queue_size)
{
	int  ret=0,i=0;
	threadpool_t * threadpool=malloc(sizeof(threadpool_t));
	if(threadpool==NULL)
	{
		return NULL;
	}
	threadpool->threads=malloc(sizeof(pthread_t)*max_thread_num);
	if(threadpool->threads==NULL)
	{
		goto error0;
	}
	threadpool->task_queue=malloc(sizeof(threadpool_task_t)*max_task_queue_size);
	if(threadpool->task_queue==NULL)
	{
		goto error1;
	}
	
	ret|=pthread_create(&threadpool->adjust_tid,NULL,pthreadpool_adjust,threadpool);
	for(i=0;i<max_thread_num;i++)
	{
		ret|=pthread_create(&threadpool->threads[i],NULL,pthreadpool_work,threadpool);
	}
	pthread_mutex_init(&threadpool->task_lock,NULL);
	pthread_mutex_init(&threadpool->thread_counter,NULL);
	pthread_cond_init(&threadpool->queue_not_full,NULL);
	pthread_cond_init(&threadpool->queue_not_empty,NULL);
	threadpool->quit=0;
	threadpool->r_index=0;
	threadpool->w_index=0;
	threadpool->queue_size=0;
	threadpool->queue_max_size=max_task_queue_size;
	return threadpool;
	

error2:
	free(threadpool->task_queue);
error1:
	free(threadpool->threads);	
error0:
	free(threadpool);
	return NULL;
}	
int threadpool_destroy(threadpool_t *threadpool)
{
	int i =0;
	threadpool->quit=1;
	pthread_join(threadpool->adjust_tid,NULL);
	
	pthread_cond_broadcast(&(threadpool->queue_not_empty));
	for(i=0;i<threadpool->max_thread_num;i++)
	{
		pthread_join(threadpool->threads[i],NULL);
	}
	
	pthread_mutex_destroy(&threadpool->task_lock);
	pthread_mutex_destroy(&threadpool->thread_counter);
	pthread_cond_destroy(&threadpool->queue_not_full);
	pthread_cond_destroy(&threadpool->queue_not_empty);
	free(threadpool->task_queue);
	free(threadpool->threads);	
	free(threadpool);
}
void *pthreadpool_adjust(void *argv)
{
	threadpool_t *threadpool=(threadpool_t *)argv;
	while(!threadpool->quit)
	{
		usleep(DEFAULT_TIME*1000);
	}

}
void *pthreadpool_work(void *argv)
{
	threadpool_t *threadpool=(threadpool_t *)argv;
	threadpool_task_t task;
	while(1)
	{
		pthread_mutex_lock(&threadpool->task_lock);
		while(threadpool->queue_size == 0 && !threadpool->quit)
		{
			pthread_cond_wait(&threadpool->queue_not_empty,&threadpool->task_lock);
		}
		if (threadpool->quit)
		{
			pthread_mutex_unlock(&(threadpool->task_lock));
			strace("thread 0x%x is exiting\n", pthread_self());
			pthread_exit(NULL);
		}
		task.function=threadpool->task_queue[threadpool->r_index].function;
		task.argv=threadpool->task_queue[threadpool->r_index].argv;
		threadpool->r_index++;
		if(threadpool->r_index>=threadpool->queue_max_size)
		{
			threadpool->r_index=0;
		}
		threadpool->queue_size--;
		pthread_cond_broadcast(&(threadpool->queue_not_full));
		pthread_mutex_unlock(&(threadpool->task_lock));
		
		trace("thread 0x%x start working\n", pthread_self());
		(*(task.function))(task.argv);
		trace("thread 0x%x end working\n", pthread_self());
	}
}

int threadpool_add(threadpool_t *threadpool, void*(*function)(void *arg), void *arg)
{
	pthread_mutex_lock(&threadpool->task_lock);
	while(threadpool->queue_size == threadpool->queue_max_size)
	{
		pthread_cond_wait(&threadpool->queue_not_full,&threadpool->task_lock);
	}
	threadpool->task_queue[threadpool->w_index].function=function;
	threadpool->task_queue[threadpool->w_index].argv=arg;
	threadpool->w_index++;
	if(threadpool->w_index>=threadpool->queue_max_size)
	{
		threadpool->w_index=0;
	}
	threadpool->queue_size++;
	pthread_cond_signal(&threadpool->queue_not_empty);
	pthread_mutex_unlock(&threadpool->task_lock);
}