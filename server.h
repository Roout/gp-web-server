#ifndef SERVER_H__
#define SERVER_H__

#include "list.h"

// Feature Test Macro Requirements for glibc
// getnameinfo():
//            Since glibc 2.22:
//                _POSIX_C_SOURCE >= 200112L
// see (https://man7.org/linux/man-pages/man3/getnameinfo.3.html)
#define _POSIX_C_SOURCE 200112L

typedef struct {
    int fd;
    struct Node *route;
} Server;

// initialize a server
void init_server(Server *server
    , const char* host
    , const char* port
    , const int backlog);

int accept_client(Server *server);

void register_route(Server *server, const char* route, const char* file);

const char * get_file(Server *server, const char* route);

#endif // SERVER_H__
