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

int main(void) {
	// Variable declarations.
	char str[20][80], inp[20*80], *beg = NULL;
	int32_t i, j;
	bool discard;

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

			// Check to see if letter and make uppercase letters lowercase.
			while (j < strlen(str[i]) && discard == false) {
				// Discard if non-alphabetical.
				if (isalpha(str[i][j]) == 0)
					discard = true;
			
				if (str[i][j] >= 'A' && str[i][j] <= 'Z')
					str[i][j] = tolower(str[i][j]);

				j++;
			}

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
					printf("%s ", str[j]);
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
	

	exit(EXIT_SUCCESS);
}
