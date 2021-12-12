#ifndef SERVER_H__
#define SERVER_H__

// Feature Test Macro Requirements for glibc
// getnameinfo():
//            Since glibc 2.22:
//                _POSIX_C_SOURCE >= 200112L
// see (https://man7.org/linux/man-pages/man3/getnameinfo.3.html)
#define _POSIX_C_SOURCE 200112L

typedef struct {
    int fd;
} Server;

/**
 * Initialize a server
 * @param host is a hostname used by server. Host and port will be resolved by getaddrinfo
 * @param port is a port used by server
 * @
 */
void init_server(Server *server
    , const char* host
    , const char* port
    , const int backlog);

int accept_client(Server *server);

#endif // SERVER_H__
