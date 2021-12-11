CC=gcc
CFLAGS=-std=c17 -Wall -Werror -pedantic -Wextra

all: server

server: iowrap.o list.o main.c
	$(CC) $(CFLAGS) iowrap.o list.o main.c -g -o server

iowrap.o: iowrap.c
	$(CC) $(CFLAGS) iowrap.h -g -c iowrap.c

list.o: list.c
	$(CC) $(CFLAGS) list.h -g -c list.c

clean:
	rm *.o server
