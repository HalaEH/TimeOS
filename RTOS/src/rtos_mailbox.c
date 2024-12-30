/*
 * rtos_mailbox.c
 *
 *  Created on: Dec 29, 2024
 *      Author: halaabdelrahman
 */

#include "rtos.h"

static void checkWaitingThreads(RTOS_mailbox_t * pMailbox);
static void blockCurrentThread(RTOS_mailbox_t * pMailbox);

/**
 * @brief Checks and wakes up threads waiting on a mailbox.
 *
 * This static function verifies if there are any threads in the mailbox's waiting list.
 * If one or more threads are waiting, the highest-priority thread is removed from the
 * waiting list and added to the ready list for execution.
 *
 * @param[in,out] pMailbox Pointer to the mailbox object to check. Must not be NULL.
 *
 */
static void checkWaitingThreads(RTOS_mailbox_t * pMailbox)
{
	ASSERT(NULL != pMailbox);
	RTOS_thread_t * pThread;

	/* Check if threads are waiting for the data */
	if(0 < pMailbox->waitingList.numOfItems)
	{
		pThread = pMailbox->waitingList.listEnd.pNext->pThread;
		ASSERT(NULL != pThread);
		RTOS_listRemove(&pThread->eventListItem);
		if(pThread->listItem.pList != NULL)
		{
			RTOS_listRemove(&pThread->listItem);
		}
		RTOS_threadAddToReadyList(pThread);
	}
	else
	{
		/* List is empty */
	}

}


/**
 * @brief Blocks the currently running thread on a mailbox's waiting list.
 *
 * This static function moves the currently running thread to the waiting list
 * of the specified mailbox, effectively blocking it until it is signaled or awakened.
 * A context switch is triggered to allow other threads to execute.
 *
 * @param[in,out] pMailbox Pointer to the mailbox object whose waiting list
 *                         will hold the current thread. Must not be NULL.
 *
 */
static void blockCurrentThread(RTOS_mailbox_t * pMailbox)
{
	RTOS_thread_t * pRunningThread;
	pRunningThread = RTOS_threadGetRunning();
	RTOS_listRemove(&pRunningThread->listItem);
	RTOS_listInsert(&pMailbox->waitingList, &pRunningThread->eventListItem);
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}


/**
 * @brief Initializes a mailbox.
 *
 * This function sets up a mailbox object for managing a circular buffer used
 * to pass messages between threads. It configures the buffer, message size,
 * and initializes the mailbox's state.
 *
 * @param[out] pMailbox     Pointer to the mailbox object to be initialized. Must not be NULL.
 * @param[in]  pBuffer      Pointer to the memory buffer used for storing messages. Must not be NULL.
 * @param[in]  bufferLength Length of the buffer in bytes. Must be non-zero.
 * @param[in]  messageSize  Size of each message in bytes. Must be 1, 2, or 4.
 *
 */
void RTOS_mailboxCreate(RTOS_mailbox_t * pMailbox, void * pBuffer, uint32_t bufferLength, uint32_t messageSize)
{
	/* Check input parameters */
	ASSERT(pMailbox != NULL);
	ASSERT(pBuffer != NULL);
	ASSERT(bufferLength != 0);
	ASSERT((messageSize == 1) || (messageSize == 2) || (messageSize == 4));

	/* Set mailbox buffer start and end */
	pMailbox->pStart = (int8_t *) pBuffer;
	pMailbox->pEnd = pMailbox->pStart + messageSize;

	/* Initialize read and write indices */
	pMailbox->pReadIndex = pMailbox->pStart;
	pMailbox->pWriteIndex = pMailbox->pStart;

	/* Set buffer length and message size */
	pMailbox->messageSize = messageSize;
	pMailbox->bufferLength = bufferLength;

	pMailbox->messageSize = 0;
	RTOS_listInit(&pMailbox->waitingList);
}


/**
 * @brief Writes a message to a mailbox, with optional blocking if the buffer is full.
 *
 * This function attempts to write a message to the specified mailbox. If the mailbox
 * buffer has available space, the message is written, and waiting threads (if any) are notified.
 * If the buffer is full, the behavior depends on the `waitFlag`:
 * - If `waitFlag` is 1, the calling thread is blocked until space becomes available.
 * - If `waitFlag` is 0, the function returns immediately without writing the message.
 *
 * @param[in,out] pMailbox  Pointer to the mailbox object to write to. Must not be NULL.
 * @param[in]     waitFlag  Determines blocking behavior:
 *                          0 - Do not block if the buffer is full.
 *                          1 - Block until space is available.
 * @param[in]     pMessage  Pointer to the message to be written. Must not be NULL.
 *
 * @return Status of the write operation:
 *         1 - Message successfully written to the mailbox.
 *         2 - Buffer is full, and the calling thread was blocked (only if `waitFlag` is 1).
 *         0 - Message not written, and no blocking occurred (only if `waitFlag` is 0).
 *
 */
uint32_t RTOS_mailboxWrite(RTOS_mailbox_t * pMailbox, int32_t waitTime, const void * const pMessage)
{
	/* Check input parameters */
	ASSERT(pMailbox != NULL);
	ASSERT(waitTime >= WAIT_INDEFINITELY);
	ASSERT(pMessage != NULL);

	RTOS_return_t returnStatus = RTOS_FAILURE;

	/* Check if there is a free place to write */
	if(pMailbox->bufferLength > pMailbox->messagesNum)
	{
		/* Buffer is not full, copy data */
		memcpy((void *) pMailbox->pWriteIndex, pMessage, pMailbox->messageSize);
		pMailbox->pWriteIndex += pMailbox->messageSize;

		/* Check if the buffer is written until the end */
		if(pMailbox->pWriteIndex >= pMailbox->pEnd)
		{
			pMailbox->pWriteIndex = pMailbox->pStart;
		}
		else
		{
			/* Do nothing, end is still not reached */
		}
		pMailbox->messagesNum++;
		checkWaitingThreads(pMailbox);
		returnStatus = RTOS_SUCCESS;
	}
	else
	{
		/* Do nothing, Buffer is full */
	}

	if((waitTime != NO_WAIT) && (returnStatus != RTOS_SUCCESS))
	{
		blockCurrentThread(pMailbox);
		if(waitTime > NO_WAIT)
		{
			RTOS_threadAddRunningToTimerList(waitTime);
		}
		returnStatus = RTOS_CONTEXT_SWITCH_TRIGGERED;
	}
	else
	{
		/* No blocking required, do nothing */
	}
	return returnStatus;
}


/**
 * @brief Reads a message from a mailbox, with optional blocking if the buffer is empty.
 *
 * This function attempts to read a message from the specified mailbox. If the mailbox
 * buffer contains messages, the oldest message is read and removed from the buffer.
 * If the buffer is empty, the behavior depends on the `waitFlag`:
 * - If `waitFlag` is 1, the calling thread is blocked until a message becomes available.
 * - If `waitFlag` is 0, the function returns immediately without reading a message.
 *
 * @param[in,out] pMailbox  Pointer to the mailbox object to read from. Must not be NULL.
 * @param[in]     waitFlag  Determines blocking behavior:
 *                          0 - Do not block if the buffer is empty.
 *                          1 - Block until a message becomes available.
 * @param[out]    pMessage  Pointer to the buffer where the message will be stored. Must not be NULL.
 *
 * @return Status of the read operation:
 *         1 - Message successfully read from the mailbox.
 *         2 - Buffer is empty, and the calling thread was blocked (only if `waitFlag` is 1).
 *         0 - No message was read, and no blocking occurred (only if `waitFlag` is 0).
 *
 */
uint32_t RTOS_mailboxRead(RTOS_mailbox_t * pMailbox, int32_t waitTime, void * const pMessage)
{
	/* Check input parameters */
	ASSERT(pMailbox != NULL);
	ASSERT(waitTime >= WAIT_INDEFINITELY);
	ASSERT(pMessage != NULL);

	RTOS_return_t returnStatus = RTOS_FAILURE;

	if(pMailbox->messagesNum > 0)
	{
		memcpy((void *) pMailbox->pReadIndex, pMessage, pMailbox->messageSize);
		pMailbox->pReadIndex += pMailbox->messageSize;
		/* Check if the buffer is read until the end */
		if(pMailbox->pReadIndex >= pMailbox->pEnd)
		{
			pMailbox->pReadIndex = pMailbox->pStart;
		}
		else
		{
			/* Do nothing, end is still not reached */
		}
		pMailbox->messagesNum--;
		checkWaitingThreads(pMailbox);
		returnStatus = RTOS_SUCCESS;
	}
	else
	{
		/* Do nothing, Buffer is full */
	}

	if((waitTime != NO_WAIT) && (returnStatus != RTOS_SUCCESS))
	{
		blockCurrentThread(pMailbox);
		if(waitTime > NO_WAIT)
		{
			RTOS_threadAddRunningToTimerList(waitTime);
		}
		returnStatus = RTOS_CONTEXT_SWITCH_TRIGGERED;
	}
	else
	{
		/* No blocking required, do nothing */
	}
	return returnStatus;
}
