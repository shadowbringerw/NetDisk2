#include "../inc/tcpServer.h"


//获得服务端的ip地址，端口号, 生成一个套接口描述符
int tcpInit(char *ip, char *port, int *sockFd){
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	ERROR_CHECK(sfd, -1, "socket");
	int ret = 0;

	//保存本机的ip地址和端口号
	struct sockaddr_in serAddr;
	memset(&serAddr, 0, sizeof(serAddr));
	serAddr.sin_family = AF_INET;
	serAddr.sin_addr.s_addr = inet_addr(ip);
	serAddr.sin_port = htons(atoi(port));

	//将本机的ip和端口号绑定到sfd上
	ret = bind(sfd, (struct sockaddr *)&serAddr, sizeof(serAddr));
	ERROR_CHECK(ret, -1, "bind");

	//设置最大监听数
	ret = listen(sfd, 10);
	ERROR_CHECK(ret, -1, "listen");

	//传输文件描述符
	*sockFd = sfd;
	return 0;
}

//使用epoll监听文件描述符，对客户端的请求做出处理
int tcpServer(int sockFd, int epfd, pThreadPool_t pool){
	int ret = 0;

	struct epoll_event events;
	memset(&events, 0, sizeof(events));

	//将服务端的sockFd加入epoll监听集合中
	events.data.fd = sockFd;
	events.events = EPOLLIN;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sockFd, &events);

	struct epoll_event evs[10];
	memset(evs, 0, sizeof(evs));

	int readyNum = 0;
	int clientFd = 0;

	while(1){
		readyNum = epoll_wait(epfd, evs, 10, -1);
		for(int i = 0; i < readyNum; ++i){
			if(sockFd == evs[i].data.fd){
				clientFd = accept(sockFd, NULL, NULL);
				pNode_t pNew = (pNode_t)calloc(1, sizeof(Node_t));
				pNew->clientFd = clientFd;

				pthread_mutex_lock(&pool->que.mutex);
				queInsert(&pool->que, pNew);
				pthread_cond_broadcast(&pool->que.cond);
				pthread_mutex_unlock(&pool->que.mutex);

			}
		}
	}


}