/*
 * rtos_list.c
 *
 *  Created on: Dec 24, 2024
 *      Author: halaabdelrahman
 */
#include "rtos.h"

/**
 * @brief Initializes an RTOS list structure.
 *
 * This function sets up the list for use by initializing its pointers and
 * counters. It ensures the list is empty and ready to accept items.
 *
 * @param pList Pointer to the RTOS_list_t structure to be initialized.
 *
 */
void RTOS_listInit(RTOS_list_t * pList)
{
	/* Initially no threads in the list, index is pointing to end */
	pList->pIndex = (RTOS_listItem_t *) &pList->listEnd;
	pList->listEnd.pNext = (RTOS_listItem_t *) &pList->listEnd;
	pList->listEnd.pPrev = (RTOS_listItem_t *) &pList->listEnd;
	pList->numOfItems = 0;
}

/**
 * @brief Inserts a list item at the end of the RTOS list.
 *
 * This function adds a new item to the end of the specified RTOS list,
 * updating the necessary pointers to maintain the doubly-linked list structure.
 *
 * @param pList Pointer to the RTOS_list_t structure where the item will be inserted.
 * @param pItem Pointer to the RTOS_listItem_t structure to be inserted into the list.
 *
 */
void RTOS_listInsertEnd(RTOS_list_t * pList, RTOS_listItem_t * pItem)
{
	pItem->pNext = pList->pIndex->pNext;
	pItem->pPrev = pList->pIndex;
	pList->pIndex->pNext->pPrev = pItem;
	pList->pIndex->pNext = pItem;
	pList->pIndex = pItem;
	pItem->pList = (void *) pList;
	pList->numOfItems++;
}

/**
 * @brief Removes a list item from the RTOS list.
 *
 * This function detaches a specified item from its parent RTOS list,
 * updating the necessary pointers to maintain the integrity of the
 * doubly-linked list structure.
 *
 * @param pItem Pointer to the RTOS_listItem_t structure to be removed.
 *
 */
void RTOS_listRemove(RTOS_listItem_t * pItem)
{
	pItem->pPrev->pNext = pItem->pNext;
	pItem->pNext->pPrev = pItem->pPrev;
	RTOS_list_t * pList = (RTOS_list_t *) pItem->pList;
	if(pList->pIndex == pItem)
	{
		pList->pIndex = pItem->pPrev;
	}
	/* Remove item from the list*/
	pItem->pList = NULL;
	pList->numOfItems--;
}

/**
 * @brief Inserts a list item in RTOS list based on the priority.
 *
 * This function adds a new item to the specified RTOS list based on the priority order
 *
 * @param pList Pointer to the RTOS_list_t structure where the item will be inserted.
 * @param pItem Pointer to the RTOS_listItem_t structure to be inserted into the list.
 *
 */
void RTOS_listInsert(RTOS_list_t * pList, RTOS_listItem_t * pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	/* Temp for the insert index */
	RTOS_listItem_t * pInsertIndex = (RTOS_listItem_t *) &pList->listEnd;

	/* Get insert index, find the high item value */
	while((pInsertIndex->pNext != (RTOS_listItem_t *) &pList->listEnd) && (pInsertIndex->pNext->itemValue <= pItem->itemValue))
	{
		pInsertIndex = pInsertIndex->pNext;
	}

	/* Connect the new item with insert index */
	pItem->pNext = pInsertIndex->pNext;
	pItem->pPrev = pInsertIndex;
	pInsertIndex->pNext->pPrev = pItem;
	pInsertIndex->pNext = pItem;

	pItem->pList = (void *) pList;
	pList->numOfItems++;
}


