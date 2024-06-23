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

// Structure that contains a word and the number of words.
typedef struct normWord {
	char *word;
	int numTimes;
} normWord_t;

// Golbal variables.
static int32_t sum;

/*
 * This function compares a URL of a given page to one in the hash table.
 * Inputs: URL to search for; URL in hash table.
 * Outputs: true if found in hash table; false if not found in hash table.
 */
static bool search(void *elementp, const void *searchkeyp) {
	// Declare variables and coerce to valid datatypes.
	normWord_t *data = (normWord_t *)elementp;
	char *searchString = (char *)searchkeyp;
	char *string = data->word;

	// See if URL passed in is same as relevant URL in hash table.
	if (strcmp(string, searchString) == 0)
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
static void freeWord(void *data) {
	// Coerce to vaild datatypes.
	normWord_t *wrd = (normWord_t *)data;
	char *str = wrd->word;

	// Free the memory that was allocated.
	free(str);
	free(wrd);
}

static void printWord(void *data) {
	normWord_t *wrd = (normWord_t *)data;
	printf("The word %s occurs %d times.\n", wrd->word, wrd->numTimes);
}

static void sumWords(void *data) {
	normWord_t *wrd = (normWord_t *)data;
	sum += wrd->numTimes;
}

int main(void) {
	// Variable declarations.
	webpage_t *pageLoad;
	char *word, name[] = "outputFile", readWord[50];
	int32_t pos;
	FILE *ifile;
	normWord_t *wordStr;
	hashtable_t *hash;
	
	// Load webpage index 1.
	pageLoad = pageload(1, "../pages");

	// Initialise pos to be 0.
	pos = 0;
	
	// Open the file for writing.
	ifile = fopen(name, "w");

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

	// Open the file for reading.
	ifile = fopen(name, "r");
	
	// Exit if file not opened for reading properly.
	if (access(name, R_OK) != 0)
		exit(EXIT_FAILURE);

	// Open a hash table for the index.
	hash = hopen(100);

	while ((fscanf(ifile, "%s", readWord)) != EOF) {
		// If first occurance of word.
		if (hsearch(hash, search, readWord, strlen(readWord)) == NULL) {
			// Create a new word structure.
			wordStr = (normWord_t *)malloc(sizeof(normWord_t));
			
			// Allocate space for word.
			wordStr->word = (char *)malloc(strlen(readWord)*sizeof(char) + 1);

			// Initialise word and number of occurances.
			strcpy(wordStr->word, readWord);
			wordStr->numTimes = 1;

			// Put word int hash table.
			if (hput(hash, wordStr, wordStr->word, strlen(wordStr->word)) != 0)
				printf("Unsuccessful put into hash word: %s.\n", wordStr->word);
		}
		// If word is already on hash table.
		else {
			// Find the word in the hash table.
			wordStr = hsearch(hash, search, readWord, strlen(readWord));

			// Increase the number of occurances.
			(wordStr->numTimes)++;
		}
		
		// Read in newline character
		if (fgetc(ifile) != '\n')
			printf("Reading an actual character.\n");

	}

	fclose(ifile);

	printf("Printing hash table:\n");
	happly(hash, printWord);

	// Sum all of the words up.
	sum = 0;
	happly(hash, sumWords);
	printf("The sum of the words is %d.\n", sum);
	
	// Free memory.
	happly(hash, freeWord);
	hclose(hash);
	webpage_delete(pageLoad);
	
	exit(EXIT_SUCCESS);
}
