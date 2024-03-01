CFLAGS=-Wall -pedantic -std=c11 -I../utils -L../lib -g
LIBS=-lutils -lcurl

crawler:
				gcc $(CFLAGS) crawler.c $(LIBS) -o $@

prep:
				gcc $(CFLAGS) prep.c $(LIBS) -o $@

str:  
			gcc $(CFLAGS) str.c $(LIBS) -o $@


clean:
			rm -f crawler str
