#include "iowrap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>

void chop_left(BufferState* state, size_t bytes) {
    assert(state);
    assert(state->buffer);
    assert(state->size >= bytes);

    state->buffer += bytes;
    state->size -= bytes;
    state->capacity -= bytes;
}

/* 
 * Read from socket to buffer of maxsize `size`

 * @return -1 if some error occured, 0 on EOF,
 *			otherwise return number of read bytes on success
 *			and fill the buffer with null-terminated string
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
 * @param state pointer to current state of the some global buffer
 * @param pattern reading until it meet this substring
 * 
 * @return pointer to the matched string on success otherwise return NULL
*/
char* read_until(int fd, BufferState *state, char *pattern) {
    assert(state != NULL);
    assert(pattern != NULL);

    size_t pattern_len = strlen(pattern);
    char *search_start = state->buffer;
    // pointer where the search for the pattern will start
    char *read_start =  state->buffer + state->size;
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
                // e.g., we can have "abc" in buffer and search for "cd". There is no match
                // so read from the socket "def". Now we've got in buffer "abcdef" and 
                // start serach from the position: len(abc) - len(cd) + 1 = 2 which is "cdef"
                search_start += search_len - pattern_len + 1;
            }
        }
        if (state->capacity <= state->size + 1) {
            // Not enough memory to read more
			// Note, add 1 because read_some adds '\0'
            return NULL;
        }
        // else read not enough characters for the search
        int read_bytes = read_some(fd, read_start, state->capacity - state->size);
        if (read_bytes <= 0) {
			// meet either EOF either error
            return NULL;
        }

        read_start += read_bytes;
        state->size += read_bytes;
    }
    // found nothing but capacity is not enough
    return NULL;
}

int read_file(const char* filename, Buffer *dst) {
    assert(buffer);
    assert(filename);

    FILE* file = open(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Fail to open the file %s with error: %s\n"
            , filename
            , strerror(errno));
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (filesize == EOF) {
        return -1;
    }
    char *buffer = (char*) malloc(sizeof(char) * (size + 1));
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory for the file %s\n"
            , filename);
        return -1;
    }
    buffer[size] = '\0';
    // set cursor to the file's begining
    rewind(file);
    // read
    size_t read_items = fread(buffer, 1, size, file);
    if (read_items < size) {
        fprintf(stderr, "Fail to read the file %s with error: %s\n"
            , filename
            , strerror(errno));
        free(buffer);
        return -1;
    }
    // update buffer
    dst->buffer = buffer;
    dst->size = size;
    fclose(file);
    // success
    return 0;
}