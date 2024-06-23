/* 
 * queue.c --- implements the functions described in the queue.h interface
 * 
 * Author: Joshua Meise
 * Created: 09-30-2023
 * Version: 1.0
 * 
 * Description: This module implements a queue data structure.
 * 
 */

#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

/* This structure contains an individual node of a queue.
 * Each node contains data of some type.
 * Each node also contains a pointer to the next node in the queue.
 */
typedef struct queuenode {
	void *data;
	struct queuenode *next;
} queuenode_t;

/* This structure contains the hidden aspects of an queue.
 * Each queue will have a front and a back pointer which point to nodes in that queue.
 */
typedef struct privateQueue {
	struct queuenode *front;
	struct queuenode *back;
} privateQueue_t;

// Function prototypes for hidden functions.
static queuenode_t *newQueueNode(void *elementp);

/* 
 * create an empty queue 
 * inputs: none
 * outputs: pointer to the new queue that was created
 */
queue_t* qopen(void) {
	// Variable declarations.
	privateQueue_t *newQ;

	// Allocate space for the new queue what is being created.
	newQ = (privateQueue_t *)malloc(sizeof(privateQueue_t));

	// Checks that the queue was created and initialises pointers to front and back; returns NULL is queue not created.
  if (newQ != NULL) {
    newQ->front = NULL;
		newQ->back = NULL;
  }
	else {
		return NULL;
	}

	// Return new queue that was created; coerce to "non-hidden" queue.
  return (queue_t *)newQ;
}


/* 
 * deallocate a queue, frees everything in it
 * inputs: queue to be freed
 */ 
void qclose(queue_t *qp){
	// Variable declarations.
	privateQueue_t *pQ;
	queuenode_t *ptr, *tmp;

	// Coerce queue to private queue type.
	pQ = (privateQueue_t *)qp;

	// Start freeing from front of list.
	ptr = pQ->front;

	// Loop through whole queue freeing each node successively/\.
	while(ptr !=NULL){
		tmp = ptr->next;
		free(ptr);
		ptr = tmp;
		
	}

	// Free the queue itself.
	free(pQ);
	
}

/* put element at the end of the queue
 * inputs: pointer to queue into which to insert, popinter to element to be inserted
 * returns 0 is successful; nonzero otherwise 
 */
int32_t qput(queue_t *qp, void *elementp) {
	// Variable declarations.
	queuenode_t *newNode;
	privateQueue_t *pQ;

	// Coerce queue to private queue type.
	pQ = (privateQueue_t *)qp;

	// Create a new node to add to the queue.
	newNode = newQueueNode(elementp);
	
	// Unseccssful if either are non-existent
	if (pQ == NULL || newNode == NULL)
		return(1);

	// If adding is first node in queue.
	if (pQ->front == NULL) {
		// Front and back both point to the new node.
		pQ->front = newNode;
		pQ->back = newNode;
	}
	// If not the first node being added, simply insert at back of queue.
	else {
		pQ->back->next = newNode;
		pQ->back = newNode;
	}

	return(0);
}

/*
 * get the first first element from queue, removing it from the queue 
 * inputs: queue from which first element is being taken
 * outputs: pointer to the data on that node of the queue
 */
void* qget(queue_t *qp) {
	//Variabel declarations.
	privateQueue_t *pQ;
	void *p;
	queuenode_t *curFrt;

	// Coerce queue to private queue type.
	pQ = (privateQueue_t *)qp;

	// If queue does not exits return NULL
	if (pQ == NULL) {
		return NULL;
	}
	// If queue is empty return NULL
	else if (pQ->front == NULL) {
		return NULL;
	}
	
	// Set the value to be returned to be the data in  the first item in the queue
	p = pQ->front->data;

	// Keep track of the current front of the queue so that it may be freed.
	curFrt = pQ->front;

	// Update the front of the queue to point to the next element in the queue.
	pQ->front = pQ->front->next;

	// Free the element that was removed from the queue.
	free(curFrt);

	// Return the data from the freed node.
	return p;
	
}

/* 
 * apply a function to every element of the queue 
 * inputs: the queue to which the function will be applied, the function to apply which takes the data from a queue's node as an input
 * output: none
 */
void qapply(queue_t *qp, void (*fn)(void* elementp)){
	// Variable declarations.
	queuenode_t *ptr;
	privateQueue_t *pQ;

	// Coerce queue to private queue type.
	pQ = (privateQueue_t *)qp;

	// Loop through the queue and apply the function to the data in each node.
	for (ptr = pQ->front; ptr!=NULL; ptr = ptr->next){
		fn(ptr->data);
	}

}

/* 
 * search a queue using a supplied boolean function
 * inputs: skeyp -- a key to search for
 *         searchfn -- a function applied to every element of the queue
 *                  -- elementp - a pointer to an element
 *                  -- keyp - the key being searched for (i.e. will be set to skey at each step of the search)
 *                  -- returns TRUE or FALSE as defined in bool.h
 * output: a pointer to an element, or NULL if not found
 */
void* qsearch(queue_t *qp, bool (*searchfn)(void* elementp, const void* keyp), const void* skeyp){
	// Variable declarations.
	queuenode_t *ptr;
	privateQueue_t *pQ;
	bool result;
	
	// Coerce queue to private queue type.
	pQ = (privateQueue_t *)qp;
	
	// Loop thtough the entire queue.
	for (ptr = pQ->front; ptr!=NULL; ptr = ptr->next){
		// Call the search function and send the data and search key to it.
		result =	searchfn(ptr->data, skeyp);

		// If the search matched, return a pointer to the data in the element that is being searched for.
		if (result){
			return(ptr->data);
		}
	}

	// If nothing was found return null.
	return NULL;
}

/* 
 * search a queue using a supplied boolean function (as in qsearch),
 * removes the element from the queue and returns a pointer to it or
 * NULL if not found
 * inputs: queue to be searched, search function, ket to be searched for
 * output: data from the element that was removed
 */
void* qremove(queue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp){
	// Variable declarations.
	queuenode_t *ptr, *prev;
	privateQueue_t *pQ;
	void *p;
	bool result;

	// Check that queue exists.
	if (qp == NULL)
		return NULL;
	
	// Coerce queue to private queue type.
	pQ = (privateQueue_t *)qp;

	if (pQ->front == NULL)
		return NULL;
	
	// Loop through whole queue.
	for (ptr = pQ->front; ptr!=NULL; ptr = ptr->next){
		// Call the search function and send the data and key to to it.
		result =	searchfn(ptr->data, skeyp);
		
		// If the key search mathced the criteria.
		if (result){
			// If the first element matched the search criteria.
			if(ptr == pQ->front){
				// Call the qget() function to remove the first element.
				return(qget(qp));
			}
			// If removing the last element of the queue.
			else if (ptr == pQ->back){
				// The back of the queue now needs to point to the previous element.
				pQ->back = prev;

				// Set back's next pointer to point to NULL to indicate end of queue.
				pQ->back->next = NULL;

				// The data to be returned.
				p = ptr->data;
				
				// Free the element that was removed.
				free(ptr);
				
				return(p);
			}
			else {
				// Data to be returned.
				p = ptr->data;

				// Link the previous to the the element following the one to be removed.
				prev->next = ptr->next;

				// Free the removed element.
				free(ptr);
	
				return(p);
			}
			
		}
		// Update previous to move along the queue.
		prev = ptr; 
	}

	// Return NULL if nothing was found.
	return NULL;

}

/* 
 * concatenatenates elements of q2 into q1
 * q2 is dealocated, closed, and unusable upon completion 
 * inputs: queues to be concatenated
 * outputs: none
 */
void qconcat(queue_t *q1p, queue_t *q2p){
	// Variable declarations.
	privateQueue_t *pQ1;
	privateQueue_t *pQ2;

	// Coerce queues to private queue type.
	pQ1 = (privateQueue_t *)q1p;
	pQ2 = (privateQueue_t *)q2p;

	// If both queues are empty or the secodn queue is empty.
	if ((pQ1->front == NULL && pQ2->front == NULL) || pQ2->front == NULL)
		// Nothing to join so simply free queue 2.
		free(pQ2);
	// If the first queue is empty.
	else if (pQ1->front == NULL) {
		// Make the front of the first queue point to the front of the next queue and same thing with the back.
		pQ1->front = pQ2->front;
		pQ1->back = pQ2->back;

		// Close queue 2.
		free(pQ2);
	}
	else {
		
		// Make the back pointer of queue 1's next point to the front of queue 2.
		pQ1->back->next = pQ2->front;

		// Move the back pointer to point to the back of the new queue.
		pQ1->back = pQ2->back;

		// Close queue 2 now that they have been joined.
		free(pQ2);
	}
}

/*
 * creates a new queue node and initialises data
 * inputs: data to be inserted
 * outputs: pointer to a new node in the queue that was created
 */

static queuenode_t *newQueueNode(void *elementp) {
	// Create new node and allocate memory
	queuenode_t *newNode = (queuenode_t *)malloc(sizeof(queuenode_t));

	// If the new node was not created return NULL.
	if (newNode == NULL) {
		return NULL;
	}
	
	// Initialise the new node to have a next pointer pointing to NULL and insert data.
	newNode->next = NULL;
	newNode->data = elementp;
	
	return newNode; 
}
