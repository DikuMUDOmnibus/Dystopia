/***************************************************************************
 * Dystopia 2 © 2000, 2001, 2002, 2003, 2004 & 2005 by Brian Graversen     *
 *                                                                         *
 * In order to use any part of this code, you must comply with the license *
 * files found in the license folder in the doc folder of this codebase.   *
 *                                                                         *
 * Dystopia MUD is based of Godwars by Richard Woolcock.                   *
 *                                                                         *
 * Godwars is based of Merc by Michael Chastain, Michael Quan and Mitchell *
 * Tse.                                                                    *
 *                                                                         *
 * Merc is based of DIKU by Sebastian Hammer, Michael Seifert, Hans Henrik *
 * Staerfeldt. Tom Madsen and Katja Nyboe.                                 *
 *                                                                         *
 * Any additional licenses, copyrights, etc that affects this sourcefile   *
 * will be mentioned just below this copyright notice.                     *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dystopia.h"

/* local procedures */
void       FreeCell        ( CELL *pCell, LIST *pList );
CELL     * AllocCell       ( void );
void       InvalidateCell  ( CELL *pCell );
void       FreeIterator    ( ITERATOR *pIter );
ITERATOR *_AllocIterator   ( LIST *pList, int reverse );

STACK *scell_free = NULL;
ITERATOR  *iterator_list = NULL;
ITERATOR *iterator_free = NULL;

/***********************
 ****   LIST CODE   ****
 ***********************/

LIST *AllocList()
{
  LIST *pList;

  pList = malloc(sizeof(*pList));
  pList->_pFirstCell   = NULL;
  pList->_pLastCell    = NULL;
  pList->_iterators    = 0;
  pList->_invalidcells = 0;
  pList->_valid        = 1;
  pList->_size         = 0;

  return pList;
}

void ReleaseAllIterators()
{
  ITERATOR *pIter, *pIterNext;

  for (pIter = iterator_list; pIter != NULL; pIter = pIterNext)
  {
    pIterNext = pIter->_pNext;

    FreeIterator(pIter);
  }

  iterator_list = NULL;
}

ITERATOR *AllocReverseIterator(LIST *pList)
{
  return (_AllocIterator(pList, 1));
}

ITERATOR *AllocIterator(LIST *pList)
{
  return (_AllocIterator(pList, 0));
}

ITERATOR *AllocQuickIterator()
{
  ITERATOR *pIter;

  if (iterator_free == NULL)
    pIter = malloc(sizeof(*pIter));
  else
  {
    pIter = iterator_free;
    iterator_free = iterator_free->_pNext;
  }

  pIter->_pCell = NULL;
  pIter->_pList = NULL;
  pIter->_reverse = 0;
  pIter->_pNext = NULL;

  return pIter;
}

ITERATOR *_AllocIterator(LIST *pList, int reverse)
{
  ITERATOR *pIter;

  if (pList == NULL)
    abort();

  if (iterator_free == NULL)
    pIter = malloc(sizeof(*pIter));
  else
  {
    pIter = iterator_free;
    iterator_free = iterator_free->_pNext;
  }

  pIter->_pList = pList;
  pIter->_reverse = reverse;
  pList->_iterators++;
  pIter->_pCell = (reverse) ? pList->_pLastCell : pList->_pFirstCell;

  /* Attach to list of all iterators */
  pIter->_pNext = iterator_list;
  iterator_list = pIter;

  return pIter;
}

CELL *AllocCell()
{
  CELL *pCell;

  pCell = malloc(sizeof(*pCell));
  pCell->_pNextCell = NULL;
  pCell->_pPrevCell = NULL;
  pCell->_pContent = NULL;
  pCell->_valid = 1;

  return pCell;
}

/* pos = 0 -> Attach to front of list
 * pos = 1 -> Attach to end of list
 * pos = 2 -> Attach AFTER pItem
 * pos = 3 -> Attach BEFORE pItem
 */
void _AttachToList(void *pContent, LIST *pList, void *pItem, int pos)
{
  CELL *pCell, *pItemCell = NULL;
  int found = 0;

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pCell->_pNextCell)
  {
    if (!pCell->_valid)
      continue;

    if (pCell->_pContent == pContent)
      found = 1;

    if (pCell->_pContent == pItem)
      pItemCell = pCell;
  }

  if (found)
  {
    bug("_AttachToList: Attempting to add item already in list (pos=%d).", pos);
    return;
  }

  if (pos >= 2 && pItemCell == NULL)
  {
    bug("_AttachToList: Attempting to add before/after an item not in list.", 0);
    return;
  }

  pCell = AllocCell();
  pCell->_pContent = pContent;

  switch (pos)
  {
    default:
      bug("_AttachToList: Calling with pos=%d.", pos);
      break;
    case 0:
      if (pList->_pFirstCell != NULL)
        pList->_pFirstCell->_pPrevCell = pCell;
      if (pList->_pLastCell == NULL)
        pList->_pLastCell = pCell;
      pCell->_pNextCell = pList->_pFirstCell;
      pList->_pFirstCell = pCell;
      break;
    case 1:
      if (pList->_pLastCell != NULL)
        pList->_pLastCell->_pNextCell = pCell;
      if (pList->_pFirstCell == NULL)
        pList->_pFirstCell = pCell;
      pCell->_pPrevCell = pList->_pLastCell;
      pList->_pLastCell = pCell;
      break;
    case 2:
      pCell->_pNextCell = pItemCell->_pNextCell;
      pCell->_pPrevCell = pItemCell;
      if (pItemCell->_pNextCell)
        pItemCell->_pNextCell->_pPrevCell = pCell;
      pItemCell->_pNextCell = pCell;
      if (pList->_pLastCell == pItemCell)
        pList->_pLastCell = pCell;
      break;
    case 3:
      pCell->_pPrevCell = pItemCell->_pPrevCell;
      pCell->_pNextCell = pItemCell;
      if (pItemCell->_pPrevCell)
        pItemCell->_pPrevCell->_pNextCell = pCell;
      pItemCell->_pPrevCell = pCell;
      if (pList->_pFirstCell == pItemCell)
        pList->_pFirstCell = pCell;
      break;
  }

  pList->_size++;
}



void AttachToList(void *pContent, LIST *pList)
{
  _AttachToList(pContent, pList, NULL, 0);
}

void AttachToEndOfList(void *pContent, LIST *pList)
{
  _AttachToList(pContent, pList, NULL, 1);
}

void AttachToListAfterItem(void *pContent, LIST *pList, void *pItem)
{
  _AttachToList(pContent, pList, pItem, 2);
}

void AttachToListBeforeItem(void *pContent, LIST *pList, void *pItem)
{
  _AttachToList(pContent, pList, pItem, 3);
}

void DetachFromList(void *pContent, LIST *pList)
{
  CELL *pCell;

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pCell->_pNextCell)
  {
    if (pCell->_pContent == pContent)
    {
      if (pList->_iterators > 0)
      {
        pList->_invalidcells = 1;
        InvalidateCell(pCell);
      }
      else
        FreeCell(pCell, pList);

      pList->_size--;
      return;
    }
  }
}

void DetachAtIterator(ITERATOR *pIter)
{
  CELL *pCell;

  if (pIter->_pCell == NULL)
    pCell = (pIter->_reverse) ? pIter->_pList->_pFirstCell : pIter->_pList->_pLastCell;
  else
    pCell = (pIter->_reverse) ? pIter->_pCell->_pNextCell : pIter->_pCell->_pPrevCell;

  while (pCell->_valid == 0 &&
        (pCell = (pIter->_reverse) ? pCell->_pNextCell : pCell->_pPrevCell) != NULL)
    ;

  if (pCell != NULL)
  {
    InvalidateCell(pCell);
    pIter->_pList->_invalidcells = 1;
    pIter->_pList->_size--;
  }
  else
  {
    bug("DetachAtIterator: Iterator has not iterated yet.", 0);
  }
}

void FreeIterator(ITERATOR *pIter)
{
  LIST *pList = pIter->_pList;

  pList->_iterators--;

  /* if this is the only iterator, free all invalid cells */
  if ((pList->_invalidcells || !pList->_valid) && pList->_iterators <= 0)
  {
    CELL *pCell, *pNextCell;

    if (pList->_valid)
    {
      for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pNextCell)
      {
        pNextCell = pCell->_pNextCell;

        if (!pCell->_valid)
          FreeCell(pCell, pList);
      }
      pList->_invalidcells = 0;
    }
    else
      FreeList(pList);
  }

  /* attach to list of 'free' iterators */
  pIter->_pNext = iterator_free;
  iterator_free = pIter;
}

void FreeList(LIST *pList)
{
  CELL *pCell, *pNextCell;

  /* if we have any unfinished iterators, wait for later */
  if (pList->_iterators > 0)
  {
    pList->_valid = 0;
    return;
  }

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pNextCell)
  {
    pNextCell = pCell->_pNextCell;

    FreeCell(pCell, pList);
  }

  free(pList);
}

void FreeCell(CELL *pCell, LIST *pList)
{
  if (pList->_pFirstCell == pCell)
    pList->_pFirstCell = pCell->_pNextCell;

  if (pList->_pLastCell == pCell)
    pList->_pLastCell = pCell->_pPrevCell;

  if (pCell->_pPrevCell != NULL)
    pCell->_pPrevCell->_pNextCell = pCell->_pNextCell;

  if (pCell->_pNextCell != NULL) 
    pCell->_pNextCell->_pPrevCell = pCell->_pPrevCell;

  free(pCell);
}

void InvalidateCell(CELL *pCell)
{
  pCell->_valid = 0;
}

void *FirstInList(LIST *pList)
{
  CELL *pCell;

  for (pCell = pList->_pFirstCell; pCell; pCell = pCell->_pNextCell)
  {
    if (pCell->_valid)
      return pCell->_pContent;
  }

  return NULL;
}

void *LastInList(LIST *pList)
{
  CELL *pCell;

  for (pCell = pList->_pLastCell; pCell; pCell = pCell->_pPrevCell)
  {
    if (pCell->_valid)
      return pCell->_pContent;
  }

  return NULL;
}

void *PeekNextInList(ITERATOR *pIter)
{
  void *pContent = NULL;

  /* skip invalid cells */
  while (pIter->_pCell != NULL && !pIter->_pCell->_valid)
  {
    if (pIter->_reverse)
      pIter->_pCell = pIter->_pCell->_pPrevCell;
    else
      pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  if (pIter->_pCell != NULL)
    pContent = pIter->_pCell->_pContent;

  return pContent;
}

void *NextInList(ITERATOR *pIter)
{
  void *pContent = NULL;

  /* skip invalid cells */
  while (pIter->_pCell != NULL && !pIter->_pCell->_valid)
  {
    if (pIter->_reverse)
      pIter->_pCell = pIter->_pCell->_pPrevCell;
    else
      pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  if (pIter->_pCell != NULL)
  {
    pContent = pIter->_pCell->_pContent;

    if (pIter->_reverse)
      pIter->_pCell = pIter->_pCell->_pPrevCell;
    else
      pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  return pContent;
}

int SizeOfList(LIST *pList)
{
  return pList->_size;
}


/************************
 ****   STACK CODE   ****
 ************************/

STACK *AllocStack()
{
  STACK *pStack;

  pStack = malloc(sizeof(*pStack));
  pStack->_pCells = NULL;
  pStack->_iSize = 0;

  return pStack;
}

void FreeStack(STACK *pStack)
{
  SCELL *pCell;

  while ((pCell = pStack->_pCells) != NULL)
  {
    pStack->_pCells = pCell->_pNext;
    free(pCell);
  }

  free(pStack);
}

void *PopStack(STACK *pStack)
{
  SCELL *pCell;

  if ((pCell = pStack->_pCells) != NULL)
  {
    void *pContent = pCell->_pContent;

    pStack->_pCells = pCell->_pNext;
    pStack->_iSize--;

    pCell->_pNext = scell_free->_pCells;
    scell_free->_pCells = pCell;

    return pContent;
  }

  return NULL;
}

void PushStack(void *pContent, STACK *pStack)
{
  SCELL *pCell;

  if (scell_free->_pCells != NULL)
  {
    pCell = scell_free->_pCells;
    scell_free->_pCells = pCell->_pNext;
  }
  else
  {
    pCell = malloc(sizeof(*pCell));
  }

  pCell->_pNext = pStack->_pCells;
  pCell->_pContent = pContent;
  pStack->_pCells = pCell;
  pStack->_iSize++;
}

int StackSize(STACK *pStack)
{
  return pStack->_iSize;
}
