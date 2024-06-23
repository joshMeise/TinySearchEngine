/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 * Josh Meise
 */

#include <stdint.h>
#include <queue.h>
#include <hash.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Table data structure. Contains size of table (number of slots) and array of queues (one for each slot).
typedef struct privatehash {
	uint32_t size;
	queue_t **thingsInTable;
} privatehash_t;

/* 
 * SuperFastHash() -- produces a number between 0 and the tablesize-1.
 * 
 * The following (rather complicated) code, has been taken from Paul
 * Hsieh's website under the terms of the BSD license. It's a hash
 * function used all over the place nowadays, including Google Sparse
 * Hash.
 */
#define get16bits(d) (*((const uint16_t *) (d)))

static uint32_t SuperFastHash (const char *data,int len,uint32_t tablesize) {
  uint32_t hash = len, tmp;
  int rem;
  
  if (len <= 0 || data == NULL)
		return 0;
  rem = len & 3;
  len >>= 2;
  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }
  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }
  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash % tablesize;
}

/* 
 * hopen -- opens a hash table with initial size hsize 
 * inputs: size of hash table (number of slots)
 * outputs: pointer to hash table structure
 */
hashtable_t *hopen(uint32_t hsize) {
	// Variable declarations.
	privatehash_t *table;
	int32_t i;
	
	// Create table by allocating memory.
	table = (privatehash_t *)malloc(sizeof(privatehash_t));
	
	// If the table has been created, initialise its size and a pointer to the items in the table.
	if (table != NULL) {
		// Number of slots in table.
		table->size = hsize;

		// Array to hold queues in table.
		table->thingsInTable = (queue_t **)malloc(hsize*sizeof(queue_t *));

		// Open a queue at each location in the table.
		for (i = 0; i < hsize; i++)
			table->thingsInTable[i] = qopen();
		
	}
	
	// Return the newly created table.
	return (hashtable_t*)table;
}

/* 
 * hclose -- closes a hash table and frees all memory associated with it
 * inputs: table to close 
 * outputs: none
 */
void hclose(hashtable_t *htp) {
	// Variable declarations.
	int32_t i;
	privatehash_t *phtp;

	// Coerce into valid datatype.
	phtp = (privatehash_t *)htp;

	// Close queues at each location.
	for (i = 0; i < phtp->size; i++)
		qclose(phtp->thingsInTable[i]);

	// Free array and table structure.
	free(phtp->thingsInTable);
	free(phtp);
}

/* 
 * hput -- puts an entry into a hash table under designated key
 * inputs: table into which to insert, element to place in table, key associated with element, length of the key
 * outputs: 0 for success, nonzero otherwise
 */
int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen) {
	//Variable declarations.
	privatehash_t *phtp;
	uint32_t location, success;

	// Check if all arguments are valid.
	if (htp == NULL || ep == NULL || key == NULL)
		return 1;

	// Coerce into correct datatype.
	phtp = (privatehash_t *)htp;

	// Find slot for key.
	location = SuperFastHash(key, keylen, phtp->size);

	// Put the data into the queue at the corresponding key's location.
	success = qput(phtp->thingsInTable[location], ep);

	return success;
	
}

/* 
 * happly -- applies a function to every entry in hash table
 * inputs: table to which to apply function, function to apply
 * outputs: none
 */
void happly(hashtable_t *htp, void (*fn)(void* ep)) {
 	// Variable declarations.
	privatehash_t *phtp;
	int32_t i;

	// Coerce into correct datatype
	phtp = (privatehash_t *)htp;

	// Apply the function to each of the queues in the table.
	for (i = 0; i < phtp->size; i++)
		qapply(phtp->thingsInTable[i], fn);
	
}

/* 
 * hsearch -- searches for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 * inputs: table to search, search function, key to search for, length of the key
 * outputs: pointer to entry that is being searched for
 */

void *hsearch(hashtable_t *htp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
	// Variable declarations.
	privatehash_t *phtp;
	void *p;
	int32_t location;

	
	// Coerce into correct datatype
	phtp = (privatehash_t *)htp;

	// Find slot in table for the designated key.
	location = SuperFastHash(key, keylen, phtp->size);

	// Search through the queue at the corresponding lodation.
	p = qsearch(phtp->thingsInTable[location], searchfn, (const void *)key);

	return p;
}

/* 
 * hremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 * inputs: table to search, search function, key to search for, length of the key
 * outputs: pointer to entry that is being searched for
 */
void *hremove(hashtable_t *htp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
	// Variable declarations.
	privatehash_t *phtp;
	void *p;
	int32_t location;

	// Coerce into valid datatype.
	phtp = (privatehash_t *)htp;

	// Find slot in table for the designated key.
	location = SuperFastHash(key, keylen, phtp->size);

	// Search through queue to find element to remove at relevant index.
	p = qremove(phtp->thingsInTable[location], searchfn, (const void *)key);

	return p;
	
}
