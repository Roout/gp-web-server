#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h> 

#include <unistd.h>	    // close, fork
#include <sys/wait.h>   // wait

#include "iowrap.h"
#include "server.h"

#define HOST "127.0.0.1"
#define PORT "18001"

// size of listen's queue
#define BACKLOG 64
// maximum number of connections the server can maintain simultaneously
#define MAX_CONNECTIONS 1000

// client's buffer size used for reading incoming requests
// if the request will be longer than this size then 
// server will stop reading and close the connection 
#define BUFFER_SIZE (1<<16)

/**
 * Handle client with provided socket descriptor `fd`
 * It's being invoked in child process
 * 
 * @param fd socket descriptor of the accepted client
 */
static void handle_client(int fd);

/**
 * Process result of the waitpid (pid, wstatus) and modify process_count on success
 * 
 * @param pid pid of the child process
 * @param wstatus status of the child process assigned by `waitpid`
 * @param process_count counter of the active child processes (connections)
 * 
 * @return -1 of failure otherwise 0
 */
static int handle_client_pid(pid_t pid, int wstatus, size_t *process_count);

/**
 * Wait until number of active processes will be lesser than MAX_CONNECTION constant
 * Use `waitpid` to handle alread terminated child processes
 * 
 * @param process_count number of created child processes/connections
 * 
 * @return -1 of failure otherwise 0
 */
static int wait_slots(size_t *process_count);

int main() {
    printf("Server started at %shttp://%s:%s%s\n", "\033[92m", HOST, PORT, "\033[0m");

    Server server;
    init_server(&server, HOST, PORT, BACKLOG);

    // number of not-terminated processes
    size_t process_count = 0;
    // accept clients
    while (1) {
        if (wait_slots(&process_count) < 0) {
            // failed to terminate any child process
            exit(EXIT_FAILURE);
        }
		int client_fd = accept_client(&server);
        int pid = fork();
        if (pid < 0) { // error
            fprintf(stderr, "Failed to create a child process for the new client.\n");
            fprintf(stderr, "Reason: %s\n", strerror(errno));
            close(client_fd);
        }
        else if (pid == 0) { // here goes child process's branch
            handle_client(client_fd);
            exit(EXIT_SUCCESS);
        }
        else { // parent's branch
            // process was created successfully
            process_count++;
            // parent process
            // close the fd because only the child should own it now
            close(client_fd);
        }
    }
    return 0;
}

static int handle_client_pid(pid_t pid, int wstatus, size_t *process_count) {
    assert(process_count);
    if (pid == -1) {
        fprintf(stderr, "waitpid failed.\n");
        fprintf(stderr, "Reason: %s\n", strerror(errno));
        return -1;
    }
    // process was terminated
    assert(*process_count > 0);
    (*process_count)--;
    if (!WIFEXITED(wstatus)) {
        // child isn't terminated normally by exit(...)
        // log this to stderr
        fprintf(stderr, "Child %d isn't terminated normally\n", pid);
    }
    return 0;
}

static int wait_slots(size_t *process_count) {
    assert(process_count);
    if (*process_count < MAX_CONNECTIONS) {
        return 0;
    }
    // otherwise hang server until there will be free slot
    int wstatus = 0;
    pid_t pid = 0;
    // try to handle as many terminated processes as possible in non-blocking mode
    while ((pid = waitpid(-1, &wstatus, WNOHANG)) != 0) {
        // handle success or error in function `handle_client_pid`
        if (handle_client_pid(pid, wstatus, process_count) < 0) {
            return -1;
        }
    }
    // if still no free connections -> block until there will be one
    if (*process_count == MAX_CONNECTIONS) {
        wstatus = 0;
        // default option = 0 (last arg)
        // because it waits only for terminated children
        pid = waitpid(-1, &wstatus, 0);
        if (handle_client_pid(pid, wstatus, process_count) < 0) {
            return -1;
        }
    }
    // success
    return 0;
}

typedef struct {
    const char *method;   // GET or POST
    const char *route;    // /route/to/file
    const char *protocol; // HTTP/1.1
} RequestHeader;

/**
 * @param data null-terminated string
 * @param header parsing result on success
 * @return 0 on success otherwise -1
*/
static int parse_header(char* data, RequestHeader* header) {
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

static void handle_client(int fd) {
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
        // example how to parse headers { key : value } from the request
        // using read_until
        while (match != NULL) {
            match = read_until(fd, &state, pattern);
            if (match == NULL) {
                // unexpected format of the input request
                close(fd);
                return;
            }
            if (state.buffer == match) {
                // meet the combination \r\n\r\n which means end of the GET request
                break;
            }
            // skip already processes part of request
            char *line = state.buffer;
            chop_left(&state, (match - line) + strlen(pattern));
            
            // (note, this is done within child process and access to stdout is not synchronized
            // so it can be used only for testing)
            // To print parsed {filed : value} uncomment lines below

            // const char *field = strtok(line, ": ");
            // const char *value = strtok(NULL, "\r\n");
            // assert(*match == '\0');
            // printf("Parse: %s:%s\n", field, value);
        }
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nGET";
        write_some(fd, response, strlen(response));
    }
    else if (!strcmp(header.method, "POST")) {
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPOST";
        write_some(fd, response, strlen(response));
    }
    else {
        const char *response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
        write_some(fd, response, strlen(response));
    }
    
    close(fd);
}
