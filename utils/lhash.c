/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 * Josh Meise
 */

#include <stdint.h>
#include <hash.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <lhash.h>

/*
 * This structure contains the hidden aspcents of a locked hash table.
 * Each locked hash table contains a hash table and an associated mutex.
 */
typedef struct privatelhash {
	hashtable_t *hash;
	pthread_mutex_t m;
} privatelhash_t;

/* 
 * lhopen -- opens a locked hash table with initial size hsize 
 * inputs: size of hash table (number of slots)
 * outputs: pointer to locked hash table structure
 */
lhashtable_t *lhopen(uint32_t hsize) {
	// Variable declarations.
	privatelhash_t *table;
	
	// Create table by allocating memory.
	table = (privatelhash_t *)malloc(sizeof(privatelhash_t));
	
	// If the table has been created, open its associated hash table and mutex.
	if (table != NULL)
		// Open hash table and initialise associated mutex.
		if ((table->hash = hopen(hsize)) == NULL || pthread_mutex_init(&(table->m), NULL) != 0)
			return NULL;
	
	// Return the newly created locked table.
	return (lhashtable_t*)table;
}

/* 
 * lhclose -- closes a locked hash table and frees all memory associated with it
 * inputs: table to close 
 * outputs: none
 */
void lhclose(lhashtable_t *lhtp) {
	// Variable declarations.
	privatelhash_t *lphtp;

	// Coerce into valid datatype.
	lphtp = (privatelhash_t *)lhtp;

	// Close associated hash table.
	hclose(lphtp->hash);
	
	// Destroy the mutex.
	pthread_mutex_destroy(&(lphtp->m));
	
	// Free locked hash structure.
	free(lphtp);
	
}

/* 
 * lhput -- puts an entry into a locked hash table under designated key
 * inputs: table into which to insert, element to place in table, key associated with element, length of the key
 * outputs: 0 for success, nonzero otherwise
 */
int32_t lhput(lhashtable_t *lhtp, void *ep, const char *key, int keylen) {
	//Variable declarations.
	privatelhash_t *lphtp;

	// Check if all arguments are valid.
	if (lhtp == NULL || ep == NULL || key == NULL)
		return 1;

	// Coerce into correct datatype.
	lphtp = (privatelhash_t *)lhtp;

	// Lock the mutex associated with the locked hash.
	if (pthread_mutex_lock(&(lphtp->m)) != 0)
		return 1;
	
	// Put into hash table.
	if (hput(lphtp->hash, ep, key, keylen) != 0)
		return 1;
	
	// Unlock mutex.
	if (pthread_mutex_unlock(&(lphtp->m)) != 0)
		return 1;

	return 0;
	
}

/* 
 * lhapply -- applies a function to every entry in locked hash table
 * inputs: table to which to apply function, function to apply
 * outputs: none
 */
void lhapply(lhashtable_t *lhtp, void (*fn)(void* ep)) {
 	// Variable declarations.
	privatelhash_t *lphtp;

	// Coerce into correct datatype
	lphtp = (privatelhash_t *)lhtp;

	// Lock the mutex associated with the locked hash table.
	pthread_mutex_lock(&(lphtp->m));

	// Apply function to associated hash table.
	happly(lphtp->hash, fn);
	
	// Unlock mutex.
	pthread_mutex_unlock(&(lphtp->m));
 
}

/* 
 * lhsearch -- searches for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 * inputs: table to search, search function, key to search for, length of the key
 * outputs: pointer to entry that is being searched for
 */

void *lhsearch(lhashtable_t *lhtp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
 	// Variable declarations.
	privatelhash_t *lphtp;
	void *data;

	// Coerce into correct datatype
	lphtp = (privatelhash_t *)lhtp;

	// Lock the mutex associated with the locked hash table.
	if ((pthread_mutex_lock(&(lphtp->m))) != 0)
		return NULL;
	
	// Search the associated hash table.
	data = hsearch(lphtp->hash, searchfn, key, keylen);
	
	// Unlock mutex.
	if ((pthread_mutex_unlock(&(lphtp->m))) != 0)
		return NULL;

	return data;

}

/* 
 * lhremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 * inputs: table to search, search function, key to search for, length of the key
 * outputs: pointer to entry that is being searched for
 */
void *lhremove(hashtable_t *lhtp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
 	// Variable declarations.
	privatelhash_t *lphtp;
	void *data;

	// Coerce into correct datatype
	lphtp = (privatelhash_t *)lhtp;

	// Lock the mutex associated with the locked hash table.
	if ((pthread_mutex_lock(&(lphtp->m))) != 0)
		return NULL;

	// Search the associated hash table.
	data = hremove(lphtp->hash, searchfn, key, keylen);
	
	// Unlock mutex.
	if ((pthread_mutex_unlock(&(lphtp->m))) != 0)
		return NULL;

	return data;
	
}
