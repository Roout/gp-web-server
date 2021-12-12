#include "server.h"
#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <netdb.h>

// bind all available routes to the existing files
static void register_routes(Server *server) {
    // add all basic already existing routes
    register_route(server, "/", "res/index.html");
    register_route(server, "/index.html", "res/index.html");
    register_route(server, "/about", "res/about.txt");
    register_route(server, "/test/other/route", "res/test/other/route.txt");
    register_route(server, "/test/other/test", "res/test/other/test.html");
}

void init_server(Server *server
    , const char* host
    , const char* port
    , const int backlog) 
{
		assert(server);
		memset(server, 0, sizeof(*server)); 

    const char *node = NULL;
    if (!host || !*host || !strcmp(host, "*")) {
        node = NULL;
    }
    else {
        node = host;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 and IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    struct addrinfo *list;
    int ec = getaddrinfo(node, port, &hints, &list);
    if (ec != 0) {
        fprintf(stderr, "[ERROR] getaddrinfo: %s\n", gai_strerror(ec));
        exit(EXIT_FAILURE);
    }

    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully bind(2).
    // If socket(2) (or bind(2)) fails, we close the socket and try the next address. 
    struct addrinfo *curr = NULL;
    int fd = -1;
    for (curr = list; curr != NULL; curr = curr->ai_next) {
        fd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
        if (fd == -1) {
            continue;
        }
        
        if (bind(fd, curr->ai_addr, curr->ai_addrlen) == -1) { 
            close(fd);
            fd = -1;
            continue;
        }

        // set reuse address option to prevent 
        // failure on bind for different IP but same PORT addresses
        int reuse = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
            close(fd);
            fd = -1;
            continue;
        }

        if (listen(fd, backlog) == -1) {
            close(fd);
            fd = -1;
            continue;
        }
        // success on listening
        break;
    }
    freeaddrinfo(list); 

    if (fd == -1) { 
        // No address succeeded
        fprintf(stderr, "[ERROR] Could not create/bind/listen socket\n");
        exit(EXIT_FAILURE);
    }

    server->fd = fd;

    // add routes to server resources
    register_routes(server);
}

int accept_client(Server *server) {
    // accept clients
    struct sockaddr_in client_addr;
    while (1) {
        memset(&client_addr, 0, sizeof(client_addr));	
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server->fd, (struct sockaddr*)(&client_addr), &addr_len);
        if (client_fd < 0) {
            fprintf(stderr, "Failed to accept a client %d: %s\n"
                , client_fd 
                , strerror(errno));
            // try to accept next
            continue;
        }
        return client_fd;
    }
    assert(0 && "Unreachable");
    return -1;
}

void register_route(Server *server, const char* path, const char* file) {
    printf("register route %s to file: %s\n", path, file); 
    insert_list(&server->route, path, file);
}

const char * get_file(Server *server, const char* route) {
    struct Node *node = find_route(server->route, route);
    if (node == NULL) {
        return NULL;
    }
    return node->file;
}

