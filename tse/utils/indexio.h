#pragma once
/* 
 * indexio.h --- Interface for indexer.
 * 
 * Author: Joshua M. Meise
 * Created: 10-21-2023
 * Version: 1.0
 * 
 * Description: Contains functions to save an index to a file and read an index from a file.
 * 
 */

#include <inttypes.h>
#include <hash.h>
#include <stdio.h>
#include <queue.h>

/*
 * Function to save an index to a file.
 * Inputs: Index to save, name of file to save to.
 * Outputs: 0 for success; non-zero for failure.
 */
int32_t indexsave(hashtable_t *index, char *indexnm);

/*
 * Function to load an index from a file.
 * Inputs: File name to load from.
 * Outputs: Hash table cotaining index, NULL if failure
 */
hashtable_t *indexload(char *indexnm);

