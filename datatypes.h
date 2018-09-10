/* datatypes.h
 * Harsh Patel
 * harshp
 * ECE 2230 Fall 2016
 * MP2
 *
 * Purpose: The data type that is stored in the list ADT is defined here.  We
 *          define a single mapping that allows the list ADT to be defined in
 *          terms of a generic data_t.
 *
 * Assumptions:
 *     For MP2 there are no changes to this file.  For all other MP's
 *     this file must be updated
 *
 *     data_t: The type of data that we want to store in the list
 *
 * Bugs:
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