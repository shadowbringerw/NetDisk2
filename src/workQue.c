#include "../inc/workQue.h"
#include "../inc/main.h"

//初始化队列
int queInit(pQue_t pQue){
	pQue->size = 0;
	pQue->pHead = NULL;
	pQue->pTail = NULL;
	pthread_mutex_init(&pQue->mutex,NULL);
	pthread_cond_init(&pQue->cond,NULL);
	return 0;
}

//队列插入新的结点
int queInsert(pQue_t pQue,pNode_t pNew){
	if(0 == pQue->size){
		pQue->pHead = pNew;
		pQue->pTail = pNew;
	}
	else{
		pQue->pTail->pNext = pNew;
		pQue->pTail = pNew;
	}
	pQue->size ++;
	return 0;
	
}

//获得队列中的结点
int queGet(pQue_t pQue, pNode_t *pGet){
	if(pQue->pHead == NULL){
		return -1;
	}
	*pGet = pQue->pHead;
	pQue->pHead = pQue->pHead->pNext;
	pQue->size --;
	if(pQue->size == 0){
		pQue->pTail = NULL;
	}
	return 0;
}