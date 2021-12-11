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
#include "server.h"

#define PORT "18000"
#define CRLF "\r\n"

// client buffer
#define BUFFER_SIZE (1<<16)

void HandleClient(int fd);

int main() {
    Server server;
    InitServer(&server, PORT);
    // accept clients
    while (1) {
		int client_fd = AcceptClient();
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

void HandleClient(int fd) {
    // global buffer for this client
    char buffer[BUFFER_SIZE + 1];
    char *pattern = CRLF;

    BufferState state = {
        .buffer = buffer,
        .size = 0,
        .capacity = BUFFER_SIZE
    };

    // read header
    char *match = read_until(fd, &state, pattern);
    if (match == NULL) {
        fprintf(stderr, "Failed to read header.\n");
        exit(1);
    }
    // header is the line: [buffer, match)
    *match = '\0';
    char *line = state.buffer;
    // update state
    chop_left(&state, (match - line) + strlen(pattern));
		
    RequestHeader header;
    if (ParseHeader(line, &header) < 0) {
        // TODO: handle error
        fprintf(stderr, "Fail to parse a header\n");
        close(fd);
        exit(1);
    }

    if (!strcmp(header.method, "GET")) {
        while (match != NULL) {
            match = read_until(fd, &state, CRLF);
            if (match == NULL) {
                // TODO: handle error
                close(fd);
                exit(1);
            }
            if (state.buffer == match) {
                // meet the combination CRLF CRLF
                break;
            }
            char *line = state.buffer;
            chop_left(&state, (match - line) + strlen(pattern));
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
    else if (!strcmp(header.method, "POST")) {
        // TODO: check what answer is needed in rfc
        char default_answer[] = "HTTP/1.1 404 Not Found" CRLF "Content-Length: 0" CRLF CRLF;
        printf("Send: %s\n", default_answer); 
        write_some(fd, default_answer, strlen(default_answer));
    }
    else {
        char default_answer[] = "HTTP/1.1 404 Not Found" CRLF "Content-Length: 0" CRLF CRLF;
        printf("Send: %s\n", default_answer);
        write_some(fd, default_answer, strlen(default_answer));
    }
    
    close(fd);
}
