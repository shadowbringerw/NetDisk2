#include "../inc/transmitFile.h"
#include "../inc/threadPool.h"
#include "../inc/main.h"
#include "../inc/md5.h"


//用while循环接收小文件
//把协议结构体体的数据读取进来
int recvCycle(int clientFd, void *pData, int totalLen){
	int ret = 0;
	int recvLen = 0;
	while(recvLen < totalLen){
		ret = recv(clientFd, (char *)pData + recvLen, totalLen - recvLen, 0);
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


//客户端发送文件到服务端,服务端接收文件
int getFile(int clientFd, const char *fileName){
	int ret = 0;
	int fd = 0;

	int type = 0;//读取数据状态
	int dataLen = 0;//读取数据大小

	off_t recvLen = 0;//接收的文件长度
	off_t fileLen = 0;//文件的总长度
	char data[MAXSIZE] = {0};//存放数据

	//使用堆空间创建传输数据的私有协议结构体
	Protocol_t *pProtocol = (Protocol_t *)calloc(1, sizeof(*pProtocol) + MAXSIZE +1);

	//判断同名文件是否存在
	if(-1 != (open(fileName, O_RDONLY))){
		pProtocol->type = FILE_EXIST;
		pProtocol->dataLen = 0;
		//告诉对端不用传送文件过来
		send(clientFd, pProtocol, getProtocolSize(pProtocol), 0);
		close(fd);
		free(pProtocol);
		return 0;
	}

	//如果同名文件不存在
	pProtocol->type = FILE_TRANS_START;
	pProtocol->dataLen = 0;
	//告诉对端可以传送文件过来
	send(clientFd, pProtocol, getProtocolSize(pProtocol), 0);

	//创建同名文件
	fd = open(fileName, O_RDWR|O_CREAT);
	if(-1 == fd){
		printf("创建文件失败！\n");
		free(pProtocol);
		return 0;
	}

	//接收文件的大小信息
	getOneFrame(clientFd, pProtocol);
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
			recv(clientFd, &type, sizeof(type), MSG_WAITALL);
			recv(clientFd, &dataLen, sizeof(dataLen), MSG_WAITALL);
			ret = recvCycle(clientFd, data, dataLen);

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
		ret = recvCycle(clientFd, pMap, fileLen);
		munmap(pMap, fileLen);
	}
	printf("文件：%s 已成功下载！\n", fileName);
	close(fd);
	free(pProtocol);
	return 0;	

}

//服务器向客户端上传文件
//和客户端的putFile有偏移量的区别,因为客户端的getFile发送的是偏移量
int putFile(int clientFd, const char *fileName){
	//返回函数的值
	int ret = 0;

	//使用堆空间创建传输数据的私有协议结构体
	Protocol_t *pProtocol = (Protocol_t *)calloc(1, sizeof(*pProtocol) + MAXSIZE + 1);

	pProtocol->dataLen = strlen(fileName);
	//通过getOneFrame获得客户端的判断信息
	getOneFrame(clientFd, pProtocol);
	//解决断点重传
	off_t offset = 0;
	if(pProtocol->type == FILE_OFFSET){
		memcpy(&offset, pProtocol->data, sizeof(offset));
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
		send(clientFd, pProtocol, getProtocolSize(pProtocol), 0);
		printf("fileInfo.st_size = %ld\n", fileInfo.st_size);

	}
	
	//发送文件数据
	pProtocol->type = FILE_DATA;
	//如果文件大小小于100MB，采用循环发送
	if(fileInfo.st_size <= (1 << 20) * 100){
		//从上次下载终止的地方继续上传
		lseek(fd, offset, SEEK_SET);
		int sendLen = 0;
		while(sendLen < fileInfo.st_size){
			pProtocol->dataLen = read(fd, pProtocol->data, MAXSIZE);
			ret = send(clientFd, pProtocol, getProtocolSize(pProtocol),0);
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
		send(clientFd, pMap + offset, fileInfo.st_size - offset, 0);
		munmap(pMap, fileInfo.st_size);
	}

	close(fd);
	free(pProtocol);
	return 0;


}