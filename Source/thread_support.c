/*
* Harsh Patel
* Spring 2017
*/

#include <stdlib.h>
#include <assert.h>

#include "datatypes.h"
#include "list.h"
#include "thread_support.h"

// compare two threads by thread id
// if tids are same, contexts are assumed to be same
// because I made sure of it
int thread_comp(const thread_t *t1, const thread_t *t2)
{
    assert(t1 != NULL && t2 != NULL);
    if (t1->thread_id == t2->thread_id)
	return 1;
    else
	return 0;
}

// make a list
list_t *build_list(void)
{
    return list_construct(thread_comp);
}

// free list memory
void destroy_list(list_t *list_ptr)
{
   list_destruct(list_ptr);
}

// empty the list - meant to be used on scheduler, cond_vars and blockedThreads
// activeThreads SHOULD NOT USE THIS because then threads are not free'd
void empty_list(list_t* list_ptr){
   while(list_entries(list_ptr) != 0){
      fetch_thread(list_ptr, NEXT_THREAD);
   }
   assert((list_entries(list_ptr)) == 0);
}

// number of threads in the list
int num_threads(list_t *list_ptr)
{
    return list_entries(list_ptr);
}


// add thread to queue
void queue_thread(list_t* list_ptr, thread_t* t){
   list_insert(list_ptr, t, LISTPOS_TAIL);
}

/* checks if a thread with tid provided exists in list
* meant to be used with activeThreads
* return 1 means thread not in list and 0 means it is
* this is done like this because other list functions depend on output format
*/
int thread_done(list_t* list_ptr, int tid){
   if(list_entries(list_ptr) == 0)
      return 1;

   int pos;
   thread_t to_find;
   to_find.thread_id = tid;
   if(list_elem_find(list_ptr, &to_find, &pos) == NULL)
      return 1;
   return 0;
}

// fetches next runnable thread
thread_t* fetch_thread(list_t* list_ptr, int tid){
   if(tid == NEXT_THREAD)
      return list_remove(list_ptr, LISTPOS_HEAD);
   else{
      int pos;
      thread_t to_find;
      to_find.thread_id = tid;
      list_elem_find(list_ptr, &to_find, &pos);
      return list_remove(list_ptr, pos);
   }
}

// return a thread at the INDEX provided
// should only be used if thread is guaranteed to be at that pos
thread_t* access_thread(list_t* list_ptr, int pos){
   return list_access(list_ptr, pos);
}

// free the thread structure and return the context
ucontext_t* delete_thread(thread_t* t){
   ucontext_t* ret = t->context;
   t->context = NULL;
   free(t);
   return ret;
}

// build a thread from a context which is already instantiated
thread_t* build_thread(int tid, ucontext_t* c){
   thread_t* new = (thread_t*)malloc(sizeof(thread_t));
   new->thread_id = tid;
   new->context = c;
   return new;
}
