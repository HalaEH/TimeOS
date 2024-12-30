/*
 * mutex.h
 *
 *  Created on: Dec 28, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_MUTEX_H_
#define INC_RTOS_MUTEX_H_

typedef struct
{
	uint32_t mutexValue;
	RTOS_list_t waitingList;
} RTOS_mutex_t;

void RTOS_mutexCreate(RTOS_mutex_t * pMutex, uint32_t initialValue);
void RTOS_SVC_mutexCreate(RTOS_mutex_t * pMutex, uint32_t initialValue);


uint32_t RTOS_mutexLock(RTOS_mutex_t * pMutex, int32_t waitTime);
uint32_t RTOS_SVC_mutexLock(RTOS_mutex_t * pMutex, int32_t waitTime);


void RTOS_mutexRelease(RTOS_mutex_t * pMutex);
void RTOS_SVC_mutexRelease(RTOS_mutex_t * pMutex);


#endif /* INC_RTOS_MUTEX_H_ */
