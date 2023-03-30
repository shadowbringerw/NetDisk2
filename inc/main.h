#define _GNU_SOURCE
#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdio.h>
#include<sys/stat.h>
#include<unistd.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<stdlib.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include<fcntl.h>
#include<malloc.h>
#include<sys/mman.h>
#include<sys/time.h>
#include<sys/select.h>
#include<sys/wait.h>
#include<syslog.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/msg.h>
#include<sys/sem.h>
#include<signal.h>
#include<pthread.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/sendfile.h>

#define ARGS_CHECK(argc,num){ if(argc != num){ fprintf(stderr,"ARGS ERROR!\n"); return -1; } }
#define ERROR_CHECK(ret,num,msg){if(ret==num)\
    {perror(msg);return -1;}}
#define THREAD_ERROR_CHECK(ret,funcName)\
    do { \
        if (0 != ret) { \
            printf("%s : %s\n", funcName,strerror(ret));\
        }\
    }while(0)




//设置通信协议数据的最大长度
#define MAXSIZE 1500 

//设置传输数据的私有协议
typedef struct{
    int type; //传输的数据状态
    int dataLen; //数据的大小
    char data[]; //实际使用的信息
}Protocol_t;

inline
static int getProtocolSize(const Protocol_t *pProtocol){
    return sizeof(*pProtocol) + pProtocol->dataLen;
}

//使用枚举类型 设置传输数据类型
enum {
    CD = 0x1000,
    LS = 0x1001,
    PWD = 0x1002,
    RM = 0x1003,
    MKDIR = 0x1004,
    PUTS = 0x1005,
    GETS = 0x1006,
    MESSAGE = 0x1007, //服务器回传给客户端提示信息

    FILE_SIZE = 0x2001, //传输文件的大小
    FILE_DATA = 0x2002, //表示要实际传输文件数据
    FILE_MD5 = 0x2003, //文件的md5值
    FILE_OFFSET = 0x2004, //文件偏移
    FILE_EXIST = 0x2005, //表示文件已经存在
    FILE_TRANS_START = 0x2006, //表示可以传输文件

    USER_SIGNIN = 0x3000,  // 用户登录信息
    PASSWD_ERROR = 0x3001,  // 密码错误
    PASSWD_RIGHT = 0x3002  // 密码正确

};

#endif




