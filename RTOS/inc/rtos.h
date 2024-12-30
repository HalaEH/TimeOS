/*
 * rtos.h
 *
 *  Created on: Dec 25, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_H_
#define INC_RTOS_H_

#include <stddef.h>
#include <string.h>
#include "stm32f4xx.h"
#include "rtos_config.h"
#include "rtos_list.h"
#include "rtos_thread.h"
#include "rtos_mutex.h"
#include "rtos_semaphore.h"
#include "rtos_mailbox.h"

typedef enum
{
  RTOS_SUCCESS,
  RTOS_FAILURE,
  RTOS_CONTEXT_SWITCH_TRIGGERED
} RTOS_return_t;


#define NO_WAIT				((int32_t) 0)
#define WAIT_INDEFINITELY	((int32_t) -1)


#define MEM32_ADDRESS(ADDRESS) (*((volatile unsigned long *)(ADDRESS)))

void RTOS_init(void);
void RTOS_schedulerStart(void);
void RTOS_SVC_Handler_main(uint32_t * svc_args);
void RTOS_SysTick_Handler(void);
void RTOS_SVC_schedulerStart(void);
uint32_t RTOS_isSchedulerRunning(void);

#endif /* INC_RTOS_H_ */
