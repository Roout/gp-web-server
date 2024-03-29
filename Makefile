CC=gcc
CFLAGS=-std=c17 -Wall -Werror -pedantic -Wextra

all: webserver

webserver: iowrap.o server.o main.c
	$(CC) $(CFLAGS) iowrap.o server.o main.c -g -o webserver

iowrap.o: iowrap.c
	$(CC) $(CFLAGS) -g -c iowrap.c

server.o: server.c
	$(CC) $(CFLAGS) -g -c server.c


clean:
	rm *.o webserver
