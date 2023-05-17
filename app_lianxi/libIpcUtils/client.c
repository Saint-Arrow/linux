#include "../header_file.h"


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
	while(1)
	{
		ret=IpcSemaPhore_Wait(qid);
		if(ret<0)
		{
			IpcSemaphore_kill(qid);
			qid=-1;
			break;
		}
		printf("client get\n");
		sleep(1);
		IpcSemaPhore_Post(qid);
		
	}
	sync();
	return 0;

}


