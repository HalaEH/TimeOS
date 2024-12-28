/*
 * rtos_semaphore.h
 *
 *  Created on: Dec 28, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_SEMAPHORE_H_
#define INC_RTOS_SEMAPHORE_H_

typedef struct
{
	uint32_t semaphoreValue;
	RTOS_list_t waitingList;
}RTOS_semaphore_t;

void RTOS_semaphoreCreate(RTOS_semaphore_t * pSemaphore, uint32_t initialValue);
void RTOS_SVC_semaphoreCreate(RTOS_semaphore_t * pSemaphore, uint32_t initialValue);

uint32_t RTOS_semaphoreTake(RTOS_semaphore_t * pSemaphore, uint32_t waitFlag);
uint32_t RTOS_SVC_semaphoreTake(RTOS_semaphore_t * pSemaphore, uint32_t waitFlag);

void RTOS_semaphoreGive(RTOS_semaphore_t * pSemaphore);
void RTOS_SVC_semaphoreGive(RTOS_semaphore_t * pSemaphore);

#endif /* INC_RTOS_SEMAPHORE_H_ */
