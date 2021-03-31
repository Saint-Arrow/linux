
/* ===========================================================================
* @file 		IpcShareMemUtils.c
*
* @version 	V1.0
*
* @author	Liyuan
*
* @create		2014/10/17
* =========================================================================== */


#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>  

#include "libIpcUtils.h"

/**
* @brief Initialize shared memory driver.

* Initialize shared momory driver.
* @note Once initialed, the memory ID is saved to global variable.
* @return Memory ID
* @retval -1 Fail to initialize shared memory.
*/
int IpcShareMemInit(int key, int size)
{
	int mid = shmget(key, size, IPC_CREAT | 0777);	// ���������ڴ����  ���ر�ʶ��
	if(mid < 0)
		mid = shmget(key, 0, 0);
	__D("shared memory id:%d\n",mid);
	if(mid < 0)
		return -1;
	char *pSrc = shmat(mid,0,0);			// ӳ�乲���ڴ�  �����ڴ��ַ
	__D("pDst + offset: 0x%x######\n", pSrc );
	return mid;
}

int IpcShareMemDestroy(int mid)
{
	shmctl(mid, IPC_RMID, 0);			// ɾ�������ڴ�
	return 0;
}

/**
* @brief Read shared memory driver.

* Read shared momory.
* @param offset [i]  memory offset
* @param *buf [i]  pointer to memory buffer
* @param length [i]  data length to read
*/
void IpcShareMemRead(int mid, int offset,void *buf, int length)
{
	char *pSrc = shmat(mid,0,0);		// ӳ�乲���ڴ�
	__D("%s\n", __func__);
	__D("offset: %d\n", offset);
	__D("buf: %p\n", buf);
	__D("length: %d\n", length);
	
	memcpy(buf, pSrc + offset, length);  // ��ȡ�����ڴ����ݵ�buf
	shmdt(pSrc);						// �Ͽ�ӳ�䣬������ɾ�������ڴ�
}
/**
* @brief Write shared memory driver.

* Write shared momory.
* @param offset [i]  memory offset
* @param *buf [i]  pointer to memory buffer
* @param length [i]  data length to read
*/
void IpcShareMemWrite(int mid, int offset,void *buf, int length)
{	
	char *pDst = shmat(mid,0,0);

	if (pDst == NULL)
	{
		__E("%s-%s-%d shmat return error %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
		return ;
	}

	memcpy(pDst + offset, buf, length);
	shmdt(pDst);
}


