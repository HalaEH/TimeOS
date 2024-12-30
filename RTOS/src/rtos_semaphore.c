/*
 * rtos_semaphore.c
 *
 *  Created on: Dec 28, 2024
 *      Author: halaabdelrahman
 */

#include "rtos.h"

/**
 * @brief Initializes a semaphore object for use in the RTOS.
 *
 * This function creates a semaphore by initializing its waiting list and setting
 * its initial value. The semaphore can be used for synchronization between tasks.
 *
 * @param[in,out] pSemaphore       Pointer to the semaphore object to be initialized.
 *                             Must not be NULL.
 * @param[in]     initialValue
 *
 */
void RTOS_semaphoreCreate(RTOS_semaphore_t * pSemaphore, uint32_t initialValue)
{
	ASSERT(pSemaphore != NULL);
	RTOS_listInit(&pSemaphore->waitingList);
	pSemaphore->semaphoreValue = initialValue;
}

/**
 * @brief Attempts to take (decrement) a semaphore, with optional blocking behavior.
 *
 * This function tries to acquire a semaphore by decrementing its value. If the semaphore
 * value is greater than zero, it is decremented, and the function returns success. If the
 * semaphore value is zero, the behavior depends on the `waitFlag`:
 * - If `waitFlag` is 1, the calling thread is added to the semaphore's waiting list, and
 *   a context switch is triggered to allow other threads to execute.
 * - If `waitFlag` is 0, the function returns immediately without blocking.
 *
 * @param[in,out] pSemaphore Pointer to the semaphore object to be taken. Must not be NULL.
 * @param[in]     waitFlag   Determines blocking behavior:
 *                           0 - Do not block if the semaphore is unavailable.
 *                           1 - Block and wait for the semaphore to become available.
 *
 * @return Status of the semaphore take operation:
 *         1 - Semaphore successfully taken (decremented).
 *         2 - Semaphore not available, thread added to the waiting list (only if `waitFlag` is 1).
 *
 */
uint32_t RTOS_semaphoreTake(RTOS_semaphore_t * pSemaphore, int32_t waitTime)
{
	ASSERT(pSemaphore != NULL);
	ASSERT(waitTime >= WAIT_INDEFINITELY);

	RTOS_thread_t * pRunningThread;
	RTOS_return_t returnStatus = RTOS_FAILURE;
	uint32_t semaphoreValueTemp = 0;
	uint32_t terminate = 0;

	while(terminate != 1)
	{
		semaphoreValueTemp = __LDREXW(&pSemaphore->semaphoreValue);
				if(semaphoreValueTemp > 0)
				{
					if(__STREXW((semaphoreValueTemp - 1), &pSemaphore->semaphoreValue) == 0)
					{
						__DMB();
						returnStatus = RTOS_SUCCESS;
						terminate = 1;
					}else
					{
						/* Semaphore can not be locked */
					}
				}else
				{
					/* Semaphore is locked, terminate the loop */
					terminate = 1;
				}
	}

	if((waitTime != NO_WAIT) && (returnStatus != RTOS_SUCCESS))
	{
		pRunningThread = RTOS_threadGetRunning();
		RTOS_listRemove(&pRunningThread->listItem);
		RTOS_listInsert(&pSemaphore->waitingList, &pRunningThread->eventListItem);
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
		if(waitTime > NO_WAIT)
		{
			RTOS_threadAddRunningToTimerList(waitTime);
		}
		returnStatus = RTOS_CONTEXT_SWITCH_TRIGGERED;
	}else
	{
		/* No blocking required, do nothing  */
	}

	return returnStatus;
}

/**
 * @brief Releases a semaphore and wakes up a waiting thread if necessary.
 *
 * This function increments the value of a semaphore, indicating that the resource it
 * represents has been released. If any threads are waiting for the semaphore, the
 * highest-priority thread is moved from the waiting list to the ready list to resume execution.
 * If no threads are waiting, the function simply releases the semaphore without further action.
 *
 * @param[in,out] pSemaphore Pointer to the semaphore object to be released. Must not be NULL.
 *
 */
void RTOS_semaphoreGive(RTOS_semaphore_t * pSemaphore)
{
	ASSERT(pSemaphore != NULL);
	RTOS_thread_t * pThread;
	uint32_t semaphoreValueTemp = 0;
	uint32_t terminate = 0;
	while(terminate != 1)
	{
		semaphoreValueTemp = __LDREXW(&pSemaphore->semaphoreValue);
		semaphoreValueTemp++;
		if(__STREXW(semaphoreValueTemp, &pSemaphore->semaphoreValue) == 0)
		{
			terminate = 1;
			__DMB();
		}else
		{
			/* Store failed, try again */
		}
	}


	if((pSemaphore->waitingList.numOfItems) > 0)
	{
		pThread = pSemaphore->waitingList.listEnd.pNext->pThread;
		ASSERT(pThread != NULL);
		RTOS_listRemove(&pThread->eventListItem);
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
