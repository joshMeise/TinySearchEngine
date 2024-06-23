/* 
 * crawler.c --- implements a crawler though a website
 * 
 * Author: Joshua M. Meise
 * Created: 10-15-2023
 * Version: 1.0
 * 
 * Description: Takes in a seed URL and directory name and crawls the website at the seed URL to a given depth.
 * 
 */

// Libraries to include.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <webpage.h>
#include <queue.h>
#include <hash.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pageio.h>

/*
 * This function compares a URL of a given page to one in the hash table.
 * Inputs: URL to search for; URL in hash table.
 * Outputs: true if found in hash table; false if not found in hash table.
 */
bool search(void *elementp, const void *searchkeyp) {
	// Declare variables and coerce to strings.
	char *string = (char *)elementp;
	char *searchString = (char *)searchkeyp;

	// See if URL passed in is same as relevant URL in hash table.
	if (strcmp(string, searchString) == 0)
		return true;
	else
		return false;
	
}

int main(int argc, char *argv[]) {
	// Variable declarations.
	int32_t maxDepth, docID, curDepth, pos;
	char *str = NULL, *URL;
	queue_t *q;
	hashtable_t *h;
	webpage_t *page, *newPage;
	bool added;

	// Check number of arguments.
	if (argc != 4) {
		printf("usage: crawler <seedurl> <pagedir> <maxdepth>\n");
		exit(EXIT_FAILURE);
	}

	// Extract the maximum depth.
	maxDepth = strtol(argv[3], &str, 10);

	// Check that maximum depth is positive and is an integer.
	if (maxDepth < 0 || strcmp(str, "\0") != 0) {
 		printf("usage: crawler <seedurl> <pagedir> <maxdepth>\n");
		exit(EXIT_FAILURE);
	}

	// Initialise current depth to be 0 and insert webpage associated with seed at depth 0;
	curDepth = 0;
	page = webpage_new(argv[1], curDepth, NULL);

	// Open a queue to store webpages adn a has table to store pages already visited.
	q = qopen();
	h = hopen(1000);

	// Put the first webpage into the queue.
	if (qput(q, (void *)page) != 0)
		exit(EXIT_FAILURE);

	// Allocate memory for the first webpage's URL and store it in a string.
	URL = (char *)malloc(strlen(webpage_getURL(page))*sizeof(char) + 1);
	strcpy(URL, webpage_getURL(page));

	// Put the first URL into the hash table.
	if (hput(h, (void *)URL, URL, sizeof(URL)) != 0)
		exit(EXIT_FAILURE);

	// Get the webpage out of the queue again.
	page = (webpage_t *)qget(q);

	// Set doc ID so that the first page to save saves in a fil with name "1".
	docID = 1;

	// Keep writing webpages until reaching maximum depth.
	while (curDepth <= maxDepth) {
		// Fetch the html for a given webpage; if failure move on to next webpage.
		while (webpage_fetch(page) == false) {
			// Delete current qebpage that cannot be fetched.
			webpage_delete(page);

			// Get next webpage.
			page = (webpage_t *)qget(q);

			// Update the current depth to new webpage's current depth.
			curDepth = webpage_getDepth(page);
		}

		// Save the webpage.
		if (pagesave(page, docID, argv[2]) != 0)
			exit(EXIT_FAILURE);

		// Increase the docID so that the file name changes accordingly
		docID++;

		// Get the next URL from the current page.
		pos = webpage_getNextURL(page, 0, &str);

		// Obtain all links from page.				
		while (pos != -1) {
			added = false;
			// Only work with internal URLs.
			if (IsInternalURL(str) == true) {
				// See if URL is already in hash table; if not then add to hash table and queue.
				if (hsearch(h, search, str, sizeof(str)) == false) {
					// Put URL into hash table.
					if (hput(h, (void *) str, str, sizeof(str)) != 0)
						exit(EXIT_FAILURE);

					// Create a new webpage to put into queue.
					newPage = webpage_new(str, webpage_getDepth(page) + 1, NULL);

					// Put new webpage into queue.
					if (qput(q, (void *)newPage) != 0)
							exit(EXIT_FAILURE);

					added = true;
				}
			}

			// Free string if not added to hash table.
			if (added == false)
				free(str);

			// Move onto next URL.
			pos = webpage_getNextURL(page, pos, &str);
 		}

		// Free memory from current page.
 		webpage_delete(page);

		// Get next page from queue.
		page = (webpage_t *)qget(q);

		// Update depth to be current page's depth.
		curDepth = webpage_getDepth(page);
	}

	// Free all memory.
	webpage_delete((void *)page);
 	happly(h, free);
	qapply(q, webpage_delete);
	qclose(q);
	hclose(h);
	
	exit(EXIT_SUCCESS);

}
