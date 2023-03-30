#include "../inc/threadPool.h"
#include "../inc/transmitFile.h"
#include "../inc/workQue.h"
#include "../inc/directoryOperation.h"
#include "../inc/main.h"
#include "../inc/passwordConfirm.h"


//线程处理函数
void *threadFunc(void *p);

//判断recv函数的返回值
//-2代表客户端断开连接，-1代表读取异常，0代表正常读取
static inline int recvJudge(int recvResult){
	if(0 == recvResult){
		printf("客户端已关闭连接!\n");
		return -2;
	}
	if(-1 == recvResult){
		return -1;
	}
	else{
		return 0;
	}
}

//定义protocol结构体为传入传出参数
//接收文件的状态和文件的总大小传入protocol结构体
//客户端的getOneFrame与服务端的getOneFrame成对照关系，服务端通过getOneFrame从客户端获得文件大小的信息，
//客户端通过getFrame接收服务端对文件的判断信息,比如是否存在同名文件
int getOneFrame(int clientFd, Protocol_t *pProtocol){
	//定义接收信息的数据类型
	int type = 0;
	int dataLen = 0;

	//定义 接收 函数返回值的数据类型
	int ret = 0;

	//获取数据状态
	ret = recv(clientFd, &type, sizeof(type), MSG_WAITALL);

	//如果读取异常,返回定义的异常值
	if(0 != (ret = recvJudge(ret))){
		return ret;
	}

	//获取数据大小
	ret = recv(clientFd, &dataLen, sizeof(dataLen), MSG_WAITALL);

	//如果读取异常，返回定义的异常值
	if(0 != (ret = recvJudge(ret))){
		return ret;
	}

	pProtocol->type = type;
	pProtocol->dataLen = dataLen;

	if(0 == dataLen){
		return 0;
	}

	//获取数据
	ret = recv(clientFd, pProtocol->data, pProtocol->dataLen, MSG_WAITALL);

	//如果读取异常，返回定义的异常值
	if(0 != (ret = recvJudge(ret))){
		return ret;
	}

	return 0;


}

//给客户端发送信息
//牢记服务端和客户端的传输都是用私有协议结构体
int sendMessage(int clientFd, const char *message){
	int messageLen = strlen(message);
	//用堆空间创建私有协议结构体
	Protocol_t *pProtocol = (Protocol_t *)malloc(sizeof(*pProtocol) + MAXSIZE + 1);
	pProtocol->type = MESSAGE;
	pProtocol->dataLen = messageLen;
	memcpy(pProtocol->data, message, messageLen);
	
	send(clientFd, pProtocol, getProtocolSize(pProtocol), 0);

	free(pProtocol);
	return 0;


}

//接收客户端的登录信息
int waitClientSignIn(int clientFd){
	Protocol_t *pProtocol = (Protocol_t *)calloc(1, sizeof(*pProtocol) + MAXSIZE +1);
	int ret = 0;

	while(1){
		ret = getOneFrame(clientFd, pProtocol);
		if(0 != ret){
			break;
		}
		//用户名和密码隔开，找到第一个空格后面就是面膜
		char *userName = pProtocol->data;
		char *password = pProtocol->data;
		while(*password != ' '){
			password ++;
		}
		//空格处置为0
		*password = 0;
		password ++;
		pProtocol->data[pProtocol->dataLen] = 0;

		//验证密码的正确性
		if(0 == passwordConfirm(userName, password)){
			pProtocol->type = PASSWD_RIGHT;
			pProtocol->dataLen = 0;
			send(clientFd, pProtocol, getProtocolSize(pProtocol), 0);

			//密码正确可以跳出循环
			break;
		}
		else{
			pProtocol->type = PASSWD_ERROR;
			pProtocol->dataLen = 0;
			send(clientFd, pProtocol, getProtocolSize(pProtocol), 0);
		}	

	}
	free(pProtocol);
	return ret;
	

}


//客户端交互函数
int interActiveClient(int clientFd){
	//用堆空间创建私有协议结构体
	Protocol_t *pProtocol = (Protocol_t *)malloc(sizeof(*pProtocol) + MAXSIZE + 1);

	int ret = 0;
	char *message = (char *)malloc(MAXSIZE);

	//验证用户登录
	ret = waitClientSignIn(clientFd);
	if(0 != ret){
		return -1;
	}
	while(1){
		//通过getOneFrame获得客户端传来的信息
		ret = getOneFrame(clientFd, pProtocol);
		if(ret != 0){
			break;
		}
		//打个标记，用来判断客户端不同的输入,给客户端展示ls，pwd后的信息
		message[0] = 0;
		
		//根据客户端输入的信息进行判断
		switch(pProtocol->type){
			case CD:
			changeDirectory(pProtocol->data);
			break;
			case LS:
			memset(message, 0, MAXSIZE);
			listFile(".", message, MAXSIZE-1);
			break;
			case PWD:
			memset(message, 0, MAXSIZE);
			printWorkDirectory(message, MAXSIZE-1);
			break;
			case RM:
			removeFile(pProtocol->data);
			break;
			case MKDIR:
			makeDirectory(pProtocol->data);
			break;
			case PUTS:
			getFile(clientFd, pProtocol->data);
			break;
			case GETS:
			putFile(clientFd, pProtocol->data);
			break;
			default:
			break;
		}
		if(message[0] != 0){
			sendMessage(clientFd, message);

		}
		

	}
	free(message);
	free(pProtocol);
	return 0;
}

//线程处理函数
void *threadFunc(void *p){
	pQue_t pQue = (pQue_t)p;
	//用来获取队列节点
	pNode_t pCur = NULL;
	int getSuccess = 1;
	while(1){

		//访问队列
		pthread_mutex_lock(&pQue->mutex);
		if(pQue->size == 0){
			pthread_cond_wait(&pQue->cond, &pQue->mutex);
		}
		getSuccess = queGet(pQue, &pCur);
		pthread_mutex_unlock(&pQue->mutex);

		//如果访问成功,得到了clientFd
		if(0 == getSuccess){
			interActiveClient(pCur->clientFd);
			free(pCur);
			pCur = NULL;


		}

	}
	pthread_exit(NULL);
}



//线程池的创建
int threadPoolInit(int threadNum, pThreadPool_t pPool){
	pPool->threadNum = threadNum;
	pPool->pThid = (pthread_t *)calloc(threadNum, sizeof(ThreadPool_t));
	queInit(&pPool->que);
	return 0;

}


//线程池的启动
int threadPoolStart(pThreadPool_t pPool){
	for(int i = 0; i < pPool->threadNum; ++i){
		pthread_create(pPool->pThid+i, NULL, threadFunc, &pPool->que);
	}

	printf("线程池成功启动！\n");
	return 0;


}
