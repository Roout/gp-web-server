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

/* 
 * Read from socket to buffer of maxsize `size`

 * @return -1 if some error occured, 0 on EOF,
 *					otherwise return number of read bytes on success
 *					and fill the buffer with null-terminated string
 */
int read_some(int fd, char *buffer, size_t size) {
    int read_bytes = read(fd, buffer, size - 1);
    if (read_bytes == 0) {
        fprintf(stderr, "Reading from socket %d reached EOF: %s\n"
            , fd, strerror(errno));
        buffer[0] = '\0';
        return 0;
    }
    else if (read_bytes < 0) {
        fprintf(stderr, "Failed to read from socket %d: %s\n"
            , fd, strerror(errno));
        return -1;
    }
    buffer[read_bytes] = '\0';
    return read_bytes;
}   

/* 
 * Write to socket the buffer's content of size equal `size` 
 * Return value less or equal 0 if any error occured
 * otherwise return number of written bytes on success
 */
int write_some(int fd, char *buffer, size_t size) {
    size_t total_bytes = 0;
    // we need to confirm that the whole buffer is sent
    while (total_bytes < size) {
        int write_bytes = write(fd, buffer, size);
        if (write_bytes <= 0) {
            fprintf(stderr, "Failed to write to a client %d: %s\n"
                , fd, strerror(errno));
            // return as an error indication
            return write_bytes;
        }
        total_bytes += write_bytes;
    }
    return total_bytes;
}

/**
 * Read from socket with descriptor `fd` to `dst` buffer starting from position `*size`
 * until either `*size` becomes equal `capacity` or the `pattern` is met. 
 * Note, can modify `*size` to indicate how many bytes we've read 
 * 
 * @param fd socket descriptor
 * @param dst buffer where the data is being stored
 * @param size bytes in the `dst` buffer with valid (e.g., already read but not proccessed) data
 * @param capacity `dst` buffer capacity. So number of bytes can be used equal to `capacity - *size` 
 * @param pattern reading until it meet this substring
 * 
 * @return pointer to the matched string on success otherwise return NULL
*/
char* read_until(int fd, char *dst, size_t *size, size_t capacity, char *pattern) {
    assert(size != NULL);
    assert(dst != NULL && pattern != NULL);

    size_t pattern_len = strlen(pattern);
    char *search_start = dst;
    // pointer where the search for the pattern will start
    char *read_start = dst + *size;
    // read while we have enough space
    while (search_start != NULL) {
        // confirm that it's enough data where we can search for the pattern
        size_t search_len = read_start - search_start;
        if (search_len >= pattern_len) {
            // search for the match within already existing data in the buffer
            char *match = strstr(search_start, pattern);
            if (match != NULL) {
                // No need to read from the socket
                // The buffer already has the pattern
                return match;
            }
            else {
                // shift pointer by following amount of bytes to 
                // be able to include the suffix of size `pattern_len - 1`
                // in the next search
                // e.g., we can have "abc" in buffer search for "cd". There is no match
                // so read from the socket "def". Now we've got in buffer "abcdef" and 
                // start serach from the position: len(abc) - len(cd) + 1 = 2 which is "cdef"
                search_start += search_len - pattern_len + 1;
            }
        }
        if (capacity <= *size + 1) {
            // Not enough memory to read more
			// Note, add 1 because read_some adds '\0'
            return NULL;
        }
        // else read not enough characters for the search
        int read_bytes = read_some(fd, read_start, capacity - *size);
        if (read_bytes <= 0) {
			// meet either EOF either error
            return NULL;
        }

        read_start += read_bytes;
        *size += read_bytes;
    }
    // found nothing but capacity is not enough
    return NULL;
}

void HandleClient(int fd) {
    char buffer[BUFFER_SIZE + 1];
    char *pattern = "\r\n";

    char *free_buffer = buffer;
    size_t size = 0;
    size_t capacity = BUFFER_SIZE;
    size_t pattern_len = strlen(pattern);
    char *match = NULL;
    while ((match = read_until(fd, free_buffer, &size, capacity, pattern)) != NULL) {
        size_t consumed = match - free_buffer;
        *match = '\0';
        printf("read_until: %s\n", free_buffer);
        capacity -= consumed + pattern_len;
        free_buffer = match + pattern_len;
        assert(size >= consumed + pattern_len);
        size -= consumed + pattern_len;
    }
    if (size > 0) {
        printf("read_until: %s\n", free_buffer);
    }
    (void) close(fd);
}
