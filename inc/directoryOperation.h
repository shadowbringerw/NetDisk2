#ifndef __DIRECTORYOPERATION_H__
#define __DIRECTORYOPERATION_H__

int changeDirectory(const char *path); //cd
int listFile(const char *path, char *message, int length); //ls
int printWorkDirectory(char *buf, int length); //pwd
int removeFile(const char *fileName); //rm
int makeDirectory(const char *dir); //mkdir


#endif