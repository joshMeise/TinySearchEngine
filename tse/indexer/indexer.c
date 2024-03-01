/* 
 * indexer.c --- indexes queries for a given webpage.
 * 
 * Author: Joshua M. Meise
 * Created: 10-18-2023
 * Version: 1.0
 * 
 * Description: Creates an index file for a given directory crawled by the crawler.
 *              Places words and their number of occurances into a hash table and writes to file.
 *              Given directory wihtin crawler, indexes all files in given directory.
 */

#include <stdlib.h>
#include <stdio.h>
#include <webpage.h>
#include <pageio.h>
#include <string.h>
#include <ctype.h>
#include <hash.h>
#include <queue.h>
#include <sys/stat.h>
#include <indexio.h>

// Structure that contains a word and a queue that containts documents and number of occurances.
typedef struct wordQ {
	char *word;
	queue_t *qp;
} wordQ_t;

// Structure that contains a document and a number of occurances.
typedef struct docCount {
	int doc;
	int count;
} docCount_t;

/*
 * This function compares the current word to one in the hash table.
 * Inputs: word to search for; word in hash table.
 * Outputs: true if found in hash table; false if not found in hash table.
 */
static bool search(void *elementp, const void *searchkeyp) {
	// Declare variables and coerce to valid datatypes.
	wordQ_t *data = (wordQ_t *)elementp;
	char *searchString = (char *)searchkeyp;
	char *string = data->word;

	// See if URL passed in is same as relevant URL in hash table.
	if (strcmp(string, searchString) == 0)
		return true;
	else
		return false;
	
}

/*
 * This function compares a document ID of a given element to a document ID in the queue.
 * Inputs: docID to search for; docID in queue.
 * Outputs: true if found in queue; false if not found in queue.
 */
static bool searchQueue(void *elementp, const void *searchkeyp) {
	// Declare variables and coerce to valid datatypes.
	docCount_t *data = (docCount_t *)elementp;
	int *searchVal = (int *)searchkeyp;
	int comp = data->doc;

	// See if the 2 document IDs are the same.
	if (comp == *searchVal)
		return true;
	else
		return false;
	
}

/*
 * Changes words to lowercase.
 * Discards words that are less than 3 characters or that contain non-alphanumeric characters.
 * Input: word to change
 * Output: 0 for a word that is successfully converted, 1 for a word that needs to be discarded.
 */
static int normalizeWord(char *word) {
	// Variable declarations.
	int32_t i;

	// Word less than 3 characters get discarded.
	if (strlen(word) < 3)
		return 1;
	
	// Start at first character.
	i = 0;

	// Loop through word and change all letters to lowercase.
	while (i < strlen(word)) {
		// Check if letter is alphanumeric. If not return 1.
		if (isalpha(word[i]) == 0)
			return 1;
		
		// Change word to lowercase.
		if(word[i] >= 'A' && word [i] <= 'Z')
			word[i] = tolower(word[i]);

		// Increment i.
		i++;
	}

	// Place null terminator at the end of the word.
	word[i] = '\0';

	return 0;
}

/*
 * Frees memory allocated for each word structure.
 * Inputs: Pointer to a word structure.
 * Outputs: None.
 */
static void freeW(void *data) {
	// Coerce to vaild datatypes.
	wordQ_t *wrd = (wordQ_t *)data;

	// Free the memory that was allocated.
	free(wrd->word);
	qapply(wrd->qp, free);
	qclose(wrd->qp);
	free(wrd);
}

int main(int argc, char *argv[]) {
	// Variable declarations.
	webpage_t *pageLoad;
	char *readWord, fname[50];
	int32_t pos, docID = 3, curID;
	wordQ_t *wordQueue;
	hashtable_t *hash;
	docCount_t *dc;
	struct stat dir;
	
	// Check number of arguments.
	if (argc != 3) {
		printf("usage: indexer <pagedir> <indexnm>\n");
		exit(EXIT_FAILURE);
	}

	// Get details of directory.
	if (stat(argv[1], &dir) != 0)
		printf("Failure on stat.\n");

	// Check if directory exists; if not create one; if creation fails, return 1.
	if (S_ISDIR(dir.st_mode) == 0) {
		printf("usage: indexer <pagedir> <indexnm>\n");
		exit(EXIT_FAILURE);
	}

	// Open a hash table for the index.
	if ((hash = hopen(10000)) == NULL)
		printf("Failure on opening hash table.\n");

	// Initialise current ID.
	curID = 1;

	// Initialise file name to be the first id.
	sprintf(fname, "../crawler/page/%d", curID);
	
	while (access(fname, R_OK) == 0) {
		// Load webpage at given index.
		pageLoad = pageload(curID, argv[1]);

		// Initialise pos to be 0.
		pos = 0;
		
		while ((pos = webpage_getNextWord(pageLoad, pos, &readWord)) > 0) {
			// Only put word in hash table if it can be normalized.
			if (normalizeWord(readWord) == 0) {
				// If first occurance of word.
				if ((wordQueue = hsearch(hash, search, readWord, strlen(readWord))) == NULL) {
					// Create a new word structure.
					if ((wordQueue = (wordQ_t *)malloc(sizeof(wordQ_t))) == NULL)
						printf("Word-queue pair creation unseccessful.\n");
					
					// Allocate space for word.
					if ((wordQueue->word = (char *)malloc(strlen(readWord)*sizeof(char) + 1)) == NULL)
						printf("Word creation unseccessfu.\n");
					
					// Initialise word.
					strcpy(wordQueue->word, readWord);
					
					// Open a queue associated with the word.
					if ((wordQueue->qp = qopen()) == NULL)
						printf("Queue not successfully opened.\n");
					
					// Put word int hash table.
					if (hput(hash, wordQueue, wordQueue->word, strlen(wordQueue->word)) != 0)
						printf("Unsuccessful put into hash word: %s.\n", wordQueue->word);

				}
	 
				// Find the element in the word's queue associated with the current ID.
				// If the document ID is not already in the queue.
				if ((dc = (docCount_t *)qsearch(wordQueue->qp, searchQueue, (void *)&curID)) == NULL) {
					// Create a new document/count pair.
					if ((dc = (docCount_t *)malloc(sizeof(docCount_t))) == NULL)
						printf("Doc count paur not successfully allocated.\n");
					
					// Initialise this new document/count pair.
					dc->doc = curID;
					dc->count = 1;
					
					// Put document/count pair into queue.
					if (qput(wordQueue->qp, (void *)dc) != 0)
						printf("Problem putting docID %d for word %s into queue.\n", docID, wordQueue->word);
				}
				// If the document ID is already in the queue.
				else
					// Increase the number of occurances in the specified document.
					(dc->count)++;
			}
			// Free the word that was read from the webpage.
			free(readWord);
		}
		// Delete the webpage.
		webpage_delete(pageLoad);

		// Increment current ID.
		curID++;
		
		// Initialise file name to be the first id.
		sprintf(fname, "../crawler/page/%d", curID);
		
	}
	
	
	// Save index.
	if (indexsave(hash, argv[2]) != 0)
		printf("Error saving index.\n");
	
	// Free memory.
	happly(hash, freeW);
	hclose(hash);
	
	exit(EXIT_SUCCESS);
}

