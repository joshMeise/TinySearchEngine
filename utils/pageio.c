/* 
 * pageio.c --- implements pageio.h interface
 * 
 * Author: Joshua M. Meise
 * Created: 10-17-2023
 * Version: 1.0
 * 
 * Description: This module implements the pageio.h interface.
 * 
 */

#include <pageio.h>
#include <sys/stat.h>
#include <stdio.h>

/*
 * This function saed the details of a webpage to a file.
 * Inputs: Webpage to save, file ID, directory to save.
 * Output: Zero for success, non-zero otherwise.
 */
int32_t pagesave(webpage_t *pagep, int id, char *dirname) {
	// Variable declarations.
	char name[50];
	FILE *ifile;
	struct stat dir;

	// Get details of directory.
	stat(dirname, &dir);

	// Check if directory exists; if not create one; if creation fails, return 1.
	if (S_ISDIR(dir.st_mode) == 0)
		if (mkdir(dirname, 0777) == -1)
			return 1;

	// Create the file name in the specified directory.
	sprintf(name, "%s/%d", dirname, id);

	// Open the file for writing.
	ifile = fopen(name, "w");

	// Return 1 if file not opened for writing properly.
	if (access(name, W_OK) != 0)
		return 1;

	// Print relevant detatils to file.
	fprintf(ifile, "%s\n", webpage_getURL(pagep));
	fprintf(ifile, "%d\n", webpage_getDepth(pagep));
	fprintf(ifile, "%d\n", webpage_getHTMLlen(pagep));
	fprintf(ifile, "%s", webpage_getHTML(pagep));

	// Close file.
	fclose(ifile);

	// Return 0 for success.
	return (0);
	
}

/* 
 * pageload -- loads the numbered filename <id> in direcory <dirnm>
 * into a new webpage
 * inputs: id of file to load and directory in which file lives
 * returns: non-NULL for success; NULL otherwise
 */
webpage_t *pageload(int id, char *dirnm) {
	// Variable declarations.
	webpage_t *newPage;
	FILE *ifile;
	struct stat dir;
	char name[50], URL[500], *html, ch;
	int32_t depth, htmlLen, i;
	
	// Get details of directory.
	stat(dirnm, &dir);

	// Check if directory exists; if not return NULL
	if (S_ISDIR(dir.st_mode) == 0)
		return NULL;

	// Create the file name in the specified directory.
	sprintf(name, "%s/%d", dirnm, id);

	// Open the file for reading.
	ifile = fopen(name, "r");

	// Return NULL if file not opened for reading properly.
	if (access(name, R_OK) != 0)
		return NULL;

	// Read the URL from the file by reading to the end of the first line.
	fscanf(ifile, "%[^\n]", URL);

	// Read in the newline character.
	fgetc(ifile);

	// Read in an integer for the depth.
	fscanf(ifile, "%d", &depth);

	// Read in the newline character.
	fgetc(ifile);
	
	// Read in html length.
	fscanf(ifile, "%d", &htmlLen);

	// Read in the newline character.
	fgetc(ifile);	

	// Allocate memory for html string.
	html = (char *)malloc(htmlLen*sizeof(char) + sizeof(char));

	// Start inserting into html at position 0.
	i = 0;
	
	// Read the file character by character and place them into html.
	while ((ch = fgetc(ifile)) != EOF) {
		html[i] = ch;
		i++;
	}

	// Make last character null terminator.
	html[i] = '\0';

	// Close the file.
	fclose(ifile);

	// Create a webpage at the URL read.
	newPage = webpage_new(URL, depth, html);

	// Return the new page.
	return newPage;
}
