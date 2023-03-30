#ifndef __CONFIG_H__
#define __CONFIG_H__

//获取本服务器的ip地址，端口号，线程数量

typedef struct{
	char ip[20];
	char port[10];
	char threadNum[10];
}config_t;

//将配置文件的名字作为参数，设置传入参数获取配置文件内容
int getConfig(const char *pConfigFileName,config_t *pConfig);




#endif
