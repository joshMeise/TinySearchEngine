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
	fprintf(ifile, "%s\n", webpage_getHTML(pagep));

	// Close file.
	fclose(ifile);

	// Return 0 for success.
	return (0);
	
}
