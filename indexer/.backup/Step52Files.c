/* 
 * indexer.c --- 
 * 
 * Author: Joshua M. Meise
 * Created: 10-18-2023
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <webpage.h>
#include <pageio.h>
#include <string.h>
#include <ctype.h>
#include <hash.h>
#include <queue.h>

// Golbal variables.
static int32_t sum;

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
		if (isalpha(word[i] != 0))
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

static void printQ(void *data) {
	docCount_t *d = (docCount_t *)data;
	printf("DocID %d, %d times.\n", d->doc, d->count);
}

static void printWordH(void *data) {
	wordQ_t *wrd = (wordQ_t *)data;
	printf("The word %s occurs in:\n", wrd->word);
	qapply(wrd->qp, printQ);
	printf("\n");
}


static void sumWordsQ(void *data) {
	docCount_t *d = (docCount_t *)data;
	sum += d->count;
}

static void sumWordsH(void *data) {
	wordQ_t *wrd = (wordQ_t *)data;
	qapply(wrd->qp, sumWordsQ);
}

int main(int argc, char *argv[]) {
	// Variable declarations.
	webpage_t *pageLoad;
	char *word, name[50], readWord[50], *str = NULL;
	int32_t pos, docID, curID;
	FILE *ifile;
	wordQ_t *wordQueue;
	hashtable_t *hash;
	docCount_t *dc;

	// Extract the document ID.
	docID = strtol(argv[1], &str, 10);

	// Open a hash table for the index.
	hash = hopen(1000);
	
	for (curID = 1; curID <= docID; curID++) {
		// Load webpage at given index.
		pageLoad = pageload(curID, "../crawler/page");

		// Initialise pos to be 0.
		pos = 0;

		// Create the file name for the current document ID.
		if (sprintf(name, "outputFiles/%d", curID) < 0)
			printf("File name creation failed.\n");
	
		// Open the file for writing the first time to clear file from previously.
		//if (curID == 1)
		ifile = fopen(name, "w");
		//else
		//ifile = fopen(name, "a");

		// Return 1 if file not opened for writing properly.
		if (access(name, W_OK) != 0)
			exit(EXIT_FAILURE);

		// Get word from html and print word to screen.
		while ((pos = webpage_getNextWord(pageLoad, pos, &word)) > 0) {
			// Only print the word to the file if it can be normalized
			if (normalizeWord(word) == 0)
				fprintf(ifile, "%s\n", word);

			// Free memory allocated to word.
			free(word);
		}
	
		// Close file.
		fclose(ifile);

		// Delete the webpage.
		webpage_delete(pageLoad);
		
		// Open the file for reading.
		ifile = fopen(name, "r");
	
		// Exit if file not opened for reading properly.
		if (access(name, R_OK) != 0)
			exit(EXIT_FAILURE);

		while ((fscanf(ifile, "%s", readWord)) != EOF) {
			// If first occurance of word.
			if (hsearch(hash, search, readWord, strlen(readWord)) == NULL) {
				// Create a new word structure.
				wordQueue = (wordQ_t *)malloc(sizeof(wordQ_t));
			
				// Allocate space for word.
				wordQueue->word = (char *)malloc(strlen(readWord)*sizeof(char) + 1);
			
				// Initialise word.
				strcpy(wordQueue->word, readWord);

				// Open a queue associated with the word.
				wordQueue->qp = qopen();

				// Create a new document/count pair.
				dc = (docCount_t *)malloc(sizeof(docCount_t));

				// Initialise this new document/count pair.
				dc->doc = curID;
				dc->count = 1;

				// Put document/count pair into queue.
				if (qput(wordQueue->qp, (void *)dc) != 0)
					printf("Problem putting curID %d for word %s into queue.\n", curID, wordQueue->word);
			
				// Put word int hash table.
				if (hput(hash, wordQueue, wordQueue->word, strlen(wordQueue->word)) != 0)
					printf("Unsuccessful put into hash word: %s.\n", wordQueue->word);
			}
			// If word is already in hash table.
			else {
				// Find the word in the hash table.
				wordQueue = hsearch(hash, search, readWord, strlen(readWord));
			
				// Find the element in the word's queue associated with the current IDID.
				// If the document ID is not already in the queue.
				if ((dc = (docCount_t *)qsearch(wordQueue->qp, searchQueue, (void *)&curID)) == NULL) {
					// Create a new document/count pair.
					dc = (docCount_t *)malloc(sizeof(docCount_t));

					// Initialise this new document/count pair.
					dc->doc = curID;
					dc->count = 1;

					// Put document/count pair into queue.
					if (qput(wordQueue->qp, (void *)dc) != 0)
						printf("Problem putting docID %d for word %s into queue.\n", docID, wordQueue->word);
				}
				// If the document ID is already in the queue.
				else {
					// Increase the number of occurances in the specified document.
					(dc->count)++;
				}
			}
		
			// Read in newline character
			if (fgetc(ifile) != '\n')
				printf("Reading an actual character.\n");

		}
	
		fclose(ifile);
	}
	
	printf("Printing hash table:\n");
	happly(hash, printWordH);
	
	// Sum all of the words up.
	sum = 0;
	happly(hash, sumWordsH);
	printf("The sum of the words is %d.\n", sum);
	
	// Free memory.
	happly(hash, freeW);
	printf("TELS");
	hclose(hash);
	
	exit(EXIT_SUCCESS);
}
