CC=gcc
CFLAGS=-std=c11 -Wall -Werror -pedantic -Wextra

all: server

server: main.c
	$(CC) $(CFLAGS) main.c -g -o server

clean:
	rm *.o server
