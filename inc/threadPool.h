#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "../inc/main.h"
#include "../inc/workQue.h"
typedef struct{
	int threadNum;
	pthread_t *pThid;
	Que_t que;	
}ThreadPool_t, *pThreadPool_t;

//线程池的创建
int threadPoolInit(int threadNum, pThreadPool_t);
//线程池的启动
int threadPoolStart(pThreadPool_t pPool);

//获得protocol结构体的信息
int getOneFrame(int clientFd, Protocol_t *pProtocol);

//客户端交互函数
int interActiveClient(int clientFd);

//给客户端发送信息的函数
int sendMessage(int clientFd, const char *message);



#endif