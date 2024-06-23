/* 
1;95;0c1;95;0c * crawler.c --- prints hello
 * 
 * Author: Joshua M. Meise
 * Created: 10-11-2023
 * Version: 1.0
 * 
 * Description: Prints hello
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <webpage.h>
#include <queue.h>
#include <hash.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

void printURL(void * input) {
	char *p = (char *)input;
	printf("%s\n", p);
}

void printPage(void * input) {
	webpage_t * pg = (webpage_t *)input;
	char *str = webpage_getURL(pg);
	
	printf("%s\n", str);
}

bool search(void *elementp, const void *searchkeyp) {
	char *string = (char *)elementp;
	char *searchString = (char *)searchkeyp;
	
	if (strcmp(string, searchString) == 0)
		return true;
	else
		return false;
	
}

int32_t pagesave(webpage_t *pagep, int id, char *dirname) {
	char name[50], d[50];
	FILE *ifile;
	struct stat dir;
	int32_t success = 1;

	sprintf(d, "../%s", dirname);
	
	stat(d, &dir);

	if (S_ISDIR(dir.st_mode) == 0)
		success = mkdir(d, 0777);

	if (success == -1)
		return 1;
	
	sprintf(name, "../%s/%d", dirname, id);
	
	ifile = fopen(name, "w");

	if (access(name, W_OK) != 0)
		return 1;

	fprintf(ifile, "%s\n", webpage_getURL(pagep));
	fprintf(ifile, "%d\n", webpage_getDepth(pagep));
	fprintf(ifile, "%d\n", webpage_getHTMLlen(pagep));
	fprintf(ifile, "%s\n", webpage_getHTML(pagep));

	fclose(ifile);

	return (0);
	
}

int main(void) {
	webpage_t *page, *newPage;
	bool success, added;
	int pos = 0, successInt;
	char *string;
	queue_t *queue;
	hashtable_t *visited;
	
	page = webpage_new("https://thayer.github.io/engs50/", 0, NULL);

	success = webpage_fetch(page);

	if (success == false)
		exit(EXIT_FAILURE);
 
	
	queue = qopen();
	visited = hopen(10);
	
 	successInt = pagesave(page, 1, "pages");

	if (successInt != 0)
		exit(EXIT_FAILURE);

	//	successInt = hput(visited, (void *) webpage_getURL(page),  webpage_getURL(page), sizeof(webpage_getURL(page)));

	//		if (successInt != 0)
	//			exit(EXIT_FAILURE);

	//						successInt = qput(queue, (void *)page);

	//		if (successInt != 0)
	//					exit(EXIT_FAILURE);
					//
	
 	pos = webpage_getNextURL(page, pos, &string);

	while (pos != -1) {
		added = false;
		if (IsInternalURL(string) == true) {
			
			if (hsearch(visited, search, string, sizeof(string)) == false) {
				successInt = hput(visited, (void *) string, string, sizeof(string));

				if (successInt != 0)
					exit(EXIT_FAILURE);
				
				newPage = webpage_new(string, 1, NULL);
				successInt = qput(queue, (void *)newPage);

				if (successInt != 0)
					exit(EXIT_FAILURE);

				added = true;
			}
			
			printf("Internal ");
		}
		else {
			printf("External ");
		}
		
		printf("URL = %s\n", string);

		if (added == false)
			free(string);
		
		pos = webpage_getNextURL(page, pos, &string);
	}

	printf("\nPrinting the hash table's URLs:\n");
	happly(visited, printURL);
	
	printf("\nPrinting the queue's URLs:\n");
	
	qapply(queue, printPage);
	
	happly(visited, free);
	qapply(queue, webpage_delete);
	qclose(queue);
	hclose(visited);
	
	webpage_delete(page);

	exit(EXIT_SUCCESS);
}
