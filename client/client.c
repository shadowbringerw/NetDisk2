#include "../inc/main.h"
#include "../inc/md5.h"

//向服务器发送需求的函数
int sendCommand(int sockFd, const char *commandBuffer, int commandType);
//找到键盘输入的文件名的位置
int getFirstArgumentPosition(const char *commandBuffer, int *position);
//客户端发送文件到服务端
int putFile(int sockFd, const char *fileName);
//客户端从服务端下载文件
int getFile(int sockFd, const char *fileName);
//获得数据帧的信息
int getOneFrame(int sockFd, Protocol_t *pProtocol);
//登录函数
int userSignIn(int sockFd);

int main(int argc, char *argv[]){

	int sfd = socket(AF_INET,SOCK_STREAM,0);
	ERROR_CHECK(sfd,-1,"socket");

	int ret = 0;

	//保存服务端的ip地址和端口
	struct sockaddr_in serAddr;
	memset(&serAddr,0,sizeof(serAddr));
	serAddr.sin_family = AF_INET;
	serAddr.sin_addr.s_addr = inet_addr(argv[1]);
	serAddr.sin_port = htons(atoi(argv[2]));

	//将客户端连接到服务端
	ret = connect(sfd,(struct sockaddr*)&serAddr,sizeof(serAddr));
	ERROR_CHECK(ret,-1,"connect");

	//用户登录
	userSignIn(sfd);

	Protocol_t *pProtocol = (Protocol_t *)malloc(sizeof(*pProtocol) + MAXSIZE + 1);
	//使用select函数控制逻辑
	fd_set rdset;
	FD_ZERO(&rdset);
	char buf[128] = {0};

	int readyNum = 0;
	while(1){
		//把需要监听的文件描述符加入到集合中
		FD_SET(STDIN_FILENO, &rdset);
		FD_SET(sfd, &rdset);
		readyNum = select(sfd + 1, &rdset, NULL, NULL, NULL);
		for(int i = 0; i < readyNum; ++i){
			//如果是标准输入就绪，就代表要发送数据给对端
			if(FD_ISSET(STDIN_FILENO, &rdset)){
				memset(buf, 0, sizeof(buf));
				int readLen = read(STDIN_FILENO, buf, sizeof(buf));
				buf[readLen - 1] = 0;
				if(!strncmp("puts", buf, 4)){
					//传递指令类型，文件名，文件名大小给服务端
					sendCommand(sfd, buf, PUTS);
					//上传文件到服务端
					char *fileName = buf;
					while(' ' != *fileName){
						fileName ++;
					}
					while(' ' == *fileName){
						fileName ++;
					}
					putFile(sfd, fileName);
				}
				else if(!strncmp("gets", buf, 4)){
					sendCommand(sfd, buf, GETS);

					//获取文件到客户端
					char *fileName = buf;
					while(' ' != *fileName){
						fileName ++;
					}
					while(' ' == *fileName){
						fileName ++;
					}
					getFile(sfd, fileName);
				}
				else if(!strncmp("cd", buf, 2)){
					sendCommand(sfd, buf, CD);

				}
				else if(!strncmp("ls", buf, 2)){
					pProtocol->type = LS;
					pProtocol->dataLen = 0;
					send(sfd, pProtocol, getProtocolSize(pProtocol), 0);

				}
				else if(!strncmp("pwd", buf, 3)){
					pProtocol->type = PWD;
					pProtocol->dataLen = 0;
					send(sfd, pProtocol, getProtocolSize(pProtocol), 0);
				}
				else if(!strncmp("rm", buf, 2)){
					sendCommand(sfd, buf, RM);
				}
				else if(!strncmp("mkdir", buf, 5)){
					sendCommand(sfd, buf, MKDIR);
				}
			}
			if(FD_ISSET(sfd, &rdset)){
				memset(pProtocol->data, 0, MAXSIZE+1);
				getOneFrame(sfd, pProtocol);
				if(pProtocol->type == MESSAGE){
					printf("%s\n", pProtocol->data);
				}

			}

		}
	}
	return 0;

	

}

int getFirstArgumentPosition(const char *commandBuffer, int *position){
	*position = 0;
	while(' ' != *commandBuffer){
		commandBuffer++;
		(*position)++;
	}
	while(' ' == *commandBuffer){
		commandBuffer++;
		(*position)++;
	}
	//找到文件名停止
	return 0;
}

int sendCommand(int sockFd, const char *commandBuffer, int commandType){
	Protocol_t *pProtocol = (Protocol_t *)malloc(sizeof(*pProtocol) + MAXSIZE +1);
	//传递指令类型
	pProtocol->type = commandType; 
	int position = 0;
	getFirstArgumentPosition(commandBuffer, &position);
	//传递文件名长度
	pProtocol->dataLen = strlen(commandBuffer + position);
	//传递文件名字
	strcpy(pProtocol->data, commandBuffer + position);
	//send发送到服务端
	send(sockFd, pProtocol, getProtocolSize(pProtocol), 0);
	return 0;
}

static inline int recvJudge(int recvResult){
	if(0 == recvResult){
		printf("客户端已关闭连接!\n");
		return -2;
	}
	if(-1 == recvResult){
		return -1;
	}
		return 0;
	
}

int getOneFrame(int sockFd, Protocol_t *pProtocol){
	//定义接收信息的数据类型
	int type = 0;
	int dataLen = 0;

	//定义 接收 函数返回值的数据类型
	int ret = 0;

	//获取数据状态
	ret = recv(sockFd, &type, sizeof(type), MSG_WAITALL);

	//如果读取异常,返回定义的异常值
	if(0 != (ret = recvJudge(ret))){
		return ret;
	}

	//获取数据大小
	ret = recv(sockFd, &dataLen, sizeof(dataLen), MSG_WAITALL);

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
	ret = recv(sockFd, pProtocol->data, pProtocol->dataLen, MSG_WAITALL);

	//如果读取异常，返回定义的异常值
	if(0 != (ret = recvJudge(ret))){
		return ret;
	}

	return 0;


}


int putFile(int sockFd, const char *fileName){
		//返回函数的值
	int ret = 0;

	//使用堆空间创建传输数据的私有协议结构体
	Protocol_t *pProtocol = (Protocol_t *)calloc(1, sizeof(*pProtocol) + MAXSIZE + 1);

	pProtocol->dataLen = strlen(fileName);
	//通过getOneFrame获得客户端的判断信息
	getOneFrame(sockFd, pProtocol);
	//判断是否存在同名文件
	off_t offset = 0;
	if(pProtocol->type == FILE_EXIST){
		printf("文件已经存在！\n");
		free(pProtocol);
		return 0;
	}

	//打开文件
	int fd = open(fileName, O_RDWR);
	if(-1 == fd){
		printf("传输文件失败，请检查文件名！\n");
		free(pProtocol);
		return 0;
	}

	//获取文件信息
	struct stat fileInfo;
	memset(&fileInfo, 0, sizeof(fileInfo));
	fstat(fd, &fileInfo);

	//设置发送文件大小的协议结构体
	pProtocol->type = FILE_SIZE;
	pProtocol->dataLen = sizeof(fileInfo.st_size);
	memcpy(pProtocol->data, &fileInfo.st_size, pProtocol->dataLen);

	//发送存储文件大小的协议结构体
	if(pProtocol->type == FILE_SIZE){
		send(sockFd, pProtocol, getProtocolSize(pProtocol), 0);
		printf("fileInfo.st_size = %ld\n", fileInfo.st_size);

	}
	
	//发送文件数据
	pProtocol->type = FILE_DATA;
	//如果文件大小小于100MB，采用循环发送
	if(fileInfo.st_size <= (1 << 20) * 100){
		//从上次下载终止的地方继续下载
		lseek(fd, offset, SEEK_SET);
		int sendLen = 0;
		while(sendLen < fileInfo.st_size){
			pProtocol->dataLen = read(fd, pProtocol->data, MAXSIZE);
			ret = send(sockFd, pProtocol, getProtocolSize(pProtocol),0);
			sendLen += ret;

			if(-1 == ret){
				break;
			}

			if(0 == pProtocol->dataLen){
				break;
			}


		}
		
	}
	else{
		//文件大小大于100MB，使用mmap发送数据
		char *pMap = (char *)mmap(NULL, fileInfo.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		ERROR_CHECK(pMap, (char *)-1, "mmap");
		//从上次下载终止的地方继续下载
		send(sockFd, pMap + offset, fileInfo.st_size - offset, 0);
		munmap(pMap, fileInfo.st_size);
	}

	close(fd);
	free(pProtocol);
	return 0;

}


int recvCycle(int sockFd, void *pData, int totalLen){
	int ret = 0;
	int recvLen = 0;
	while(recvLen < totalLen){
		ret = recv(sockFd, (char *)pData + recvLen, totalLen - recvLen, 0);
		recvLen += ret;
		printf("recvLen = %d\n", recvLen);
		//客户端关闭时
		if(ret == 0){
			printf("bye bye\n");
			return -1;
		}
	}
	return recvLen;
}

int getFile(int sockFd, const char *fileName){
	int ret = 0;
	int fd = 0;

	int type = 0;//读取数据状态
	int dataLen = 0;//读取数据大小

	off_t offset;//已存在的文件大小
	off_t recvLen = 0;//接收的文件长度
	off_t fileLen = 0;//文件的总长度
	char data[MAXSIZE] = {0};//存放数据

	//使用堆空间创建传输数据的私有协议结构体
	Protocol_t *pProtocol = (Protocol_t *)calloc(1, sizeof(*pProtocol) + MAXSIZE +1);

	//判断同名文件是否存在, 并传递偏移量
	pProtocol->type = FILE_OFFSET;
	pProtocol->dataLen = sizeof(offset);
	if(-1 != (open(fileName, O_RDONLY))){
		struct stat fileStatus;
		fstat(fd, &fileStatus);
		offset = fileStatus.st_size;
		recvLen += offset;

		close(fd);
	}

	//客户端传送偏移量数据到服务端
	memcpy(pProtocol->data, &offset, sizeof(offset));
	send(sockFd, pProtocol, getProtocolSize(pProtocol), 0);


	//创建同名文件
	fd = open(fileName, O_RDWR|O_CREAT);
	if(-1 == fd){
		printf("创建文件失败！\n");
		free(pProtocol);
		return 0;
	}

	//接收文件的大小信息
	getOneFrame(sockFd, pProtocol);
	memcpy(&fileLen, pProtocol->data, sizeof(fileLen));
	printf("文件总长度： %ld\n", fileLen);


	//接收文件内容
	//小文件不超过100M
	//接收协议结构体的数据放在char data[MAXSIZE]中，再将char data[MAXSIZE]中的数据传给文件
	if(fileLen <= (1 << 20) * 100){
		ret = ftruncate(fd, fileLen);
		ERROR_CHECK(ret, -1, "ftruncate");
		while(recvLen < fileLen){
			memset(data, 0, sizeof(data));
			//读putFile send 过来的协议体数据，dataLen <= MAXSIZE
			//下面三行代码可以体现私有协议结构体的分隔符，一次发送，分三段接收
			recv(sockFd, &type, sizeof(type), MSG_WAITALL);
			recv(sockFd, &dataLen, sizeof(dataLen), MSG_WAITALL);
			ret = recvCycle(sockFd, data, dataLen);

			//下载进度显示
			recvLen += ret;
			float rate = (float)recvLen / fileLen * 100;
			printf("rate = %5.2f %%\r", rate);
			fflush(stdout);

			//循环结束条件
			if(0 == ret){
				break;
			}

			//写入文件
			write(fd, data, ret);

		}
	}
	else{
		//当文件超过100M，采用mmap方式传输
		ret = ftruncate(fd, fileLen);
		ERROR_CHECK(ret, -1, "ftruncate");
		char *pMap = (char *)mmap(NULL, fileLen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
		ERROR_CHECK(pMap, (char *)-1, "mmap");
		ret = recvCycle(sockFd, pMap, fileLen);
		munmap(pMap, fileLen);
	}
	printf("文件：%s 已成功下载！\n", fileName);
	close(fd);
	free(pProtocol);
	return 0;	

}

int userSignIn(int sockFd){
	Protocol_t *pProtocol = (Protocol_t *)calloc(1, sizeof(*pProtocol) + MAXSIZE + 1);

	while(1){
		//设置登录请求的类型
		pProtocol->type =  USER_SIGNIN;
		//输入用户名
		printf("请输入名字：");
		fflush(stdout);
		scanf("%s", pProtocol->data);

		//用户名和密码在输入终端之间用空格隔开
		pProtocol->data[strlen(pProtocol->data)] = ' ';

		//密码不在终端显示
		char *password = getpass("请输入密码：");
		strcpy(pProtocol->data + strlen(pProtocol->data), password);

		pProtocol->dataLen = strlen(pProtocol->data);

		int ret = send(sockFd, pProtocol, getProtocolSize(pProtocol), 0);
		ERROR_CHECK(ret, -1, "send");

		getOneFrame(sockFd, pProtocol);
		if(pProtocol->type == PASSWD_RIGHT){
			printf("密码正确！\n");
			break;
		}
		printf("密码或用户名错误！\n");


	}
	free(pProtocol);
	return 0;

}

