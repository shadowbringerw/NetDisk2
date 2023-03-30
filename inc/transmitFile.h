#ifndef __TRANSMITFILE_H__
#define __TRANSMITFILE_H__

int getFile(int clientFd, const char *fileName);
int putFile(int clientFd, const char *fileName);

#endif