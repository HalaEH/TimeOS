/*
 * rtos.h
 *
 *  Created on: Dec 25, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_H_
#define INC_RTOS_H_

#include <stddef.h>
#include "stm32f4xx.h"
#include "rtos_config.h"
#include "rtos_list.h"
#include "rtos_thread.h"

#define MEM32_ADDRESS(ADDRESS) (*((volatile unsigned long *)(ADDRESS)))


#endif /* INC_RTOS_H_ */
