#ifndef SERVER_H__
#define SERVER_H__

#include "list.h"

#define LISTEN_QUEUE_SIZE 64
#define MAX_HOST_LEN 256

#define _POSIX_C_SOURCE 200112L

typedef struct {
    const char *port;
    char name[MAX_HOST_LEN];
    int fd;
    struct Node *route;
} Server;

// initialize a server
void InitServer(Server *server, const char* port);

int AcceptClient(Server *server);

#endif // SERVER_H__
