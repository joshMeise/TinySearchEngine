/* 
 * lqueue.c -- implements the functions prototyped in the locked queue interface
 */

#include <lqueue.h>
#include <stdlib.h>
#include <queue.h>

/*
 * This structure contains the hidden aspcents of a locked queue.
 * Each locked queue contains a queue and an associated mutex.
 */
typedef struct privatelqueue {
	queue_t *qp;
	pthread_mutex_t m;
} privatelqueue_t;

/* 
 * create an empty locked queue 
 * inputs: none
 * outputs: pointer to the new queue that was created
 */
lqueue_t* lqopen(void) {
	// Variable declarations.
	privatelqueue_t *plqp;

	// Allcoate memory for the new locked queue.
	if((plqp = (privatelqueue_t *)malloc(sizeof(privatelqueue_t))) == NULL)
		return NULL;

	// Initialise the new locked queue's aspects; queue and mutex.
	if ((plqp->qp = qopen()) == NULL || pthread_mutex_init(&(plqp->m), NULL) != 0)
		return NULL;

	return (lqueue_t *)plqp;

}

/* deallocate a locked queue, frees everything in it */
void lqclose(lqueue_t *lqp) {
	// Variable declarations and coercing.
	privatelqueue_t *plqp = (privatelqueue_t *)lqp;

	// Close the associated queue.
	qclose(plqp->qp);

	// Destroy the mutex.
	pthread_mutex_destroy(&(plqp->m));

	// Free the locked queue data structure.
	free(plqp);
}

/* 
 * put element at the end of the queue
 * inputs: pointer to queue into which to insert, popinter to element to be inserted
 * returns 0 is successful; nonzero otherwise 
 */
int32_t lqput(lqueue_t *lqp, void *elementp) {
	// Variable declarations and coercing.
	privatelqueue_t *plqp = (privatelqueue_t *)lqp;
	
	// Check that queue and element exist.
	if(plqp == NULL || elementp == NULL)
		return 1;
	
	// Lock the mutex associated with the locked queue.
	if (pthread_mutex_lock(&(plqp->m)) != 0)
		return 1;

	// Put element into associated queue.
	if (qput(plqp->qp, elementp) != 0)
		return 1;

	// Unlock mutex.
	if (pthread_mutex_unlock(&(plqp->m)) != 0)
		return 1;

	return 0;
	
}

/*
 * get the first first element from queue, removing it from the queue 
 * inputs: queue from which first element is being taken
 * outputs: pointer to the data on that node of the queue
 */
void* lqget(lqueue_t *lqp) {
	// Variable declarations and coercing.
	void *data;
	privatelqueue_t *plqp = (privatelqueue_t *)lqp;
		
	// Check that queue and element exist.
	if(plqp == NULL)
		return NULL;
	
	// Lock the mutex associated with the locked queue.
	if (pthread_mutex_lock(&(plqp->m)) != 0)
		return NULL;

	// Get first element from associated queue.
	data = qget(plqp->qp);
	
	// Unlock mutex.
	if (pthread_mutex_unlock(&(plqp->m)) != 0)
		return NULL;
	
	return data;

}

/* 
 * apply a function to every element of the queue 
 * inputs: the queue to which the function will be applied, the function to apply which takes the data from a queue's node as an input
 * output: none
 */
void lqapply(lqueue_t *lqp, void (*fn)(void* elementp)) {
	// Variable declarations and coercing.
	privatelqueue_t *plqp = (privatelqueue_t *)lqp;

	// Lock the mutex associated with the locked queue.
	pthread_mutex_lock(&(plqp->m));

	// Apply function to every element in the associated queue.
	qapply(plqp->qp, fn);
	
	// Unlock mutex.
	pthread_mutex_unlock(&(plqp->m));

}

/* search a locked queue using a supplied boolean function
 * inputs: skeyp -- a key to search for
 *         searchfn -- a function applied to every element of the queue
 *                  -- elementp - a pointer to an element
 *                  -- keyp - the key being searched for (i.e. will be 
 *                     set to skey at each step of the search
 *                  -- returns TRUE or FALSE as defined in bool.h
 * output: returns a pointer to an element, or NULL if not found
 */
void* lqsearch(lqueue_t *lqp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
	// Variable declarations and coercing.
	void *data;
	privatelqueue_t *plqp = (privatelqueue_t *)lqp;
	
	// Lock the mutex associated with the locked queue.
	if (pthread_mutex_lock(&(plqp->m)) != 0)
		return NULL;
	
	// Apply search function to associated queue.
	data = qsearch(plqp->qp, searchfn, skeyp);
	
	// Unlock mutex.
	if (pthread_mutex_unlock(&(plqp->m)))
		return NULL;

	return data;
	
}
