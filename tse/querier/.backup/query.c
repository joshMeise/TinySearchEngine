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

int main(void) {
	// Variable declarations.
	char str[20][80], inp[20*80], *beg = NULL, word[50], *jnk, ch;
	int32_t i, j, sum, k, cnt, min;
	bool discard, rank;
	FILE *ifile;
	
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

			// Insert into next position in array.
			i++;

			// Obtain next word from query string.
			beg = strtok(NULL, " \t");
		}

		// Rest this flag to show that no words have been ranked yet in the guven query - set here since discarded qeuries also do not have a rank.
		rank = false;
		if (str[0][0] == '\n')
			discard = true;
		// If not discarding a query, go through words in query string and see how many times they occur in a given index.
		if (discard == false) {
			// If not only a newline character was entered - if not here it prints out null terminator.
			//			if (str[0][0] != '\0') {
				j = 0;
				// Print out words in query string.
				while (j < i) {
					
					// Word less than 3 characters get discarded or is and/or.
					if (strlen(str[j]) >= 3 && strcmp(str[j], "and") != 0 && strcmp(str[j], "or") != 0) {
						if((ifile = fopen("../indexer/Depth0Index", "r")) == NULL)
							printf("File not opened correctly.\n");
						
						while (fscanf(ifile, "%s", word) != EOF) {
							sum = 0;
							if (strcmp(word, str[j]) == 0) {
								printf("%s: ", word);
								
								if (fgetc(ifile) != ' ')
									printf("Invalid character read when trying to read space.\n");
								
								// Read the remainder if the line.
								fscanf(ifile, "%[^\n]", word);
								
								// Grab the first word from the string.
								beg = strtok(word, " ");
								
								// Start at beginning of line.
								k = 0;		
								
								// Obtain words from query tosum up count.
								while (beg != NULL) {
									if (k%2 == 1) {
										// Extract the count.
										cnt = strtol(beg, &jnk, 10);
										
										sum += cnt;
										rank = true;
									}
									
									// Obtain next count.
									beg = strtok(NULL, " ");
									k++;
								}
								
								printf("%d ", cnt);
								
								if (j == 0)
									min = sum;
								else if (sum < min)
									min = sum;
								 
							}
							
							ch = fgetc(ifile);
							
							if (ch != ' ' && ch != '\n')
								printf("Invalid chacter read.\n");
							
						}
						
						if (fclose(ifile) != 0)
							printf("Error in closing the file.\n");
					}
					
					j++;
					//}
				
			}
			if (rank == true)
				printf("-- %d", min);
			else if (str[0][0] != '\0')
				printf("The query did not have any valid words.");

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
