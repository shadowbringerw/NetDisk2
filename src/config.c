#include "../inc/config.h"
#include "../inc/main.h"

int getConfig(const char * pConfigFileName,config_t *pConfig){
	FILE *file = fopen(pConfigFileName,"r");
	ERROR_CHECK(file,NULL,"open file failed!");

	fscanf(file, "%*s %*s %s", pConfig->ip);
	fscanf(file, "%*s %*s %s", pConfig->port);
	fscanf(file, "%*s %*s %s", pConfig->threadNum);

	return 0;

}

//fscanf函数的作用是将FILE文件流中的数据读取什么
//int fscanf(FILE *fp,char *format,....)
//第一个参数：文件流
//第二个参数：格式
//后面的参数：最终要写入的内存空间
//可以体会到fscanf的指针一直是后移的
//%*s 是跳过一个字符串，用第一个fscanf举例，即跳过了“server_ip = ”