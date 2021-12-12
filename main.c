#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <error.h> 
#include <string.h> 

#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h>		// close

#include "iowrap.h"
#include "server.h"

#define HOST "127.0.0.1"
#define PORT "18000"
#define BACKLOG 64

// client buffer
#define BUFFER_SIZE (1<<16)

void handle_client(int fd);

int main() {
    printf("Server started at %shttp://%s:%s%s\n", "\033[92m", HOST, PORT, "\033[0m");

    Server server;
    init_server(&server, HOST, PORT, BACKLOG);
    // accept clients
    while (1) {
			int client_fd = accept_client(&server);
      handle_client(client_fd);
    }
    return 0;
}

typedef struct {
    char *method;   // GET or POST
    char *route;    // /route/to/file
    char *protocol; // HTTP/1.1
} RequestHeader;

/**
 * @param data null-terminated string
 * @param header parsing result on success
 * @return 0 on success otherwise -1
*/
int parse_header(char* data, RequestHeader* header) {
    header->method = strtok(data, " ");
    if (header->method == NULL) {
        return -1;
    }
    header->route = strtok(NULL, " ");
    if (header->route == NULL) {
        return -1;
    }
    header->protocol = strtok(NULL, " ");
    if (header->protocol == NULL) {
        return -1;
    }
    return 0;
}

void handle_client(int fd) {
    // global buffer for this client
    char buffer[BUFFER_SIZE + 1];
    const char *pattern = "\r\n";

    BufferState state = {
        .buffer = buffer,
        .size = 0,
        .capacity = BUFFER_SIZE
    };

    // read header
    char *match = read_until(fd, &state, pattern);
    if (match == NULL) {
        fprintf(stderr, "Failed to read header.\n");
        exit(EXIT_FAILURE);
    }
    // header is a line: [buffer, match)
    *match = '\0';
    char *line = state.buffer;
    // update state
    chop_left(&state, (match - line) + strlen(pattern));
		
    RequestHeader header;
    if (parse_header(line, &header) < 0) {
        fprintf(stderr, "Fail to parse a header\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (!strcmp(header.method, "GET")) {
        while (match != NULL) {
            match = read_until(fd, &state, pattern);
            if (match == NULL) {
                close(fd);
                exit(EXIT_FAILURE);
            }
            if (state.buffer == match) {
                // meet the combination \r\n\r\n
                break;
            }
            char *line = state.buffer;
            chop_left(&state, (match - line) + strlen(pattern));
            const char *field = strtok(line, ": ");
            const char *value = strtok(NULL, "\r\n");
            assert(*match == '\0');
            printf("Parse: %s:%s\n", field, value);
        }
        const char *default_response = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nGET";
        write_some(fd, default_response, strlen(default_response));
    }
    else if (!strcmp(header.method, "POST")) {
        const char *default_response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPOST";
        write_some(fd, default_response, strlen(default_response));
    }
    else {
        const char *default_response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
        write_some(fd, default_response, strlen(default_response));
    }
    
    close(fd);
}
