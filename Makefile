CC=gcc
CFLAGS=-std=c11 -Wall -Werror -pedantic -Wextra

all: server

server: iowrap.o main.c
	$(CC) $(CFLAGS) iowrap.o main.c -g -o server

iowrap.o: iowrap.c
	$(CC) $(CFLAGS) iowrap.h iowrap.c -g -c -o iowrap.o

clean:
	rm *.o server
