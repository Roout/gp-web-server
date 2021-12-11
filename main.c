#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <error.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>	// htonl
#include <unistd.h>		// close

#include "iowrap.h"

#define PORT 18000
#define LISTEN_QUEUE_SIZE 10

// client buffer
#define BUFFER_SIZE (1<<16)

void HandleClient(int fd);

int main() {
    printf("Server started at %shttp://127.0.0.1:%d%s\n", "\033[92m", PORT, "\033[0m");

    /* create socket for listening  */
    printf("Create a socket...\n");
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        fprintf(stderr, "Failed to create a socket: %s\n", strerror(errno));
        exit(1);
    }

    // bind socket
    printf("Bind a socket...\n");
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);

	int ec = bind(listen_fd, (struct sockaddr*)(&server_addr), sizeof(server_addr));
	if (ec < 0) {
		fprintf(stderr, "Failed to bind a socket: %s\n", strerror(errno));
		exit(1);
	}

    /* listen socket */
    printf("Listen a socket...\n");
    ec = listen(listen_fd, LISTEN_QUEUE_SIZE);
    if (ec < 0) {
        fprintf(stderr, "Failed to listen a socket %d: %s\n"
            , listen_fd
            , strerror(errno));
        exit(1);
    }
    
    // accept clients
    struct sockaddr_in client_addr;
    while (1) {
		memset(&client_addr, 0, sizeof(client_addr));	
		socklen_t addr_len = sizeof(client_addr);
		int client_fd = accept(listen_fd, (struct sockaddr*)(&client_addr), &addr_len);
		if (client_fd < 0) {
            fprintf(stderr, "Failed to accept a client %d: %s\n"
                , client_fd
                , strerror(errno));
            continue;
		}
        // handle accepted client
        printf("Handle client...\n");
        HandleClient(client_fd);
    }
    return 0;
}

void HandleClient(int fd) {
    char buffer[BUFFER_SIZE + 1];
    char *pattern = "\r\n";

    BufferState state {
        .buffer = buffer,
        .size = 0,
        .capacity = BUFFER_SIZE
    };

    size_t pattern_len = strlen(pattern);
    char *match = NULL;
    while ((match = read_until(fd, &state, pattern)) != NULL) {
        size_t consumed = match - state.buffer;
        *match = '\0';
        printf("read_until: %s\n", state.buffer);
        assert(state.size >= consumed + pattern_len);
        chop_left(&state, consumed + pattern_len);
    }
    if (size > 0) {
        printf("read_until: %s\n", free_buffer);
    }
    (void) close(fd);
}
