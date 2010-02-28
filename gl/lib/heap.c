/*
 * This file is part of the Generic Data Structures Library (GDSL).
 * Copyright (C) 1998-2006 Nicolas Darnis <ndarnis@free.fr>.
 *
 * The GDSL library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * The GDSL library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the GDSL library; see the file COPYING.
 * If not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * $RCSfile: gdsl_heap.c,v $
 * $Revision: 1.13 $
 * $Date: 2006/06/21 14:47:24 $
 */


#include <assert.h>
#include "heap.h"


/* Internal methods prototypes */
static int heap_default_compare (const void *, const void *);
static size_t heapify_down (void **, size_t, size_t,
                            int (*)(const void *, const void *));
static void heapify_up (void **, size_t,
                        int (*)(const void *, const void *));


/* Implementation (alphabetical order) */

struct heap *
heap_alloc (int (*compare)(const void *, const void *))
{
  struct heap *heap;

  heap = (struct heap *) malloc (sizeof heap);

  if (!heap)
    return NULL;

  heap->array = (void **) malloc (sizeof *(heap->array));
  if (!heap->array)
    {
      free (heap);
      return NULL;
    }

  heap->array[0] = NULL;
  heap->count = 0;
  heap->compare = compare ? compare : heap_default_compare;

  return heap;
}


static int
heap_default_compare (const void *a, const void *b)
{
  return 0;
}


void
heap_free (struct heap *heap)
{
    assert (heap != NULL);
    free (heap);
}


int
heap_insert (struct heap *heap, void *item)
{
  assert (heap != NULL);

  size_t new_size = (2 + heap->count) * sizeof (void *);
  heap->array = (void **) realloc (heap->array, new_size);

  if (!heap->array)
    return -1;

  heap->array[++heap->count] = item;
  heapify_up (heap->array, heap->count, heap->compare);

  return 0;
}


void *
heap_remove_top (struct heap *heap)
{
    assert (heap);

    if (heap->count == 0)
      return NULL;

    void *top = heap->array[1];
    heap->array[1] = heap->array[heap->count--];
    heapify_down (heap->array, heap->count, 1, heap->compare);

    return top;
}


static size_t
heapify_down (void **array, size_t count, size_t initial,
              int (*compare)(const void *, const void *))
{
    void *element = array[initial];

    size_t parent = initial;
    while (parent <= count/2)
      {
        size_t child = 2 * parent;

        if (child < count && compare (array[child], array[child+1]) < 0)
          child++;

        if (compare (array[child], element) <= 0)
          break;

        array[parent] = array[child];
        parent = child;
      }

    array[parent] = element;
    return parent;
}


static void
heapify_up (void **array, size_t count,
            int (*compare)(const void *, const void *))
{
  size_t k = count;
  void *new_element = array[k];

  /* To avoid k != 1 test below, we could
   * set *(array[0]) = MAX_POSSIBLE_VALUE, instead of
   * having array[0] == NULL. But that's impractical.
   */
  while (k != 1 && compare (array[k/2], new_element) <= 0)
    {
      array[k] = array[k/2];
      k /= 2;
    }

  array[k] = new_element;
}
