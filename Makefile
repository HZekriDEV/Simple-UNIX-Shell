CC=gcc
CFLAGS=-Wall -Werror -g

sh: sh.o
	$(CC) $(CFLAGS) -o sh sh.o

sh.o: sh.c
	$(CC) $(CFLAGS) -c sh.c

clean:
	rm -f *.o sh