#include "../header_file.h"

//ipcs 查看信号量
//ipcrm -s qid 删除对应的信号量

int qid=-1;

int main()
{
	int ret=0;

	qid=IpcSemaphore_Init(ftok(".",1001));
	if(qid<0)
	{
		printf("IpcSemaphore_Init err");
		return -1;
	}
	printf("IpcSemaphore_Init %d\n",qid);
	IpcSemaphore_SetValue(qid,1);
	while(1)
	{
		ret=IpcSemaPhore_Wait(qid);
		if(ret<0)
		{
			IpcSemaphore_kill(qid);
			qid=-1;
			break;
		}
		printf("server get\n");
		sleep(1);
		IpcSemaPhore_Post(qid);
		
	}
	sync();
	return 0;

}
