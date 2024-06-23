/* 
 * indexer.c --- implements a multi-threaded indexer
 * 
 * Author: Joshua M. Meise
 * Created: 10-18-2023
 * Version: 1.0
 * 
 * Description: This program takes in as arguments a directory with webpages created by the crawler, the name of an index file and a number of threads.
 *              It opens the given number of threads, builds an index and then wrotes that index to an output file.
 * 
 */

// Library inclusions.
#include <stdlib.h>
#include <stdio.h>
#include <webpage.h>
#include <pageio.h>
#include <string.h>
#include <ctype.h>
#include <lhash.h>
#include <lqueue.h>
#include <sys/stat.h>
#include <pthread.h>

// Global variables.
lhashtable_t *hash;
int32_t nextID;
char directory[500];
pthread_mutex_t m;
FILE *ifile;

// Structure that contains a word and a queue that containts documents and number of occurances.
typedef struct wordQ {
	char *word;
	lqueue_t *qp;
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
	lqapply(wrd->qp, free);
	lqclose(wrd->qp);
	free(wrd);
}

/* Increments the file id number so that the next file is read.
 * No inputs or outputs.
 */
void updateNextID(void) {
	// Lock mutex.
	pthread_mutex_lock(&m);

	// Increment file ID number.
	nextID++;

	// Unlock mutex.
	pthread_mutex_unlock(&m);
	
}

/*
 * Operates on each thread.
 * Reads saved pages into index.
 * Inputs: None.
 * Outputs: NULL to terminate thread.
 */
void *threadFunc(void *argp) {
	// Variable declarations.
	char fname[1000], *readWord;
	int32_t pos, curID;
	webpage_t *pageLoad;
	wordQ_t *wordQueue;
	docCount_t *dc;

	// Loop through all files in crawler directory.
	do {

		// Keep track of the current file being dealt with.
		curID = nextID;

		// Move onto the next file for the next thread.
		updateNextID();
		
		// Initialise file name to be the current file being worked on.
		sprintf(fname, "%s/%d", directory, curID);

		// If file is not accessible, terminate thread.
		if (access(fname, R_OK) != 0)
			return NULL;
		
		// Load webpage for given id number.
		pageLoad = pageload(curID, directory);

		// Initialise pos to be 0.
		pos = 0;

		// Read through all words in a given webpage.
		while ((pos = webpage_getNextWord(pageLoad, pos, &readWord)) > 0) {
			// Only put word in hash table if it can be normalized.
			if (normalizeWord(readWord) == 0) {
				// If first occurance of word.
				if ((wordQueue = lhsearch(hash, search, readWord, strlen(readWord))) == NULL) {
					// Create a new word structure.
					if ((wordQueue = (wordQ_t *)malloc(sizeof(wordQ_t))) == NULL)
						printf("Word-queue pair creation unseccessful.\n");
					
					// Allocate space for word.
					if ((wordQueue->word = (char *)malloc(strlen(readWord)*sizeof(char) + 1)) == NULL)
						printf("Word creation unseccessful.\n");
					
					// Initialise word.
					strcpy(wordQueue->word, readWord);
					
					// Open a queue associated with the word.
					if ((wordQueue->qp = lqopen()) == NULL)
						printf("Queue not successfully opened.\n");
					
					// Put word int hash table.
					if (lhput(hash, wordQueue, wordQueue->word, strlen(wordQueue->word)) != 0)
						printf("Unsuccessful put into hash word: %s.\n", wordQueue->word);
				
				}
				// Find the element in the word's queue associated with the current ID.
				// If the document ID is not already in the queue.
				if ((dc = (docCount_t *)lqsearch(wordQueue->qp, searchQueue, (void *)&curID)) == NULL) {
					// Create a new document/count pair.
					if ((dc = (docCount_t *)malloc(sizeof(docCount_t))) == NULL)
						printf("Doc count paur not successfully allocated.\n");
					
					// Initialise this new document/count pair.
					dc->doc = curID;
					dc->count = 1;
					
					// Put document/count pair into queue.
					if (lqput(wordQueue->qp, (void *)dc) != 0)
						printf("Problem putting docID %d for word %s into queue.\n", nextID, wordQueue->word);
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
	} while (access(fname, R_OK) == 0);

	return NULL;
}

/* 
 * Prints the document id and number of occurances into outout file.
 * Inputs: Pointer to the element in the queue that is to be printed into the file.
 * Outputs: None.
 */
static void printFileQ(void *data) {
	// Variable declarations and coercion.
	docCount_t *d = (docCount_t *)data;

	// Write the document id and the count to a file.
	fprintf(ifile, " %d %d", d->doc, d->count);
}

/*
 * Prints word to file and calls function to print corresponding document ids and counts to file.
 * Inputs: Pointer to the word to be printed.
 * Outputs: None.
 */
static void printFileH(void *data) {
	// Variable declaration and coercion.
	wordQ_t *wrd = (wordQ_t *)data;

	// Print word to file.
	fprintf(ifile, "%s", wrd->word);

	// Work through associated queue of document/count pairs and write them to file.
	lqapply(wrd->qp, printFileQ);
	
	fprintf(ifile, "\n");
}

/*
 * Function to save an index to a file.
 * Inputs: Index to save, name of file to save to.
 * Outputs: 0 for success; non-zero for failure.
 */
int32_t indexsave(char *indexnm) {
	// Check that hash and indexnm exist.
	if (hash == NULL || indexnm == NULL)
		return 1;
	
	// Open the file for writing.
	ifile = fopen(indexnm, "w");

	// Return 1 if file not opened for writing properly.
	if (access(indexnm, W_OK) != 0)
		return 1;

	// Write to the file using happly();
	lhapply(hash, printFileH);

	// Close reading file after loop has completed execution.
	if (fclose(ifile) != 0)
		printf("Error closing file.\n");
	
	return 0;
}

int main(int argc, char *argv[]) {
	// Variable declarations.
	char *str = NULL;
	int32_t num, i;
	struct stat dir;
	pthread_t *threads;
	
	// Check number of arguments.
	if (argc != 4) {
		printf("usage: indexer <pagedir> <indexnm> <numthreads>\n");
		exit(EXIT_FAILURE);
	}
	
	// Get details of directory.
	if (stat(argv[1], &dir) != 0)
		printf("Failure on stat.\n");

	// Check if directory exists; if not return 1.
	if (S_ISDIR(dir.st_mode) == 0) {
		printf("usage: indexer <pagedir> <indexnm> <numthreads>\n");
		exit(EXIT_FAILURE);
	}

	// Set the directory.
	strcpy(directory, argv[1]);
	
	// Read the number of threads.
	num = strtol(argv[3], &str, 10);
	
	// Check that a valid number was enetered by the user.
	if (num <= 0 || strcmp(str, "\0") != 0) {
		printf("usage: indexer <pagedir> <indexnm> <numthreads>\n");
		exit(EXIT_FAILURE);
	}
	
	// Start off reading the first file.
	nextID = 1;
	
	// Allocate memory for the given number of threads.
	threads = (pthread_t *)malloc(sizeof(pthread_t)*num);

	// Create the specified number of threads.
	for (i = 0; i < num; i++) {
		if(pthread_create(threads + i, NULL, threadFunc, NULL) != 0)
			exit(EXIT_FAILURE);
	}

	// Open a hash table for the index.
	if ((hash = lhopen(10000)) == NULL)
		printf("Failure on opening hash table.\n");

	// Initialise mutex.
	pthread_mutex_init(&m, NULL);

	// Join all threads
	for (i = 0; i < num; i++) {
		if (pthread_join(*(threads + i), NULL) != 0)
			exit(EXIT_FAILURE);
	}

	// Save index.
	if (indexsave(argv[2]) != 0)
		printf("Error saving index.\n");
	
	// Free memory.
	lhapply(hash, freeW);
	lhclose(hash);
	free(threads);
	
	// Destroy mutex.
	pthread_mutex_destroy(&m);

	exit(EXIT_SUCCESS);
}

