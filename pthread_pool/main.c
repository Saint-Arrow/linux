#include "../header_file.h"
#include "dbgout.h"
#include "pthread_pool.h"

void *test(void *time_p)
{
	int time=*(int *)time_p;
	sleep(time);
	*(int *)time_p=0;
}
int a=3,b=4,c=5,d=5;
int main(int argc , char *argv[])
{
	threadpool_t * threadpool=threadpool_create(3,2);
	threadpool_add(threadpool,test,(void *)&a);
	threadpool_add(threadpool,test,(void *)&b);
	threadpool_add(threadpool,test,(void *)&c);
	threadpool_add(threadpool,test,(void *)&d);
	
	//阻塞直到任务完成
	while(a||b||c||d){};
	
	trace("all task finish");
	threadpool_destroy(threadpool);
	sync();
}