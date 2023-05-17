
/* ===========================================================================
* @file 		IpcMsgUtils.c
*
* @version 	V1.0
*
* @author	Liyuan
*
* @create		2014/10/17
* =========================================================================== */

#include <stdio.h>
#include <linux/errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "libIpcUtils.h"


/*
struct my_message {
    long int message_type;
    //The data you wish to transfer 
};
*/

/**
* @brief Initialize message queue.

* Initialize message queue.
* @note This API must be used before use any other message driver API.
* @param msgKey [I ] Key number for message queue and share memory.
* @return message ID.
*/
int IpcMsgInit(int msgKey)
{
	int qid;
	key_t key = msgKey;

	qid = msgget(key,0);
	if(  qid < 0 )
	{
		qid = msgget(key,IPC_CREAT|0666);
		__D("Creat queue id:%d\n",qid);
	}

	__D("queue id:%d\n",qid);

	return qid;
}

/**
* @brief Send message .

* Excute send message command.
* @param qid [I ] Message ID.
* @param *pdata [I ] pointer to the data.
* @param size [I ] Data size.
* @return send data to message queue.
*/
int IpcMsgSend(int qid, void *pdata , int size)
{

	return msgsnd(qid, pdata, size-sizeof(long), 0 );///< send msg1

}

/**
* @brief Receive message .

* Excute receive message command.
* @param qid [I ] Message ID.
* @param msg_type [I ] Message type.
* @param *pdata [I ] pointer to the data.
* @param size [I ] Data size.
* @return receive data from message queue.
*/
int IpcMsgRecv(int qid, int msg_type, void *pdata , int size)
{
	return msgrcv(qid, pdata, size-sizeof(long), msg_type, 0);
}

/**
* @brief Send and receive message .

* Excute send and receive message command.
* @param qid [I ] Message ID.
* @param msg_type [I ] Message type.
* @param *pdata [I ] pointer to the data.
* @param size [I ] Data size.
* @return receive data from message queue if send success.
*/
int IpcMsgSendRecv(int qid, int msg_type, void *pdata , int size )
{
	int ret = 0;
	ret = msgsnd(qid, pdata, size-sizeof(long), 0 );/* send msg1 */
	if( ret < 0 )
	{
		return ret;
	}
	return msgrcv(qid, pdata, size-sizeof(long), msg_type, 0);

}

/**
* @brief Kill message queue.

* Kill message queue.
* @param qid [I ] Message ID.
* @retval 0 Queue killed.
*/
int IpcMsgKill(int qid )
{
	msgctl(qid, IPC_RMID, NULL);

	__D("Kill queue id:%d\n",qid);

	return 0;
}

