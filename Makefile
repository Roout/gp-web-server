CC=gcc
CFLAGS=-std=c17 -Wall -Werror -pedantic -Wextra

all: webserver

webserver: iowrap.o list.o server.o main.c
	$(CC) $(CFLAGS) iowrap.o list.o server.o main.c -g -o webserver

iowrap.o: iowrap.c
	$(CC) $(CFLAGS) iowrap.h -g -c iowrap.c

server.o: server.c list.o
	$(CC) $(CFLAGS) server.c list.o server.h -g -c server.c

list.o: list.c
	$(CC) $(CFLAGS) list.h -g -c list.c

clean:
	rm *.o server
