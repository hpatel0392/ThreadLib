/* list.c
 * Harsh Patel
 * Spring 2017
 *
 * Purpose: The implementation of a dually linked-list ADT
 *
 * Assumptions: This list can operate without knowing what a "data_t" is
 *              to manage the data_t a middle interface must be made for support
 *
 *
 */

#include <stdlib.h>
#include <assert.h>

#include "datatypes.h"   // defines data_t
#include "list.h"        // defines public functions for list ADT

// definitions for private constants used in list.c only

#define LIST_SORTED    999999
#define LIST_UNSORTED -888888

// prototypes for private functions used in list.c only


/* ----- below are the functions  ----- */


/* Obtains a pointer to an element stored in the specified list, at the
 * specified list position
 *
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from list_construct.
 *
 * pos_index: position of the element to be accessed.  Index starts at 0 at
 *            head of the list, and incremented by one until the tail is
 *            reached.  Can also specify LISTPOS_HEAD and LISTPOS_TAIL
 *
 * return value: pointer to element accessed within the specified list.  A
 * value NULL is returned if the pos_index does not correspond to an element in
 * the list.
 */
data_t * list_access(list_t *list_ptr, int pos_index)
{
    int count=0;
    list_node_t *L;

    // debugging function to verify that the structure of the list is valid
    //list_debug_validate(list_ptr);

    /* handle special cases.
     *   1.  The list is empty
     *   2.  Asking for the head
     *   3.  Asking for the tail
     *   4.  specifying a position that is out of range.  This is not defined
     *       to be an error in this function, but instead it is assumed the
     *       calling function correctly specifies the position index
     */
    if (list_entries(list_ptr) == 0) {
        return NULL;  // list is empty
    }
    else if (pos_index == LISTPOS_HEAD || pos_index == 0) {
        return list_ptr->ll_head->data_ptr;
    }
    else if (pos_index == LISTPOS_TAIL || pos_index == list_entries(list_ptr) - 1) {
        return list_ptr->ll_tail->data_ptr;
    }
    else if (pos_index < 0 || pos_index >= list_entries(list_ptr))
        return NULL;   // does not correspond to position in list

    // loop through the list until find correct position index
    L=list_ptr->ll_head;
    for (count = 0; count < pos_index; count++) {
        L = L->ll_next;
    }
    // already verified that pos_index should be valid so L better not be null
    assert(L != NULL);
    assert(L->data_ptr != NULL);
    return L->data_ptr;
}


/* Allocates a new, empty list
 *
 * By convention, the list is initially assumed to be sorted.  The field sorted
 * can only take values LIST_SORTED or LIST_UNSORTED
 *
 * Use list_destruct to remove and deallocate all elements on a list
 * and the header block.
 */
list_t * list_construct(int (*fcomp)(const data_t *, const data_t *))
{
    list_t *L;
    L = (list_t *) malloc(sizeof(list_t));
    L->ll_head = NULL;
    L->ll_tail = NULL;
    L->ll_entries = 0;
    L->ll_sorted_state = LIST_SORTED;
    L->comp_proc = fcomp;
    //list_debug_validate(L);
    return L;
}


/* Finds an element in a list and returns a pointer to it.
 *
 * list_ptr: pointer to list-of-interest.
 *
 * elem_ptr: element against which other elements in the list are compared.
 *           Note: use the comp_proc function pointer found in the list_t
 *           header block.
 *
 * NOTICE: pos_index is returned and is not an input value!
 *
 * The function returns a pointer to the matching element with lowest index if
 * a match if found.  If a match is not found the return value is NULL.
 *
 * The function also returns the integer position of matching element with the
 *           lowest index.  If a matching element is not found, the position
 *           index that is returned should be -1.
 *
 * pos_index: used as a return value for the position index of matching element
 *
 */
data_t * list_elem_find(const list_t *list_ptr, const data_t *elem_ptr, int *pos_index)
{
   //list_debug_validate(list_ptr);

   if(list_ptr->ll_entries != 0)
   {
      list_node_t* it = list_ptr->ll_head;
      for(*pos_index = 0; *pos_index < list_ptr->ll_entries; (*pos_index)++, it = it->ll_next){
         if(list_ptr->comp_proc(it->data_ptr, elem_ptr) == 1)
            return it->data_ptr;
      }
   }
   *pos_index = -1;
   return NULL;
   //list_debug_validate(list_ptr);
}


/* Deallocates the contents of the specified list, releasing associated memory
 * resources for other purposes.
 */
void list_destruct(list_t *list_ptr)
{
   // the first line must validate the list
   //list_debug_validate(list_ptr);

   // Your code starts here
   if(list_ptr->ll_entries != 0)
   {
      list_node_t* it = list_ptr->ll_head; // iterator
      while(it != list_ptr->ll_tail){
         list_ptr->ll_head = it->ll_next;
         free(it->data_ptr);
         free(it);
         it = list_ptr->ll_head;
      }
      free(it->data_ptr);
      free(it);
   }
   free(list_ptr);
}


/* Inserts the specified data element into the specified list at the specified
 * position.
 *
 * list_ptr: pointer to list-of-interest.
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * pos_index: numeric position index of the element to be inserted into the
 *            list.  Index starts at 0 at head of the list, and incremented by
 *            one until the tail is reached.  The index can also be equal
 *            to LISTPOS_HEAD or LISTPOS_TAIL (these are special negative
 *            values use to provide a short cut for adding to the head
 *            or tail of the list).
 *
 * If pos_index is greater than the number of elements currently in the list,
 * the element is simply appended to the end of the list (no additional elements
 * are inserted).
 *
 * Note that use of this function results in the list to be marked as unsorted,
 * even if the element has been inserted in the correct position.  That is, on
 * completion of this subroutine the list_ptr->ll_sorted_state must be equal
 * to LIST_UNSORTED.
 */
void list_insert(list_t *list_ptr, data_t *elem_ptr, int pos_index)
{
   assert(list_ptr != NULL);

   if(pos_index < 0 && pos_index != LISTPOS_HEAD && pos_index != LISTPOS_TAIL)
      return;

   list_node_t* newNode = (list_node_t*)calloc(1, sizeof(list_node_t));
   newNode->data_ptr = elem_ptr;
   if(pos_index == LISTPOS_HEAD || pos_index == 0)
   {
      newNode->ll_next = list_ptr->ll_head;
      if(list_ptr->ll_head != NULL)
         list_ptr->ll_head->ll_prev = newNode;
      list_ptr->ll_head = newNode;
   }
   else if(pos_index == LISTPOS_TAIL || pos_index >= list_ptr->ll_entries)
   {
      newNode->ll_prev = list_ptr->ll_tail;
      if(list_ptr->ll_tail != NULL)
      list_ptr->ll_tail->ll_next = newNode;
      list_ptr->ll_tail = newNode;
   }
   else
   {
      int i;
      list_node_t* it = list_ptr->ll_head;
      for(i = 0; i < pos_index-1; i++, it = it->ll_next){
      }
      newNode->ll_next = it->ll_next;
      newNode->ll_prev = it;
      it->ll_next = newNode;
      newNode->ll_next->ll_prev = newNode;
   }

   list_ptr->ll_entries++;
   if(list_ptr->ll_entries == 1)
   {
      if(list_ptr->ll_head == NULL)
         list_ptr->ll_head = list_ptr->ll_tail;
      else
         list_ptr->ll_tail = list_ptr->ll_head;
   }

   newNode = NULL;
   // the last three lines of this function must be the following
   if (list_ptr->ll_sorted_state == LIST_SORTED)
       list_ptr->ll_sorted_state = LIST_UNSORTED;
   //list_debug_validate(list_ptr);
}


/* Inserts the element into the specified sorted list at the proper position,
 * as defined by the comp_proc function pointer found in the header block.
 *
 * list_ptr: pointer to list-of-interest.
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * If you use list_insert_sorted, the list preserves its sorted nature.
 *
 * If you use list_insert, the list will be considered to be unsorted, even
 * if the element has been inserted in the correct position.
 *
 * If the list is not sorted and you call list_insert_sorted, this subroutine
 * should generate a system error and the program should immediately stop.
 *
 * The comparison procedure must accept two arguments (A and B) which are both
 * pointers to elements of type data_t.  The comparison procedure returns an
 * integer code which indicates the precedence relationship between the two
 * elements.  The integer code takes on the following values:
 *    1: A should be closer to the list head than B
 *   -1: B should be closer to the list head than A
 *    0: A and B are equal in rank
 *
 * Note: if the element to be inserted is equal in rank to an element already
 *       in the list, the newly inserted element will be placed after all the
 *       elements of equal rank that are already in the list.
 */
void list_insert_sorted(list_t *list_ptr, data_t *elem_ptr)
{
   assert(list_ptr != NULL);
   assert(list_ptr->ll_sorted_state == LIST_SORTED);

   list_node_t* newNode = (list_node_t*)calloc(1, sizeof(list_node_t));
   newNode->data_ptr = elem_ptr;

   if(list_ptr->ll_entries == 0)
   {
      list_ptr->ll_head = newNode;
      list_ptr->ll_tail = newNode;
      list_ptr->ll_entries++;
   }
   else
   {
      list_node_t* it = list_ptr->ll_head;
      for(; it != NULL; it = it->ll_next){
         if(list_ptr->comp_proc(elem_ptr, it->data_ptr) == 1)
            break;
      }

      if(it == NULL) // tail > elem (append)
      {
         newNode->ll_prev = list_ptr->ll_tail;
         list_ptr->ll_tail->ll_next = newNode;
         list_ptr->ll_tail = newNode;
      }
      else if(it->ll_prev == NULL) // elem < head (add to front)
      {
         newNode->ll_next = list_ptr->ll_head;
         list_ptr->ll_head->ll_prev = newNode;
         list_ptr->ll_head = newNode;
      }
      else
      {
         newNode->ll_next = it;
         newNode->ll_prev = it->ll_prev;
         it->ll_prev = newNode;
         newNode->ll_prev->ll_next = newNode;
      }


      list_ptr->ll_entries++;
      if(list_ptr->ll_entries == 1)
      {
         if(list_ptr->ll_head == NULL)
            list_ptr->ll_head = list_ptr->ll_tail;
         else
            list_ptr->ll_tail = list_ptr->ll_head;
      }
   }

   // the last line checks if the new list is correct
   //list_debug_validate(list_ptr);
}


/* Removes an element from the specified list, at the specified list position,
 * and returns a pointer to the element.
 *
 * list_ptr: pointer to list-of-interest.
 *
 * pos_index: position of the element to be removed.  Index starts at 0 at
 *            head of the list, and incremented by one until the tail is
 *            reached.  Can also specify LISTPOS_HEAD and LISTPOS_TAIL
 *
 * Attempting to remove an element at a position index that is not contained in
 * the list will result in no element being removed, and a NULL pointer will be
 * returned.
 */
data_t * list_remove(list_t *list_ptr, int pos_index)
{
   //list_debug_validate(list_ptr);

   if(pos_index != LISTPOS_HEAD && pos_index != LISTPOS_TAIL)
      if(pos_index < 0 || pos_index >= list_ptr->ll_entries)
         return NULL;

   list_node_t* it;
   data_t* ret;

   if(list_ptr->ll_entries == 0)
      return NULL;
   if(list_ptr->ll_entries == 1)
   {
      ret = list_ptr->ll_head->data_ptr;
      free(list_ptr->ll_head);
      list_ptr->ll_head = NULL;
      list_ptr->ll_tail = NULL;
      list_ptr->ll_entries--;
      return ret;
   }


   if(pos_index == LISTPOS_HEAD || pos_index == 0)
   {
      it = list_ptr->ll_head;
      ret = it->data_ptr;
      list_ptr->ll_head = it->ll_next;
   }
   else if(pos_index == LISTPOS_TAIL || pos_index == list_ptr->ll_entries - 1)
   {
      it = list_ptr->ll_tail;
      ret = it->data_ptr;
      list_ptr->ll_tail = it->ll_prev;
   }
   else
   {
      int i;
      it = list_ptr->ll_head;
      for(i = 0; i < pos_index; i++, it=it->ll_next){
      }
      ret = it->data_ptr;
   }

   // remove node
   if(it->ll_prev != NULL)
      it->ll_prev->ll_next = it->ll_next;
   if(it->ll_next != NULL)
      it->ll_next->ll_prev = it->ll_prev;

   free(it);
   list_ptr->ll_entries--;
   return ret;
}


/* Obtains the length of the specified list, that is, the number of elements
 * that the list contains.
 *
 * list_ptr: pointer to list-of-interest.
 *
 * Returns an integer equal to the number of elements stored in the list.  An
 * empty list has a size of zero.
 */
int list_entries(list_t *list_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->ll_entries >= 0);
    return list_ptr->ll_entries;
}


/* This function verifies that the pointers for the two-way linked list are
 * valid, and that the list size matches the number of items in the list.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 * The function produces no output if the two-way linked list is correct.  It
 * causes the program to terminate and print a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive, so an error may still exist in the
 * list even if these checks pass.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 */
void list_debug_validate(const list_t *L)
{
    list_node_t *N;
    int count = 0;
    assert(L != NULL);
    if (L->ll_head == NULL)
        assert(L->ll_tail == NULL && L->ll_entries == 0);
    if (L->ll_tail == NULL)
        assert(L->ll_head == NULL && L->ll_entries == 0);
    if (L->ll_entries == 0)
        assert(L->ll_head == NULL && L->ll_tail == NULL);
    assert(L->ll_sorted_state == LIST_SORTED || L->ll_sorted_state == LIST_UNSORTED);

    if (L->ll_entries == 1) {
        assert(L->ll_head == L->ll_tail && L->ll_head != NULL);
        assert(L->ll_head->ll_next == NULL && L->ll_head->ll_prev == NULL);
        assert(L->ll_head->data_ptr != NULL);
    }
    if (L->ll_head == L->ll_tail && L->ll_head != NULL)
        assert(L->ll_entries == 1);
    if (L->ll_entries > 1) {
        assert(L->ll_head != L->ll_tail && L->ll_head != NULL && L->ll_tail != NULL);
        N = L->ll_head;
        assert(N->ll_prev == NULL);
        while (N != NULL) {
            assert(N->data_ptr != NULL);
            if (N->ll_next != NULL) assert(N->ll_next->ll_prev == N);
            else assert(N == L->ll_tail);
            count++;
            N = N->ll_next;
        }
        assert(count == L->ll_entries);
    }
    if (L->ll_sorted_state == LIST_SORTED && L->ll_head != NULL) {
        N = L->ll_head;
        while (N->ll_next != NULL) {
            assert(L->comp_proc(N->data_ptr, N->ll_next->data_ptr) != -1);
            N = N->ll_next;
        }
    }
}
/* commands for vim. ts: tabstop, sts: softtabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
