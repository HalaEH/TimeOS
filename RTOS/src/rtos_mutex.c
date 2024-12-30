/*
 * mutex.c
 *
 *  Created on: Dec 28, 2024
 *      Author: halaabdelrahman
 */

#include "rtos.h"

/**
 * @brief Initializes a mutex object for use in the RTOS.
 *
 * This function creates a mutex by initializing its waiting list and setting
 * its initial value. The mutex can be used for synchronization between tasks.
 *
 * @param[in,out] pMutex       Pointer to the mutex object to be initialized.
 *                             Must not be NULL.
 * @param[in]     initialValue Initial value of the mutex (0 or 1).
 *                             0 indicates the mutex is locked.
 *                             1 indicates the mutex is available.
 *
 */
void RTOS_mutexCreate(RTOS_mutex_t * pMutex, uint32_t initialValue)
{
	/* Check input parameters */
	ASSERT(pMutex != NULL);
	ASSERT((initialValue == 0) || (initialValue == 1));

	/* Initialize mutex waiting list */
	RTOS_listInit(&pMutex->waitingList);

	/* Initialize mutex value */
	pMutex->mutexValue = initialValue;

}

/**
 * @brief Attempts to lock a mutex, with optional blocking behavior.
 *
 * This function attempts to acquire a mutex lock. If the mutex is available,
 * it locks the mutex and returns immediately. If the mutex is not available,
 * the behavior depends on the `waitFlag`:
 * - If `waitFlag` is 1, the calling thread is added to the mutex's waiting list,
 *   and the PendSV interrupt is triggered to perform a context switch.
 * - If `waitFlag` is 0, the function returns without blocking.
 *
 * @param[in,out] pMutex   Pointer to the mutex object to be locked. Must not be NULL.
 * @param[in]     waitFlag Determines blocking behavior:
 *                         0 - Do not block if the mutex is locked.
 *                         1 - Block and wait for the mutex to become available.
 *
 * @return Status of the lock operation:
 *         1 - Mutex successfully locked.
 *         2 - Mutex not locked, thread added to the waiting list (only if `waitFlag` is 1).
 *
 */
uint32_t RTOS_mutexLock(RTOS_mutex_t * pMutex, int32_t waitTime)
{
	/* Check input parameters */
	ASSERT(pMutex != NULL);
	ASSERT(waitTime >= WAIT_INDEFINITELY);

	RTOS_thread_t * pRunningThread;
	RTOS_return_t returnStatus = RTOS_FAILURE;
	uint32_t terminate = 0;

	while(terminate != 1)
	{
		if(__LDREXW(&pMutex->mutexValue) == 1)
		{
			/* Mutex is free, lock it*/
			if(__STREXW(0, &pMutex->mutexValue) == 0)
			{
				__DMB();
				returnStatus = RTOS_SUCCESS;
				terminate = 1;
			}else
			{
				/* Mutex can not be locked */
			}
		}else
		{
			/* Mutex is locked, terminate the loop */
			terminate = 1;
		}
	}

	if((waitTime != NO_WAIT) && (returnStatus != RTOS_SUCCESS))
	{
		pRunningThread = RTOS_threadGetRunning();
		RTOS_listRemove(&pRunningThread->listItem);
		RTOS_listInsert(&pMutex->waitingList, &pRunningThread->eventListItem);
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
		if(waitTime > NO_WAIT)
		{
			RTOS_threadAddRunningToTimerList(waitTime);
		}
		returnStatus = RTOS_CONTEXT_SWITCH_TRIGGERED;
	}

	return returnStatus;
}

/**
 * @brief Releases a mutex and handles any threads waiting for the mutex.
 *
 * This function unlocks a previously locked mutex, making it available for use.
 * If there are threads waiting for the mutex, the highest-priority thread (based
 * on the RTOS's waiting list order) is removed from the waiting list and added
 * to the ready list to resume execution.
 *
 * @param[in,out] pMutex Pointer to the mutex object to be released. Must not be NULL.
 *
 */
void RTOS_mutexRelease(RTOS_mutex_t * pMutex)
{
	ASSERT(pMutex != NULL);
	RTOS_thread_t * pThread;
	__DMB();
	pMutex->mutexValue = 1;
	/* Get highest priority waiting thread, the one next from the end */
	if(pMutex->waitingList.numOfItems > 0)
	{
		pThread = pMutex->waitingList.listEnd.pNext->pThread;
		ASSERT(pThread != NULL);
		RTOS_listRemove(&pThread->listItem);
		if(pThread->listItem.pList != NULL)
		{
			RTOS_listRemove(&pThread->listItem);
		}
		RTOS_threadAddToReadyList(pThread);
	}else
	{
		/* No threads are waiting, do nothing */
	}
}

