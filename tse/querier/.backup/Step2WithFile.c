/* 
 * query.c --- 
 * 
 * Author: Joshua M. Meise
 * Created: 10-24-2023
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/*
 * Changes words to lowercase.
 * Discards words that are less than 3 characters or that contain non-alphanumeric characters.
 * Input: word to change
 * Output: 0 for a word that is successfully converted, 1 for a word that needs to be discarded.
 */
static int normalizeWord(char *word) {
	// Variable declarations.
	int32_t i;
	/*
	// Word less than 3 characters get discarded.
	if (strlen(word) < 3)
		return 1;
	*/
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

int main(void) {
	// Variable declarations.
	char str[20][80], inp[20*80], *beg = NULL, word[50];
	int32_t i, j, sum;
	bool discard;
	//	hashtable_t *index;

	// Load in the index at depth 0.
	//	index = indexload("../indexer/Depth0Index");
	
	// Print out the first commandy prompt.
	printf("> ");
	
	// Ask for user input until such a point as the user enters CTRL+D or EOF.
	while (fgets(inp, 20*80, stdin) != NULL) {

		// Reset discard parameter.
		discard = false;

		// Grab words from user input separated by tabs and spaces.
		beg = strtok(inp, " \t");

		// Start inserting at beginning of multidimensional array.
		i = 0;		

		// Obtain words from query to insert into multi-dimensional array.
		while (beg != NULL && discard == false) {
			// Place qord into array of words.
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

			// Move onto next word.
			i++;

			// Obtain next word from query string.
			beg = strtok(NULL, " \t");
		}

		// If not discarding a query.
		if (discard == false) {
			// If not only a newline character was entered - if not here it prints out null terminator.
			if (str[0][0] != '\0') {
				j = 0;
				// Print out words in query string.
				while (j < i) {
					if((ifile = fopen("../indexer/Depth0Index", "r")) == NULL)
						printf("File not opened correctly.\n");

					while (fscanf(ifile, "%s", word) != EOF) {
						sum = 0;
						if (strcmp(word, str[j]) == 0) {
							printf("%s ", word);

							if (fgetc(ifile) != ' ')
								printf("Invalid character read when trying to read space.\n");
							
							while (fscanf
					j++;
				}
			}
		}
		else
			printf("[invalid query]");
		
		
		// Print a newline only if the user did not only enter a newline character.
		if (str[0][0] != '\0')
			printf("\n");
		
		printf("> ");
	}

	printf("\n");
	
	exit(EXIT_SUCCESS);
}
