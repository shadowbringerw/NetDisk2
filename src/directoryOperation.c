#include "../inc/directoryOperation.h"
#include "../inc/main.h"

//进入服务器对应目录
int changeDirectory(const char *path){
	int ret = 0;
	ret = chdir(path);
	ERROR_CHECK(ret, -1, "chdir");
	return 0;
}

//静态函数：使每个函数只在一个源文件中生效，不能被其他源文件所用
//获得文件的信息：状态，文件名字，文件大小等
static char *getFileMessage(char *fileName,int maxNameLength){
	static char fileMessage[200];
	struct stat fileStatBuf;
	stat(fileName, &fileStatBuf);
	//%-*s 代表输入一个字符串，-号代表左对齐、后补空白，*号代表对齐宽度由输入时确定
	//对齐宽度按照文件名的最长长度决定
	sprintf(fileMessage, "%s %-*s %ld", S_ISDIR(fileStatBuf.st_mode)? "d" : "f", maxNameLength, fileName, fileStatBuf.st_size);
	//%X的意思是以十六进制数形式输出整数
	printf("%s mode 0x%X\n",fileName, fileStatBuf.st_mode);
	return fileMessage;
}


//列出服务器上相应的目录和文件
int listFile(const char *path, char *message, int length){
	DIR *dirp = opendir(path);
	ERROR_CHECK(dirp, NULL, "opendir");
	//设置二维数组，暂时存储文件名和文件名长度
	char fileNames[200][30];
	int fileNum = 0;
	
	//访问目录项
	struct dirent *pdirent;
	//循环检查下面有多少文件，并把文件名放在fileNames数组里
	while((pdirent = readdir(dirp)) != NULL){
		char fname[20];
		strcpy(fname, pdirent->d_name);
		if(fname[0] != '.'){
			strcpy(fileNames[fileNum], pdirent->d_name);
			fileNum++;
		}

	}
	closedir(dirp);

	//文件名排序
	char tempName[30];
	for(int i = 0;i < fileNum;++i){
		for(int j = i + 1;j < fileNum;++j){
			if(strcmp(fileNames[i],fileNames[j]) > 0){
				strcpy(tempName, fileNames[i]);
				strcpy(fileNames[i], fileNames[j]);
				strcpy(fileNames[j], tempName);
			}
		}
	}

	//确定所有文件里最长的文件名的长度是多少,作用是用来对齐
	int max_len = 0;
	for(int i = 0;i < fileNum;++i){
		int len = strlen(fileNames[i]);
		if(max_len < len){
			max_len = len;
		}
	}

	//打印获取的信息
	int cnt = 0;
	//sprintf如果成功，返回写入的字符总数
	for(int i = 0; i < fileNum; i++){
		//数组指针不断偏移
		cnt += sprintf(message + cnt, "%s\n", getFileMessage(fileNames[i], max_len));
	}
	//结束符
	message[cnt - 1] = 0;
	return 0;


}

//显示目前所在路径，设置传出参数buf
int printWorkDirectory(char *buf, int length){
	char *ret = getcwd(NULL, length);
	strcpy(buf, ret);
	return 0;

}

//删除服务器上的文件
int removeFile(const char *fileName){
	int ret = remove(fileName);
	ERROR_CHECK(ret, -1, "remove");
	return 0;
}

//创建文件夹
int makeDirectory(const char *dir){
	int ret = mkdir(dir, 0777);
	ERROR_CHECK(ret, -1, "mkdir");
	return 0;
}