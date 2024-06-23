CFLAGS=-Wall -pedantic -std=c11 -I../utils -L../lib -g
LIBS=-lutils -lcurl

indexer:
				gcc $(CFLAGS) indexer.c $(LIBS) -o $@

Step5OneFile:
				gcc $(CFLAGS) Step5OneFile.c $(LIBS) -o $@

Step52Files:
				gcc $(CFLAGS) Step52Files.c $(LIBS) -o $@

Step5a:
				gcc $(CFLAGS) Step5a.c $(LIBS) -o $@

clean:
			rm -rf indexer *.o *.dSYM
