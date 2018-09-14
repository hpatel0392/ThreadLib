/* datatypes.h
 * Harsh Patel
 * Spring 2017
*/

#ifndef DATATYPES_H
#define DATATYPES_H

#include <ucontext.h>

typedef struct thread_tag {
    int thread_id;      
    ucontext_t * context;    
} thread_t;

/* the list ADT works on socket data of this type */
typedef thread_t data_t;

/* commands specified to vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */

#endif