/*
 * rtos_list.c
 *
 *  Created on: Dec 24, 2024
 *      Author: halaabdelrahman
 */
#include "rtos_list.h"

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

