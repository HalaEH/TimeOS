/**
 * @file
 * @brief Defines the assembly functions for triggering RTOS Supervisor Calls (SVC).
 *
 * This assembly code provides the implementations of two RTOS SVC functions:
 * one for starting the scheduler and another for creating a thread.
 *
 */

.syntax unified
.cpu cortex-m4
.thumb

.global RTOS_SVC_schedulerStart
.global RTOS_SVC_threadCreate
.global RTOS_SVC_mutexCreate
.global RTOS_SVC_mutexLock
.global RTOS_SVC_mutexRelease

.text

.type RTOS_SVC_schedulerStart, %function
RTOS_SVC_schedulerStart:
    svc 0
    bx lr

.type RTOS_SVC_threadCreate, %function
RTOS_SVC_threadCreate:
    svc 1
    bx lr

.type RTOS_SVC_mutexCreate, %function
RTOS_SVC_mutexCreate:
	svc 2
	bx lr

.type RTOS_SVC_mutexLock, %function
RTOS_SVC_mutexLock:
	svc 3
	bx lr

.type RTOS_SVC_mutexRelease, %function
	svc 4
	bx lr
