#ifndef SERVER_H__
#define SERVER_H__

#include "list.h"

#define LISTEN_QUEUE_SIZE 64

typedef struct {
    char *port;
    char *name;
    int fd;
    Node *route;
} Server;

// initialize a server
void InitServer(Server *server, const char* port);

int AcceptClient(Server *server);

#endif // SERVER_H__