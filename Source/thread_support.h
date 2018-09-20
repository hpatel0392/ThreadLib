/*
* Harsh Patel
* Spring 2017
*/

#ifndef THREAD_SUPPORT_H
#define THREAD_SUPPORT_H

#include <ucontext.h>
#include "datatypes.h"
#include "list.h"

#define NEXT_THREAD -1230

int thread_comp(const thread_t *t1, const thread_t * t2);

list_t* build_list(void);
void destroy_list(list_t*);
void empty_list(list_t*);
int num_threads(list_t*);
void queue_thread(list_t*, thread_t*);
thread_t* fetch_thread(list_t*, int);
thread_t* access_thread(list_t*, int);
int thread_done(list_t*, int);
ucontext_t * delete_thread(thread_t*);
thread_t* build_thread(int tid, ucontext_t*);

#endif
