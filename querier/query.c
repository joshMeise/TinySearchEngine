/* 
 * query.c --- implements a querier module.
 * 
 * Author: Joshua M. Meise
 * Created: 10-24-2023
 * Version: 1.0
 * 
 * Description: Given a file in 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <indexio.h>
#include <hash.h>
#include <queue.h>
#include <webpage.h>
#include <unistd.h>

// Global variables.
int32_t max;

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

// Structure that contains a document's rank, id and associated URL.
typedef struct doc {
	int rank;
	int docID;
	char URL[500];
} doc_t;

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
 * This function compares the rank of a given document to the rank for whcih we are searching.
 * Inputs: document in queue; rank to sarch for.
 * Outputs: true if found in queue; false if not found in queue.
 */
static bool searchQueueRank(void *elementp, const void *searchkeyp) {
	// Declare variables and coerce to valid datatypes.
	doc_t *data = (doc_t *)elementp;
	int32_t *searchVal = (int32_t *)searchkeyp;
	int32_t comp = data->rank;

	// See if the 2 document IDs are the same.
	if (comp == *searchVal)
		return true;
	else
		return false;
	
}

/*
 * Changes words to lowercase.
 * Discards words that do not contain only letters of the alphabet.
 * Input: word to change
 * Output: 0 for a word that is successfully converted, 1 for a word that needs to be discarded.
 */
static int normalizeWord(char *word) {
	// Variable declarations.
	int32_t i;
		
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

/*
 * This function prints out the data in a queue of documents.
 * Inputs: the document to print.
 * Outputs: none.
 */
static void printQ(void *data) {
	// Declare variable and coerce into valid datatype.
	doc_t *d = (doc_t *)data;

	printf("rank: %d doc: %d URL: %s\n", d->rank, d->docID, d->URL);
}

/*
 * Checks is a query is illegal.
 * Inputs: Words in the query; number of words in query.
 * Outputs: TRUE if query is liiegal, FALSE otehrwise.
 */
static bool invalid(char str[][80], int32_t i) {
	// Variable declarations.
	int32_t j;

	// Check if first word is and/or, last word is and/or.
	if (strcmp(str[0], "and") == 0 || strcmp(str[0], "or") == 0 || strcmp(str[i - 1], "and") == 0 || strcmp(str[i - 1], "or") == 0)
		return true;

	// Check if consecutive and/or.
	for (j = 0; j < i - 1; j++) {
		if (((strcmp(str[j], "and") == 0) && (strcmp(str[j + 1], "or") == 0)) || ((strcmp(str[j], "and") == 0) && (strcmp(str[j + 1], "and") == 0)) || ((strcmp(str[j], "or") == 0) && (strcmp(str[j + 1], "or") == 0)) || ((strcmp(str[j], "or") == 0) && (strcmp(str[j + 1], "and") == 0)))
			return true;
	}
	
	return false;
}

/* 
 * Get tht maximum ranked document in the queue.
 * Inputs: the document from which to extract rank.
 * Outputs: none.
 */
static void getMaxRank(void *data) {
	// Declare variable and coerce into valid datatype.
	doc_t *d = (doc_t *)data;

	// Compare current maximum to the rank of the document passed in; update if necessary.
	if (max < d->rank)
		max = d->rank;

}

/*
 * Rank the words in a query siring/subset of a query string.
 * Inputs: index; query string; end of subset; start of subset; the current file id.
 * Outputs: Rank of a particular word.
 */
int rank(hashtable_t *index, char str[][80], int32_t i, int32_t j, int32_t curID) {
	// Variable declarations.
	wordQ_t *wordQueue;
	docCount_t *dc;
	int32_t cnt, min;
	bool rank;
	
	// Rest this flag to show that no words have been ranked yet in the given subset of the query string.
	rank = false;

	// Initialise the minimum rankt o be 0
	min = 0;
	
	// Word through the query string word by word.
	while (j <= i) {
		// Word less than 3 characters or and/or do nto get a rank.
		if (strlen(str[j]) >= 3 && strcmp(str[j], "and") != 0 && strcmp(str[j], "or") != 0) {
			// Search the index for the given word.
			if ((wordQueue = hsearch(index, search, str[j], strlen(str[j]))) != NULL) {
				// Get number of occurances of word in current document.
				if ((dc = qsearch(wordQueue->qp, searchQueue, (void *)&curID)) != NULL)
					cnt = dc->count;
				else
					cnt = 0;
			}
			else
				cnt = 0;
			
			// Keep track of the minimum rank.
			if (rank == false)
				min = cnt;
			
			if (cnt < min)
				min = cnt;
			
			// At least one word has been ranked.
			rank = true;
		}
		
		// Next word in the query string.
		j++;
		
	}

	return min;
	
}

int main(int argc, char *argv[]) {
	// Variable declarations.
	char str[20][80], inp[20*80], *beg = NULL, fname[60];
	int32_t i, j, min, curID, prevOrP1;
	bool discard, quiet;
	hashtable_t *index;
	queue_t *queueOfDocs;
	doc_t *dp;
	FILE *ifile;

	// Check number of arguments.
	if (argc != 3 && argc != 4) {
		printf("usage: query <pageDirectory> <indexFile> [-q]\n");
		exit(EXIT_FAILURE);
	}

	// Check that the last flag is a -q flag is there are 4 arguments.
	if (argc == 4 && strcmp(argv[3], "-q") != 0) {
		printf("usage: query <pageDirectory> <indexFile> [-q]\n");
		exit(EXIT_FAILURE);
	}

	// Check that pageDirectory exists and is readable and check that the index file exists and is readable.
	if (access(argv[1], R_OK) != 0 || access(argv[2], R_OK) != 0){
		printf("usage: query <pageDirectory> <indexFile> [-q]\n");
		exit(EXIT_FAILURE);
	}
	
	// Set boolean if -q flag is present.
	if (argc == 4)
		quiet = true;
	else
		quiet = false;
	
	// Load in the index.
	if ((index = indexload(argv[2])) == NULL)
		printf("Index not successfully loaded.\n");

	// Print out the first command prompt.
	if (quiet == false)
		printf("> ");
	
	// Ask for user input until such a point as the user enters CTRL+D or EOF.
	while (fgets(inp, 20*80, stdin) != NULL) {
		// Reset discard flag for new query.
		discard = false;

		// Grab words from user input separated by tabs and spaces.
		beg = strtok(inp, " \t");

		// Start inserting at beginning of multidimensional array.
		i = 0;

		// Obtain words from query to insert into multi-dimensional array.
		while (beg != NULL && discard == false) {
			// Place word into array of words.
			strcpy(str[i], beg);

			// If the last word, eliminate the newline character.
			if (str[i][strlen(str[i]) - 1] == '\n')
				// Get rid of last newline character.
				str[i][strlen(str[i]) - 1] = '\0';

			// Start checking letters at first letter.
			j = 0;

			// Normalise word and see if it should be discarded.
			if (normalizeWord(str[i]) != 0)
					discard = true;

			// Increment number of words in query; if statement accounts for tab at the end of the line.
			if (strcmp(beg, "\n") != 0)
				i++;

			// Obtain next word from query string.
			beg = strtok(NULL, " \t");
		}

		// If only a newline character was entered discard the query.
		if (str[0][0] == '\0')
			discard = true;

		// If an invalid query is entered
		if (discard == false)
			discard = invalid(str, i);
		
		// If not discarding a query, go through words in query string and see how many times they occur in a given index.
		if (discard == false) {
			// Print out the words in the query string.
			j = 0;
			while (j < i) {
				printf("%s ", str[j]);
				j++;
			}

			printf("\n");

			// Open the queue of documents for the specific word.
			queueOfDocs = qopen();

			// Start off at the first file.
			curID = 1;
			
			// Initialise file name to be the first id.
			sprintf(fname, "%s/%d", argv[1], curID);

			// Go through all the files in the crawler's pages directory.
			while (access(fname, R_OK) == 0) {
				// For each file reset the counter/indexer variables.
				j = 0;
				prevOrP1 = 0;
				min = 0;

				// Work through the query string.
				while (j < i) {
					// Once an or is reached, rank the words in the substring.
					if (strcmp(str[j], "or") == 0 || j == i - 1) {
						// Use the rank function to rank the words in the substring and then add the rank to the overall rank for the query.
						min += rank(index, str, j, prevOrP1, curID);

						// Update the positon of the previous "or".
						prevOrP1 = j + 1;
					}
					j++;
				}

				// Create a new docmuent.
				dp = (doc_t *)malloc(sizeof(doc_t));

				// Set the id of the new document.
				dp->docID = curID;

				// Read the URL from the crawled webpage and place it into the document.
				if ((ifile = fopen(fname, "r")) != NULL) {
					if (fscanf(ifile, "%s", dp->URL) == 0)
						printf("Reading URL not successful.\n");
					
					if (fclose(ifile) == EOF)
						printf("File not closed successfully.\n");
				}		
				
				// Rank the document.
				dp->rank = min;

				// If the document had an occurance of the word, put it into the queue.
				if (dp->rank != 0) {
					// Put the new document into the queue.
					if (qput(queueOfDocs, (void *)dp) != 0)
						printf("Not successfully put into queue.\n");
				}
				else
					free(dp);

				// Move onto next document.
				curID++;
				
				// Initialise file name to be the first id.
				sprintf(fname, "%s/%d", argv[1], curID);
		
				
			}

			max = 0;
			qapply(queueOfDocs, getMaxRank);

			while (max != 0 && (dp = qremove(queueOfDocs, searchQueueRank, (void *)&max)) != NULL) {
				
				printQ((void *) dp);
				free(dp);
				max =  0;
				qapply(queueOfDocs, getMaxRank);
			}

			// Free memory in current query's queue and close queue.
			qapply(queueOfDocs, free);
			qclose(queueOfDocs);			
		}
		else if (str[0][0] != '\0')
			printf("[invalid query]\n");

		if (quiet == false)
			printf("> ");
	}

	printf("\n");

	// Free memory.
	happly(index, freeW);
	hclose(index);
	
	exit(EXIT_SUCCESS);
}
