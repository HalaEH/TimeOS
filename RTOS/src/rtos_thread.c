/*
 * rtos_thread.c
 *
 *  Created on: Dec 25, 2024
 *      Author: halaabdelrahman
 */

#include "rtos.h"

static RTOS_list_t readyList[THREAD_PRIORITY_LEVELS];
static RTOS_list_t timerList;
static uint32_t currentTopPriority = (THREAD_PRIORITY_LEVELS - 1);
static RTOS_thread_t * pRunningThread;
static uint32_t runningThreadID = 0;
static uint32_t numOfThreads = 0;
static volatile uint32_t sysTickCounter = 0;

/**
 * @brief Initializes the ready lists for all thread priority levels.
 *
 * This function sets up the ready lists used by the RTOS to manage threads
 * at different priority levels. Each priority level is associated with its
 * own list, and all lists are initialized to an empty state.
 *
 */
void RTOS_threadInitLists(void)
{
	for(uint32_t priority = 0; priority < THREAD_PRIORITY_LEVELS; priority++)
	{
		RTOS_listInit(&readyList[priority]);
	}
	RTOS_listInit(&timerList);
}

/**
 * @brief Creates and initializes a new RTOS thread.
 *
 * This function sets up a thread's stack, priority, and context, and adds the thread to the appropriate ready list.
 * It prepares the thread for execution by the RTOS scheduler and triggers a context switch if necessary.
 *
 * @param pThread Pointer to the RTOS_thread_t structure for the thread.
 * @param pStack Pointer to the memory allocated for the thread's stack.
 * @param priority Priority level of the thread, must be less than THREAD_PRIORITY_LEVELS.
 * @param pFunction Pointer to the thread's entry function.
 *
 */
void RTOS_threadCreate(RTOS_thread_t * pThread, RTOS_stack_t * pStack, uint32_t priority, void * pFunction)
{
	/* Check input parameters */
	ASSERT(pThread != NULL);
	ASSERT(pStack != NULL);
	ASSERT(THREAD_PRIORITY_LEVELS > priority);
	ASSERT(pFunction != NULL);

	/* Create stack frame, size multiplied by 8 for the double word
	* length converted to byte length, stack frame size is 19 words */
	pThread->pStackPointer = ((uint32_t) pStack + THREAD_STACK_SIZE * 8 - 18 * 4);

	/* Write thread function into return address */
	MEM32_ADDRESS((pThread->pStackPointer + (16 << 2))) = (uint32_t) pFunction;

	/* Write initial xPSR, program status register, thumb */
	MEM32_ADDRESS((pThread->pStackPointer + (17 << 2))) = 0x01000000;

	/* Write EXC_RETURN, since the execution threads are using PSP, this will allow SVC to return to the thread with PSP */
	MEM32_ADDRESS((pThread->pStackPointer)) = 0xFFFFFFFDUL;

	/* Write initial CONTROL register value UNPRIVILIGED, PSP & no FPU */
	MEM32_ADDRESS((pThread->pStackPointer + (1 << 2))) = 0x3;

	/* Set thread priority */
	pThread->priority = priority;

	if(pThread->threadID == 0)
	{
		pThread->threadID = ++numOfThreads;
	}


	/* Thread is not yet in the list */
	pThread->listItem.pList = NULL;
	pThread->eventListItem.pList = NULL;

	/* Link this thread with its list item */
	pThread->listItem.pThread = (void *) pThread;
	pThread->eventListItem.pThread = (void *) pThread;

	pThread->eventListItem.itemValue = priority;

	RTOS_threadAddToReadyList(pThread);
}


/**
 * @brief Retrieves the currently ready thread with the highest priority.
 *
 * This function returns the thread at the front of the ready list
 * corresponding to the current top priority level.
 *
 * @return Pointer to the RTOS_thread_t structure of the current ready thread.
 *
 */
RTOS_thread_t * RTOS_threadGetCurrentReady(void)
{
	return readyList[currentTopPriority].pIndex->pThread;

}

/**
 * @brief Switches the currently running thread to the next ready thread.
 *
 * This function updates the currently running thread by finding the highest-priority
 * thread that is ready to run. It cycles through the ready list of threads to
 * determine the next thread to execute.
 *
 */
void RTOS_threadSwitchRunning(void)
{
	/* Find highest priority ready thread */
	while(readyList[currentTopPriority].numOfItems == 0)
	{
		ASSERT(THREAD_PRIORITY_LEVELS > currentTopPriority);
		currentTopPriority++;
	}

	/* Threads are found, update list index to the next thread */
	RTOS_list_t * pReadyList = &readyList[currentTopPriority];
	pReadyList->pIndex = pReadyList->pIndex->pNext;

	/* Check if the new index pointing to the end of the list */
	if(pReadyList->pIndex == (RTOS_listItem_t *) &pReadyList->listEnd)
	{
		/* Get the next thread */
		pReadyList->pIndex = pReadyList->pIndex->pNext;
	}else
	{
		/* Do nothing, index is not pointing to the end */
	}

	/* Update current running thread */
	pRunningThread = (RTOS_thread_t *) pReadyList->pIndex->pThread;
	runningThreadID = pRunningThread->threadID;
}

/**
 * @brief Retrieves the currently running thread.
 *
 * This function returns a pointer to the thread structure representing
 * the currently running thread in the RTOS.
 *
 * @return A pointer to the `RTOS_thread_t` structure of the running thread.
 *
 */
RTOS_thread_t * RTOS_threadGetRunning(void)
{
	return pRunningThread;
}

/**
 * @brief Adds a thread to the ready list and manages priority-based scheduling.
 *
 * This function inserts a thread into the ready list corresponding to its priority level.
 * It updates the system's top priority if the added thread has a higher priority
 * (lower numerical value). If the added thread's priority is higher than the currently
 * running thread, a context switch is triggered to ensure the highest-priority thread
 * executes.
 *
 * @param[in,out] pThread Pointer to the thread to be added to the ready list.
 *                        Must not be NULL.
 *
 */
void RTOS_threadAddToReadyList(RTOS_thread_t * pThread)
{
	ASSERT(pThread != NULL);
	RTOS_listInsertEnd(&readyList[pThread->priority], &pThread->listItem);
	if(pThread->priority < currentTopPriority)
	{
		currentTopPriority = pThread->priority;
	}

	if((pThread != NULL) && (pThread->priority < pRunningThread->priority))
	{
		/* Trigger context switching */
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}else
	{
		/* Context switching is not required */
	}

}

/**
 * @brief Updates the timer list and moves threads ready for execution to the ready list.
 *
 * This function increments the system tick counter and processes the timer list to refresh
 * the state of threads whose delay timers have expired. Threads that are ready to run
 * are moved to the ready list, enabling them to be scheduled by the RTOS.
 *
 */
void RTOS_threadRefreshTimerList(void)
{
	RTOS_thread_t * pThread;
	if(RTOS_isSchedulerRunning())
	{
		sysTickCounter++;
		if(sysTickCounter == 0)
		{
			ASSERT(0);
		}
		if(timerList.numOfItems > 0)
		{
			while(sysTickCounter >= timerList.listEnd.pNext->itemValue)
			{
				pThread = timerList.listEnd.pNext->pThread;
				ASSERT(pThread != NULL);
				RTOS_listRemove(&pThread->listItem);
				if(pThread->eventListItem.pList != NULL)
				{
					RTOS_listRemove(&pThread->eventListItem);
				}
				RTOS_threadAddToReadyList(pThread);
			}
		}
	}
}

/**
 * @brief Moves the currently running thread to the timer list for delayed execution.
 *
 * This function adds the currently running thread to the timer list after calculating
 * the tick value at which the thread should be woken up. It ensures the thread is removed
 * from its current list and inserted into the timer list with the appropriate wake-up time.
 *
 * @param waitTime The delay in ticks before the thread is scheduled to wake up.
 *
 */
void RTOS_threadAddRunningToTimerList(uint32_t waitTime)
{
	ASSERT(waitTime != 0);
	uint32_t wakeUpTick = 0;
	wakeUpTick = sysTickCounter + waitTime;
	if(sysTickCounter > wakeUpTick)
	{
		ASSERT(0);
	}
	pRunningThread->listItem.itemValue = wakeUpTick;
	RTOS_listRemove(&pRunningThread->listItem);
	RTOS_listInsert(&timerList, &pRunningThread->listItem);
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Destroys a thread and removes it from all associated lists.
 *
 * This function performs the necessary cleanup for a thread by removing it
 * from any system lists it is part of (ready list, event list, etc.).
 * If the thread being destroyed is currently running, it triggers a context
 * switch to ensure proper RTOS behavior.
 *
 * @param pThread Pointer to the thread object to be destroyed. Must not be NULL.
 *
 */
void RTOS_threadDestroy(RTOS_thread_t * pThread)
{
	ASSERT(pThread != NULL);
	if(pThread->listItem.pList != NULL)
	{
		RTOS_listRemove(&pThread->listItem);
	}

	if(pThread->eventListItem.pList != NULL)
	{
		RTOS_listRemove(&pThread->eventListItem);
	}

	if(pThread == pRunningThread)
	{
		/* Trigger context switching */
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}
}
