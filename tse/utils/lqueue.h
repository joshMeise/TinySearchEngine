#pragma once
/* 
 * lqueue.h -- public interface to the locked queue module
 */

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>


/* the queue representation is hidden from users of the module */
typedef void lqueue_t;		

/* create an empty lcoked queue */
lqueue_t* lqopen(void);        

/* deallocate a locked queue, frees everything in it */
void lqclose(lqueue_t *lqp);   

/* put element at the end of the loecked queue
 * returns 0 is successful; nonzero otherwise 
 */
int32_t lqput(lqueue_t *lqp, void *elementp); 

/* get the first first element from locked queue, removing it from the locked queue */
void* lqget(lqueue_t *lqp);

/* apply a function to every element of the locked queue */
void lqapply(lqueue_t *lqp, void (*fn)(void* elementp));

/* search a locked queue using a supplied boolean function
 * skeyp -- a key to search for
 * searchfn -- a function applied to every element of the queue
 *          -- elementp - a pointer to an element
 *          -- keyp - the key being searched for (i.e. will be 
 *             set to skey at each step of the search
 *          -- returns TRUE or FALSE as defined in bool.h
 * returns a pointer to an element, or NULL if not found
 */
void* lqsearch(lqueue_t *lqp, 
							bool (*searchfn)(void* elementp,const void* keyp),
							const void* skeyp);
