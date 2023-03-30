#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include "../inc/main.h"
#include "../inc/threadPool.h"

//获得服务端的ip地址，端口号, 生成一个套接口描述符
int tcpInit(char *ip, char *port, int *sockFd);

//使用epoll监听文件描述符，对客户端的请求做出处理
int tcpServer(int sockFd, int epfd, pThreadPool_t pool);

#endif