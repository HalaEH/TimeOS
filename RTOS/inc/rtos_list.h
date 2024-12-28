/*
 * rtos_list.h
 *
 *  Created on: Dec 24, 2024
 *      Author: halaabdelrahman
 */

#ifndef INC_RTOS_LIST_H_
#define INC_RTOS_LIST_H_


struct listItem_t
{
	struct listItem_t * pNext;		/* Pointer to the next list item */
	struct listItem_t * pPrev;		/* Pointer to the previous list item */
	void * pThread;					/* Pointer to the thread of the item */
	void * pList;					/* Pointer to the list in which the item is */
	uint32_t itemValue;
};

typedef struct listItem_t RTOS_listItem_t;

typedef struct
{
	RTOS_listItem_t * pNext;		/* Pointer to the next list item */
	RTOS_listItem_t * pPrev;		/* Pointer to the previous list item */
} RTOS_listEnd_t;


typedef struct
{
	uint32_t numOfItems;			/* Number of threads items in the list */
	RTOS_listItem_t * pIndex;		/* Pointer to the current item */
	RTOS_listEnd_t listEnd;			/* List end*/
} RTOS_list_t;


void RTOS_listInit(RTOS_list_t * pList);
void RTOS_listInsertEnd(RTOS_list_t * pList, RTOS_listItem_t * pItem);
void RTOS_listRemove(RTOS_listItem_t * pItem);
void RTOS_listInsert(RTOS_list_t * pList, RTOS_listItem_t * pItem);

#endif /* INC_RTOS_LIST_H_ */
