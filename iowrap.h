#ifndef IO_WRAPPERS_H__
#define IO_WRAPPERS_H__

#include <stddef.h> // size_t

typedef struct {
    char *buffer; // pointer to the `buffer`. Not owning any memory
    size_t size; // how much is filled starting from address pointed by `buffer`
    size_t capacity; // full `buffer` capacity
} BufferState;

void chop_left(BufferState* state, size_t bytes);

/* 
 * Read from socket to buffer of maxsize `size`

 * @return -1 if some error occured, 0 on EOF,
 *			otherwise return number of read bytes on success
 *			and fill the buffer with null-terminated string
 */
int read_some(int fd, char *buffer, size_t size);

/* 
 * Write to socket the buffer's content of size equal `size` 
 * Return value less or equal 0 if any error occured
 * otherwise return number of written bytes on success
 */
int write_some(int fd, char *buffer, size_t size);

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
char* read_until(int fd, BufferState *state, char *pattern);

#endif // IO_WRAPPERS_H__