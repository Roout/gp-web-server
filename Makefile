CC=gcc
CFLAGS=-std=c17 -Wall -Werror -pedantic -Wextra

all: server

server: iowrap.o main.c
	$(CC) $(CFLAGS) iowrap.o main.c -g -o server

iowrap.o: iowrap.c
	$(CC) $(CFLAGS) iowrap.h -g -c iowrap.c

clean:
	rm *.o server
