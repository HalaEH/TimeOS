# TimeOS
# Real-Time Operating System

Welcome to the Real-Time Operating System (RTOS) implementation. This project is built using Embedded C and aims to provide a lightweight, efficient, and customizable RTOS for embedded systems.

## Table of Contents
- [Project Overview](#project-overview)
- [Features](#features)
- [Getting Started](#getting-started)
- [Future Improvements](#future-improvements)
- [Resources](#resources)

---

## Project Overview
In this project, I managed to implement a real-time preemptive operating system with alot of functionality for the ARM Cortex CM4F, specifically for the STM32F429 Discovery board. This RTOS is designed to manage tasks, resources, and timing efficiently. It supports basic real-time features, task scheduling, and inter-task communication, making it suitable for educational purposes, prototyping, and lightweight real-time applications.

## Features
- **Preemptive Scheduling**: Supports time-slicing and priority-based task switching.
- **Task Management**: Create, delete, and manage multiple tasks.
- **Timer Support**: Includes functionality for delayed task execution and periodic tasks.
- **Inter-task Communication**: Provides basic mechanisms for synchronization (e.g., semaphores, mutex).
- **Hardware Integration**: Built specifically for the STM32F429ZI, utilizing its peripherals and Cortex-M4 core features.
- **Error Handling**: Implements assertions and debugging tools for system stability.

## Getting Started

### Scheduler:
 
The RTOS scheduler determines which thread should execute next. It follows a Highest Priority Next algorithm, and if multiple threads share the same priority, it uses a Round-Robin scheduling approach to manage them.

**Implemented Methods**
```bash
void RTOS_SVC_schedulerStart(void);
```

### Threads:

A real-time application using an RTOS is composed of independent threads, each running in its own context without dependencies on other threads or the scheduler. Only one thread runs at a time, and the RTOS scheduler determines which thread to execute. The scheduler swaps threads in and out as the application runs. Since threads are unaware of the scheduler, it's the scheduler's responsibility to preserve and restore the thread's context (e.g., register values, stack) when swapping. This is achieved by giving each thread its own stack, saving the context when swapped out and restoring it when swapped back in.
	 
**Implemented Methods**
```bash
void RTOS_SVC_threadCreate(RTOS_thread_t * pThread, RTOS_stack_t * pStack, uint32_t priority, void * pFunction);
```

### Mutex:
A mutex is a synchronization mechanism that ensures only one thread or task can access a shared resource at a time. It prevents race conditions by locking the resource during access and unlocking it when done.

**Implemented Methods**
```bash
void RTOS_SVC_mutexCreate(RTOS_mutex_t * pMutex, uint32_t initialValue);
```

```bash
`uint32_t RTOS_SVC_mutexLock(RTOS_mutex_t * pMutex, int32_t waitTime);
```

```bash
void RTOS_SVC_mutexRelease(RTOS_mutex_t * pMutex);
```

### Semaphore:
A semaphore, is a protected integer variable that can facilitate and restrict access to shared resources in a multi-processing environment. It uses a counter to manage the number of tasks allowed to access a resource concurrently.
**Implemented Methods**
```bash
void RTOS_SVC_semaphoreCreate(RTOS_semaphore_t * pSemaphore, uint32_t initialValue);
```

```bash
uint32_t RTOS_SVC_semaphoreTake(RTOS_semaphore_t * pSemaphore, int32_t waitTime);
```

```bash
void RTOS_SVC_semaphoreGive(RTOS_semaphore_t * pSemaphore);
```

### Mailbox:
A mailbox in an RTOS is a message-passing mechanism that allows tasks to send and receive messages. It provides a safe and synchronized way for tasks to communicate and share data. It is implemented using a shared buffer, where one thread fills the buffer (producer) and another empties it (consumer).

**Implemented Methods**
```bash
void RTOS_SVC_mailboxCreate(RTOS_mailbox_t * pMailbox, void * pBuffer, uint32_t bufferLength, uint32_t messageSize);
```

```bash
uint32_t RTOS_SVC_mailboxWrite(RTOS_mailbox_t * pMailbox, int32_t waitTime, const void * const pMessage);
```

```bash
uint32_t RTOS_SVC_mailboxRead(RTOS_mailbox_t * pMailbox, int32_t waitTime, void * const pMessage);
```


### Prerequisites
- **Hardware**: STM32F429ZI Discovery Board
- **Software Tools**:
  - [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) or an ARM GCC toolchain.
  - A debugger such as ST-Link.

### Cloning the Repository
Clone this repository to your local machine using:
```bash
git clone https://github.com/HalaEH/TimeOS.git
```

## Future Improvements
- Implement Priority Inheritance mechanism.

## Resources
Operating Systems, Internals and Design Principles, Ninth Edition (Global Edition), By William Stallings.
ARM Cortex-M4 Course, By Ahmed Eldeep.