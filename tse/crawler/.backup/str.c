/* 
1;95;0c * str.c --- 
 * 
 * Author: Joshua M. Meise
 * Created: 10-14-2023
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <string.h>
#include <hash.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue.h>

void prS(void * data) {
	printf("\n\nIn function: %s\n\n", (char *)data);
}

int main (void) {
	queue_t *q = qopen();
	hashtable_t *h = hopen(10);
	char s[30] = "Hello", p[20] = "Hi", l[10] = "Hello1";
	int success;

	success = qput(q, (void *)s);
	success = qput(q, (void *)p);
	success = qput(q, (void *)s);
	success = hput(h, (void *)s, s, sizeof(s));
		success = hput(h, (void *)p, p, sizeof(p));
			success = hput(h, (void *)l, l, sizeof(l));
	if (success != 0)
		exit(EXIT_FAILURE);
	printf("Q\n");
	
	qapply(q, prS);
	
	printf("H\n");
	happly(h, prS);
	success = 0;
	while(s[success] != '\0'){
		printf("%d ", s[success]);
		success++;
	}
	
	exit(EXIT_SUCCESS);
}
