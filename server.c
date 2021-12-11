#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

static void RegisterRoute(Server *server, const char* path, const char* file) {
    Insert(&server->route, path, file);
}

// bind all available routes to the existing files
static void RegisterRoutes(Server *server) {
    // add all basic already existing routes
    RegisterRoute(&server, "/", "/res/index.html");
    RegisterRoute(&server, "/index.html", "/res/index.html");
    RegisterRoute(&server, "/about", "/res/about.txt");
    RegisterRoute(&server, "/test/other/route", "/res/test/other/route.txt");
    RegisterRoute(&server, "/test/other/test", "/res/test/other/test.txt");
}

void InitServer(Server *server, const char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    struct addrinfo *result;
    int ec = getaddrinfo(NULL, port, &hints, &result);
    if (ec != 0) {
        fprintf(stderr, "[ERROR] getaddrinfo: %s\n", gai_strerror(ec));
        exit(EXIT_FAILURE);
    }

    // getaddrinfo() returns a list of address structures.
    // Try each address until we successfully bind(2).
    // If socket(2) (or bind(2)) fails, we close the socket and try the next address. 
    struct addrinfo *rp = NULL;
    server->fd = -1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        server->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server->fd < 0) {
            continue;
        }
        if (bind(server->fd, rp->ai_addr, rp->ai_addrlen) == 0) { 
            // Success 
            printf("Create a socket...\n");
            printf("Bind a socket...\n");
            break;
        }
        // otherwise fail to bind -> try next
        close(server->fd);
    }

    // As the server won't be shutted down this allocation won't be freed
    server->port = (char *) malloc(strlen(port) + 1);
    server->name = (char *) malloc(strlen(rp->ai_canonname) + 1);
    if (!server->port || !server->name) {
        fprintf(stderr, "[ERROR] Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(server->port, port);
    strcpy(server->name, rp->ai_canonname);

    // No longer needed
    freeaddrinfo(result); 

    if (rp == NULL) { 
        // No address succeeded
        fprintf(stderr, "[ERROR] Could not bind\n");
        exit(EXIT_FAILURE);
    }

    printf("Server started at %s%s:%s%s\n", "\033[92m", server->name, server->port, "\033[0m");

    // listen socket
    printf("Listen a socket...\n");
    ec = listen(server->fd, LISTEN_QUEUE_SIZE);
    if (ec < 0) {
        fprintf(stderr, "[ERROR] Failed to listen a socket %d: %s\n"
            , server->fd 
            , strerror(errno));
        exit(EXIT_FAILURE);
    }
    RegisterRoutes(server);
}

int AcceptClient(Server *server) {
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
        return client_id;
    }
    assert(false && "Unreachable");
    return -1;
}