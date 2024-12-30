/*
 * rtos_thread.h
 *
 *  Created on: Dec 25, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_THREAD_H_
#define INC_RTOS_THREAD_H_

#include "rtos_config.h"

typedef struct thread_t
{
	uint32_t pStackPointer;
	uint32_t priority;
	uint32_t threadID;
	RTOS_listItem_t listItem;
	RTOS_listItem_t eventListItem;
} RTOS_thread_t;

typedef struct
{
	uint64_t thread[THREAD_STACK_SIZE];
} RTOS_stack_t;

void RTOS_threadInitLists(void);
void RTOS_threadCreate(RTOS_thread_t * pThread, RTOS_stack_t * pStack, uint32_t priority, void * pFunction);
void RTOS_SVC_threadCreate(RTOS_thread_t * pThread, RTOS_stack_t * pStack, uint32_t priority, void * pFunction);
RTOS_thread_t * RTOS_threadGetCurrentReady(void);
void RTOS_threadSwitchRunning(void);
RTOS_thread_t * RTOS_threadGetRunning(void);
void RTOS_threadAddToReadyList(RTOS_thread_t * pThread);
void RTOS_threadRefreshTimerList(void);
void RTOS_threadAddRunningToTimerList(uint32_t waitTime);
void RTOS_threadDestroy(RTOS_thread_t * pThread);
void RTOS_SVC_threadDestroy(RTOS_thread_t * pThread);
void RTOS_SVC_threadDelay(uint32_t waitTime);

#endif /* INC_RTOS_THREAD_H_ */
