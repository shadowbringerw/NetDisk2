#ifndef __WORKQUE_H__
#define __WORKQUE_H__
#include "main.h"

typedef struct node{
	int clientFd;
	struct node* pNext;
}Node_t, *pNode_t;


typedef struct{
	int size;
	pNode_t pHead, pTail;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}Que_t, *pQue_t;

int queInit(pQue_t pQue);
int queInsert(pQue_t pQue, pNode_t pNew);
int queGet(pQue_t pQue, pNode_t *pGet);

#endif