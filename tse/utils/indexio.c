/* 
 * indexio.c --- Module that implements the interface in indexio.h.
 * 
 * Author: Joshua M. Meise
 * Created: 10-21-2023
 * Version: 1.0
 * 
 * Description: Contains functions for saving an index to a file and reading an index from a file.
 * 
 */

// 

#include <indexio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Global variables.
FILE *ifile;

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


static void printFileQ(void *data) {
	docCount_t *d = (docCount_t *)data;
	fprintf(ifile, " %d %d", d->doc, d->count);
}

static void printFileH(void *data) {
	wordQ_t *wrd = (wordQ_t *)data;
	fprintf(ifile, "%s", wrd->word);
	qapply(wrd->qp, printFileQ);
	fprintf(ifile, "\n");
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
 * Function to save an index to a file.
 * Inputs: Index to save, name of file to save to.
 * Outputs: 0 for success; non-zero for failure.
 */
int32_t indexsave(hashtable_t *index, char *indexnm) {
	// Check that hash and indexnm exist.
	if (index == NULL || indexnm == NULL)
		return 1;
	
	// Open the file for writing.
	ifile = fopen(indexnm, "w");

	// Return 1 if file not opened for writing properly.
	if (access(indexnm, W_OK) != 0)
		return 1;

	// Write to the file using happly();
	happly(index, printFileH);

	// Close reading file after loop has completed execution.
	if (fclose(ifile) != 0)
		printf("Error closing file.\n");
	
	return 0;
}

/*
 * Function to load an index from a file.
 * Inputs: File name to load from.
 * Outputs: Hash table cotaining index, NULL if failure
 */
hashtable_t *indexload(char *indexnm) {
	// Variable declarations.
	char readWord[50], *str;
	int32_t i, curID, cnt;
	wordQ_t *wordQueue;
	docCount_t *dc;
	char ch;
	hashtable_t *index;

	// Return NULL if no file name passed in.
	if (indexnm == NULL)
		return NULL;
	
	// Open the file for reading, return NULL if not opened.
	if ((ifile = fopen(indexnm, "r")) == NULL)
		return NULL;
		
	// Return NULL if file not accessible for reading.
	if (access(indexnm, R_OK) != 0)
		return NULL;

	// Open a new hash table.
	if ((index = hopen(1000)) == NULL)
		printf("Hash table not opened properly.\n");

	// First going to read in a word to be added to hash table.
	i = 0;

	// Read through file, word by word.
	while (fscanf(ifile, "%s", readWord) != EOF) {
		// If the word is the first word in a line.
		if (i == 0) {
			// Create a new word structure.
			if ((wordQueue = (wordQ_t *)malloc(sizeof(wordQ_t))) == NULL)
				printf("Error in malloc.\n");

			// Allocate space for word.
			if ((wordQueue->word = (char *)malloc(strlen(readWord)*sizeof(char) + 1)) == NULL)
				printf("Error in malloc.\n");
			
			// Initialise word.
			strcpy(wordQueue->word, readWord);

			// Open a queue associated with the word.
			if ((wordQueue->qp = qopen()) == NULL)
				printf("Queue not opened correctly.\n");

			// Put word int hash table.
			if (hput(index, wordQueue, wordQueue->word, strlen(wordQueue->word)) != 0)
				printf("Unsuccessful put into hash word: %s.\n", wordQueue->word);

		}
		// If word being read is a document ID.	
		else if (i%2 == 1) {
			// Create a new document/count pair.
			if ((dc = (docCount_t *)malloc(sizeof(docCount_t))) == NULL)
				printf("Error on malloc.\n");

			// Extract the document ID.
			curID = strtol(readWord, &str, 10);
			
			// Initialise this new document/count pair.
			dc->doc = curID;

			// Put document/count pair into queue.
			if (qput(wordQueue->qp, (void *)dc) != 0)
				printf("Problem putting docID %d for word %s into queue.\n", curID, wordQueue->word);

		}
		// If word being read is a number of occurances. 
		else {
			// Find corresponding document in queue.
			dc = (docCount_t *)qsearch(wordQueue->qp, searchQueue, (void *)&curID);

			// Extract the count.
			cnt = strtol(readWord, &str, 10);

			// Put associated count into queue.
			dc->count = cnt;
		}

		// Read in space/newline character.
		ch = getc(ifile);

		// If space increment i, if newline, set i back to 0.
		if (ch == ' ')
			i++;
		else if (ch == '\n')
			i = 0;
		else
			printf("Incorrect character read.\n");
			
	}
		

	// Close reading file.
	if (fclose(ifile) != 0)
		printf("Error closing file.\n");

	return index;

}
