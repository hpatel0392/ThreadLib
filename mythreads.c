void __attribute__ ((destructor)) cleanup(void);

#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mythreads.h"
#include "thread_support.h"
#include "list.h"

#define UNLOCKED -1
#define UNIT_SIZE 50000
#define LIB_THREADS 2

int interruptsAreDisabled;
static int wasInitCalled = 0;

static list_t* scheduler;
static list_t* activeThreads;
static list_t* blockedThreads;

static void** returnVals;

static int nextId;

static thread_t* running;
static thread_t* master;
static thread_t* cleaner;

static int* locks;
static list_t*** cond_vars;

static void release();
static void thread_func(thFuncPtr, void*);
static void scheduleThreads();
static void interruptEnable();
static void interruptDisable();


// initialize the thread library
void threadInit(){
   interruptsAreDisabled = 1; // initiallize
   wasInitCalled = 1;

   locks = (int*)calloc(NUM_LOCKS, sizeof(int));
   cond_vars = (list_t***)malloc(NUM_LOCKS*sizeof(list_t**));
   int i, j;
   for(i = 0; i < NUM_LOCKS; i++){
      cond_vars[i] = (list_t**)malloc(CONDITIONS_PER_LOCK * sizeof(list_t*));
      for(j = 0; j < CONDITIONS_PER_LOCK; j++){
         cond_vars[i][j] = build_list();
      }
      locks[i] = UNLOCKED;
   }

   scheduler = build_list();
   activeThreads = build_list();
   blockedThreads = build_list();

   // main
   ucontext_t* con = (ucontext_t*)malloc(sizeof(ucontext_t));
   master = build_thread(0, con);
   assert(getcontext(con) != -1);
   running = master;
   queue_thread(activeThreads, master);
   nextId = 1; // main is zero
   con = NULL;
   
   // cleanup thread
   con = (ucontext_t*)malloc(sizeof(ucontext_t));
   cleaner = build_thread(nextId++, con);
   getcontext(con);
   con->uc_stack.ss_sp =malloc(STACK_SIZE);
   con->uc_stack.ss_size = STACK_SIZE;
   makecontext(con, release, 0);
   queue_thread(activeThreads, cleaner); 
   con = NULL;
  
   returnVals = (void**)calloc(UNIT_SIZE, sizeof(void*));
   interruptEnable();
}


// the threadfunction to run, which sends return value to threadExit
static void thread_func(thFuncPtr fptr, void* args){
   threadExit(fptr(args));
}
   

// schedule all active threads
static void scheduleThreads(){
   // user threads waiting to run but not scheduled
   queue_thread(scheduler, master);
   if(num_threads(activeThreads) > LIB_THREADS){ 
      int i;
      for(i = LIB_THREADS; i < num_threads(activeThreads); i++){
         queue_thread(scheduler, access_thread(activeThreads, i));
      }
   }
}
   

// create new thread and run it
// returns a unique thread id for the thread
int threadCreate ( thFuncPtr funcPtr , void * argPtr ){
   //interruptsAreDisabled = 1;
   interruptDisable();

   ucontext_t* con = (ucontext_t*)malloc(sizeof(ucontext_t));
   thread_t* new = build_thread(nextId++, con);
   getcontext(con);
   con->uc_stack.ss_sp = malloc(STACK_SIZE);
   con->uc_stack.ss_size = STACK_SIZE;
   makecontext(con, (void (*) (void))thread_func, 2, funcPtr, argPtr);
   queue_thread(activeThreads, new); // add to active threads

   if(num_threads(scheduler) == 0){
      scheduleThreads();
   }
   
   int ret = nextId-1;
   ucontext_t* old = running->context;
   running = new;

   if((nextId % UNIT_SIZE) == 0){
      int newSize = nextId / UNIT_SIZE;
      newSize++;
      newSize *= UNIT_SIZE;
      returnVals = (void**)realloc(returnVals, newSize * sizeof(void*)); 
   }  

   interruptEnable();
   
   swapcontext(old, con);

   
   interruptDisable();

   con = NULL;
   interruptEnable();
   return ret;
}

// calling thread is halted and swapped with another thread
void threadYield (){
   
   interruptDisable();

   if(nextId <= LIB_THREADS || num_threads(activeThreads) == LIB_THREADS){
      interruptEnable();
      return;
   }

   if(num_threads(scheduler) == 0){
      scheduleThreads();
   }
   
   ucontext_t *old, *new;
   old = running->context;
   running = fetch_thread(scheduler, NEXT_THREAD); // pop next thread from queue
   new = running->context;
 
   interruptEnable();
   swapcontext(old, new);
   
}


// wait for thread with id provided 
void threadJoin ( int thread_id , void ** result ){
   
   interruptDisable();
     
   while( thread_done(blockedThreads, thread_id) && !thread_done(activeThreads, thread_id) ){ 
      interruptEnable();
      threadYield();
      interruptDisable();
   }

   if(result != NULL)
      *result = returnVals[thread_id];
   
   interruptEnable();
   
}


// end current thread
void threadExit ( void * result ){
  
   interruptDisable();
   if(running == master){
      exit(1);
   }
   returnVals[running->thread_id] = result;
   setcontext(cleaner->context);
}


// free an exiting stacks memory
static void release(){
   ucontext_t* del= delete_thread(fetch_thread(activeThreads, running->thread_id));
   free(del->uc_stack.ss_sp);
   free(del);

   while(!thread_done(scheduler, running->thread_id)){
      fetch_thread(scheduler, running->thread_id);
   }

   if(num_threads(scheduler) == 0)
      scheduleThreads();
  
   running = fetch_thread(scheduler, NEXT_THREAD);

   interruptEnable(); // continue from threadExit
   setcontext(running->context);
}



// lock the specified lock
void threadLock ( int lockNum ){
  
   interruptDisable();
   while(locks[lockNum] != UNLOCKED){
      interruptEnable();
      threadYield();
      interruptsAreDisabled = 1;
      //interruptDisable();
   }
   locks[lockNum] = running->thread_id;
   interruptEnable();
}

// unlock specified lock
void threadUnlock ( int lockNum ){
   
   interruptDisable();
   if(locks[lockNum] != UNLOCKED){
      assert(locks[lockNum] == running->thread_id);
      locks[lockNum] = UNLOCKED;
   }
   interruptEnable();
}


// wait on the specified condition variable of the specified lock
void threadWait ( int lockNum , int conditionNum ){
  
   interruptDisable();
   if(locks[lockNum] == UNLOCKED || locks[lockNum] != running->thread_id){
      printf("\n Fatal Error!! Calling threadWait without owning Lock!\n");
      exit(-1);
   }
   
   interruptEnable();
   threadUnlock(lockNum);
   
   interruptDisable();

   queue_thread(cond_vars[lockNum][conditionNum], running);
   queue_thread(blockedThreads, fetch_thread(activeThreads, running->thread_id)); // block thread


   while(!thread_done(scheduler, running->thread_id)){
      fetch_thread(scheduler, running->thread_id);
   }

   
   thread_t* blocked = running;
   if(num_threads(scheduler) == 0)
      scheduleThreads();
   
   running = fetch_thread(scheduler, NEXT_THREAD);
   
   interruptEnable();
   swapcontext(blocked->context, running->context);
   threadLock(lockNum); // reaquire lock
}


// signal the specified condition variable and run a thread waiting there
void threadSignal ( int lockNum , int conditionNum ){
  
   interruptDisable();
   if(num_threads(cond_vars[lockNum][conditionNum]) == 0){
      interruptEnable();
      return;
   }
   
   thread_t* signaled = running;
   queue_thread(scheduler, signaled);
   running = fetch_thread(cond_vars[lockNum][conditionNum], NEXT_THREAD);
   fetch_thread(blockedThreads, running->thread_id); // remove from blocked list
   queue_thread(activeThreads, running); // thread is unblocked and ready to queue
 
   interruptEnable();
   swapcontext(signaled->context, running->context);
}


// disable interrupts
static void interruptDisable () {
   assert (! interruptsAreDisabled ) ;
   interruptsAreDisabled = 1;
}

// enable interrupts
static void interruptEnable () {
   assert ( interruptsAreDisabled ) ;
   interruptsAreDisabled = 0;
}

void cleanup(void){
   if(!wasInitCalled)
      return;

   // free locks
   free(locks);
   
   // free cond variables
   int i, j;
   for(i = 0; i < NUM_LOCKS; i++){
      for(j = 0; j < CONDITIONS_PER_LOCK; j++){
         empty_list(cond_vars[i][j]);
         destroy_list(cond_vars[i][j]);
      }
      free(cond_vars[i]);
   }
   free(cond_vars);
   
   // empty scheduler and blockedThreads
   empty_list(scheduler);
   empty_list(blockedThreads);

   // free the thread memory
   ucontext_t* con;
   free(delete_thread(fetch_thread(activeThreads, NEXT_THREAD))); // free master
      
   while(num_threads(activeThreads) != 0){
      con = delete_thread(fetch_thread(activeThreads, NEXT_THREAD));
      free(con->uc_stack.ss_sp);
      free(con);
      con = NULL;
   }
   assert(num_threads(activeThreads) == 0);

   free(returnVals);
   // destroy lists
   destroy_list(activeThreads);
   destroy_list(scheduler);
   destroy_list(blockedThreads);
       
}
