#include "../inc/main.h"
#include "../inc/config.h"
#include "../inc/directoryOperation.h"
#include "../inc/tcpServer.h"
#include "../inc/threadPool.h"
#include "../inc/transmitFile.h"
#include "../inc/workQue.h"
#include "../inc/passwordConfirm.h"




int main(int argc, char *argv[]){
	// ./main pConfigNameFile
	ARGS_CHECK(argc,2);
	int ret = 0;

	// 1.获得配置文件的内容
	config_t config;
	memset(&config,0,sizeof(config));
	ret = getConfig(argv[1], &config);

	// 2.初始化线程池
	int threadNum = atoi(config.threadNum);
	ThreadPool_t pool;
	memset(&pool, 0, sizeof(pool));
	threadPoolInit(threadNum, &pool);

	// 3.启动线程池
	threadPoolStart(&pool);

	// 4.tcpserver服务

	// 创建socket套接字
	int sfd = 0;
	tcpInit(config.ip, config.port, &sfd);

	// 设置epfd
	int epfd = epoll_create(1);
	ERROR_CHECK(epfd, -1, "epoll_create");

	// 启动tcpserver服务
	tcpServer(sfd, epfd, &pool);




	
	
	return 0;
}