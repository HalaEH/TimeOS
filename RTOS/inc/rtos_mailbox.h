/*
 * rtos_mailbox.h
 *
 *  Created on: Dec 29, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_MAILBOX_H_
#define INC_RTOS_MAILBOX_H_

typedef struct
{
  int8_t * pStart;
  int8_t * pEnd;
  int8_t * pReadIndex;
  int8_t * pWriteIndex;
  uint32_t bufferLength;
  uint32_t messageSize;
  uint32_t messagesNum;
  RTOS_list_t waitingList;
} RTOS_mailbox_t;

void RTOS_mailboxCreate(RTOS_mailbox_t * pMailbox, void * pBuffer,
    uint32_t bufferLength, uint32_t messageSize);
void RTOS_SVC_mailboxCreate(RTOS_mailbox_t * pMailbox, void * pBuffer,
    uint32_t bufferLength, uint32_t messageSize);

uint32_t RTOS_mailboxWrite(RTOS_mailbox_t * pMailbox, int32_t waitTime,
    const void * const pMessage);
uint32_t RTOS_SVC_mailboxWrite(RTOS_mailbox_t * pMailbox, int32_t waitTime,
    const void * const pMessage);

uint32_t RTOS_mailboxRead(RTOS_mailbox_t * pMailbox, int32_t waitTime,
    void * const pMessage);
uint32_t RTOS_SVC_mailboxRead(RTOS_mailbox_t * pMailbox, int32_t waitTime,
    void * const pMessage);



#endif /* INC_RTOS_MAILBOX_H_ */
