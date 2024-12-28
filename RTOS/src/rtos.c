/*
 * rtos.c
 *
 *  Created on: Dec 25, 2024
 *      Author: halaabdelrahman
 */


#include "rtos.h"

static volatile uint32_t sysTickCounter = 0;
static RTOS_thread_t idleThread;
static RTOS_stack_t idleThreadStack;
uint32_t svcEXEReturn;


static void idleThreadFunction(void);

static void idleThreadFunction(void)
{
	while(1)
	{

	}
}


/**
 * @brief Initializes the RTOS core and configures the system for multitasking.
 *
 * This function performs essential setup tasks, including configuring interrupts,
 * enabling SysTick, setting priority levels, and initializing the thread management module.
 *
 */
void RTOS_init(void)
{
	/* Disable interrupts */
	__disable_irq();

	/* Enable double word stack alignment */
	SCB->CCR |= SCB_CCR_STKALIGN_Msk;

	/* Configure and enable SysTick interrupts */
	ASSERT(0 == SysTick_Config(SYSTEM_CORE_CLOCK_HZ / SYS_TICK_RATE_HZ));

	/* Set priority group to 3
	 * bits[3:0] are the sub-priority
	 * bits[7:4] are the preempt priority (0-15)
	 */
	NVIC_SetPriorityGrouping(3);

	/* Set priority levels */
	NVIC_SetPriority(SVCall_IRQn, 0);
	NVIC_SetPriority(SysTick_IRQn, 1);

	/* Set PendSV to lowest possible priority */
	NVIC_SetPriority(PendSV_IRQn, 0xFF);

	/* Enable SVC and PendSV interrupts */
	NVIC_EnableIRQ(PendSV_IRQn);
	NVIC_EnableIRQ(SVCall_IRQn);

	/* Disable all interrupts except SVC */
	__set_BASEPRI(1);

	/* Initialize thread module */
	RTOS_threadInitLists();

	/* Enable interrupts */
	__enable_irq();

}

/**
 * @brief Starts the RTOS scheduler.
 *
 * This function initializes and starts the RTOS scheduler by creating the idle thread,
 * setting up the stack and control registers for the first running thread, and enabling interrupts.
 *
 */
void RTOS_schedulerStart(void)
{
	/* Create idle thread */
	RTOS_threadCreate(&idleThread, &idleThreadStack, (THREAD_PRIORITY_LEVELS - 1), idleThreadFunction);

	/* Pointer to the current running thread */
	RTOS_thread_t * pRunningThread;

	/* Update the running thread */
	RTOS_threadSwitchRunning();

	/* Get running thread */
	pRunningThread = RTOS_threadGetRunning();

	/* Set SVC interrupt return to the first thread */
	svcEXEReturn = MEM32_ADDRESS(pRunningThread->pStackPointer);

	/* Return to thread with PSP */
	__set_PSP((uint32_t)(pRunningThread->pStackPointer + 10 * 4));

	/* Switch to use Process Stack, unprivileged state */
	__set_CONTROL(MEM32_ADDRESS((pRunningThread->pStackPointer) + (1 << 2)));

	/* Execute ISB after changing control */
	__ISB();

	/* Reset tick counter */
	sysTickCounter = 0;

	/* Enable all interrupts */
	__set_BASEPRI(0);
}

/**
 * @brief Main handler for the RTOS Supervisor Call (SVC) interrupt.
 *
 * This function processes SVC calls made by the RTOS and executes the appropriate
 * service based on the SVC number embedded in the instruction. It decodes the
 * SVC number and handles the corresponding request.
 *
 * @param svc_args Pointer to the stacked arguments during the SVC exception.
 * - `svc_args[6]` holds the Program Counter (PC) value at the time of the exception.
 * - The SVC number is retrieved from the instruction immediately preceding the stacked PC.
 *
 */
void RTOS_SVC_Handler_main(uint32_t * svc_args)
{
	uint8_t svc_number;

	/* Memory[Stacked PC)-2] */
	svc_number = ((char *) svc_args[6])[-2];

	uint32_t returnStatus;

	/* Check SVC number */
	switch(svc_number)
	{
	case 0:
		RTOS_schedulerStart();
		break;

	case 1:
		RTOS_threadCreate(
				(RTOS_thread_t *) svc_args[0],
				(RTOS_stack_t *) svc_args[1],
				(uint32_t) svc_args[2],
				(void *) svc_args[3]);
		break;

	case 2:
		RTOS_mutexCreate((RTOS_mutex_t *) svc_args[0], (uint32_t) svc_args[1]);
	break;

	case 3:
		returnStatus = RTOS_mutexLock((RTOS_mutex_t *) svc_args[0], (uint32_t) svc_args[1]);
	break;

	case 4:
		RTOS_mutexRelease((RTOS_mutex_t *) svc_args[0]);
	break;

	default:
		/* Not supported SVC call */
		ASSERT(0);
		break;
	}

	switch(svc_number)
	{
		case 3:
			if(returnStatus == 2)
			{
				svc_args[6] = svc_args[6] - 2;
			}else
			{
				svc_args[0] = returnStatus;
			}
		break;

		default:
		break;
	}


}

/**
 * @brief RTOS SysTick interrupt handler.
 *
 * This function is called on every SysTick interrupt to handle periodic system tasks
 * and trigger a context switch if required.
 *
 */
void RTOS_SysTick_Handler(void)
{
	/* Trigger context switch, set PendSV to pending */
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;

	/* Increment SysTick counter */
	++sysTickCounter;

}
