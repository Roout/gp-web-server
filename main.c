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

typedef struct {
    char *method;   // GET or POST
    char *path;     // /path/to/file
    char *protocol; // HTTP/1.1
} RequestHeader;

/**
 * @param data null-terminated string
 * @param header parsing result on success
 * @return 0 on success otherwise -1
*/
int ParseHeader(char* data, RequestHeader* header) {
    header->method = strtok(data, " ");
    if (header->method == NULL) {
        return -1;
    }
    header->path = strtok(NULL, " ");
    if (header->path == NULL) {
        return -1;
    }
    header->protocol = strtok(NULL, " ");
    if (header->protocol == NULL) {
        return -1;
    }
    return 0;
}

#define CRLF "\r\n"

void HandleClient(int fd) {
    // global buffer for this client
    char buffer[BUFFER_SIZE + 1];
    char *pattern = CRLF;

    BufferState state {
        .buffer = buffer,
        .size = 0,
        .capacity = BUFFER_SIZE
    };

    // read header
    char *match = read_until(fd, state, pattern);
    if (match == NULL) {
        printf(stderr, "Failed to read header.\n");
        exit(1);
    }
    // header is the line: [buffer, match)
    *match = '\0';
    char *line = state->buffer;
    // update state
    chop_left(state, (match - line) + strlen(pattern));

    RequestHeader header;
    if (ParseHeader(line, &header) < 0) {
        // TODO: handle error
        close(fd);
        exit(1);
    }

    if (strcmp(header->method, "GET")) {
        while (match != NULL) {
            match = read_until(fd, state, CRLF);
            if (match == NULL) {
                // TODO: handle error
                close(fd);
                exit(1);
            }
            if (state->buffer == match) {
                // meet the combination CRLF CRLF
                break;
            }
            char *line = state->buffer;
            chop_left(state, (match - line) + strlen(pattern));
            char *field = strtok(line, ": ");
            char *value = strtok(NULL, "\r\n");
            assert(*match == '\0');
            // *match = '\0';
            printf("%s:%s\n", field, value);
        }
        // TODO: check what answer is needed in rfc
        char default_answer[] = "HTTP/1.1 200 OK" CRLF "Content-Length: 0" CRLF CRLF;
        write_some(fd, default_answer, strlen(default_answer));
    }
    else if (strcmp(header->method, "POST")) {
        // TODO: check what answer is needed in rfc
        char default_answer[] = "HTTP/1.1 404 Not Found" CRLF "Content-Length: 0" CRLF CRLF;
        write_some(fd, default_answer, strlen(default_answer));
    }
    else {
        char default_answer[] = "HTTP/1.1 404 Not Found" CRLF "Content-Length: 0" CRLF CRLF;
        write_some(fd, default_answer, strlen(default_answer));
    }
    
    close(fd);
}
