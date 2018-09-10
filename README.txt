Harsh Patel

This project includes a thread library to create and manage user threads

KNOWN PROBLEMS
--------------
* There are some issues with preemptive testing. Although the autograder said it was fine, 
  My own tests showed that with a 10us timer, the preemptive tests are not very successful in 
  completeing. I just used a modified file of an example from your git, 
  so perhaps my test driver is faulty. 

* Other than that, I've checked cooperative mode with and without locks/condition variables
  both with 10 up to 100000 threads. These worked as expected

DESIGN
-------
The basic design for managing thread is based on using queues/lists to hold a thread_t data stucture 
which keeps track of a thread_id and its context. The queues were made by using the list.c implementation
from last semester's ECE 2230 class with Dr. Russell. I used this becuase I know the list is bug free.
The list was implemented into a queue using the datatypes.h file which contains the thread_t structure, 
and the thread_support.c file which contains functions specifically for managing a list/queue of threads.

Inside the library, I have global lists which keep track of the current job list (scheduler - used as queue), 
threads that are ready to run but not necessarily in the job list (activeThreads - used as list) and then 
a list of threads that are blocked (blockedThreads - used as list). Basically when yeild is called or a 
new thread is created, the threads in scheduler are run until it is empty and then all of the active threads
besides the cleaner (more on that below) are queued in order of arrival. If a thread is waiting on a condition
variable, it is removed from the scheduler and activeThreads lists and put on blockedThreads until it is signalled,
at which point it is returned to the activeThreads list. If a thread exits then it is removed from all lists.
** Note activeThreads always has at least master (main thread) and cleaner at index 0 and 1 respectively **

The master thread, cleaner thread and running are global variables. The cleaner thread runs the function 
release on a thread that exits. This function free's the exiting thread's (running) stack and thread_t stucture 
before yeilding to the next thread. calls to the cleaner thread are only made in threadExit and that 
to is a setcontext so that cleaner starts at the beginning. the variable running is the currently running thread
UNLESS cleaner is running, in which case its the thread that is exiting. (cleaner will update this before
exiting)

The condition variables are implemented by using 1 queue per condition variable. Any thread that waits here
will be put on the queue and when a signal happens, the first thread in line will "Wake-up". if the queue
is empty, signal has no effect.

Locks are used as such: defined constant UNLOCKED means the lock is free and anyother integer means its taken.
The way this is set up is that the thread that takes the lock, sets the lock as its thread id. So when
unlock is called, the calling thread must "own" that lock (tid has to match).