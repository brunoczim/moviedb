#ifndef MOVIEDB_STRBUF_H
#define MOVIEDB_STRBUF_H 1

#include <stdlib.h>
#include "error.h"

/**
 * Defines string buffer. It handles memory allocations for a growable string.
 */

/**
 * A string buffer. Handles allocation and allows the string to grow. The
 * string buffer tracks string length and the capacity of the buffer.
 */
struct strbuf {
    /**
     * The pointer to the allocation. You can freely read from the pointer and
     * write into the memory this pointer refers to, but only strbuf internal
     * code is allowed to update the pointer itself.
     */
    char *ptr;
    /**
     * Length of the string. Can be freely updated if you guarantee that:
     *      length <= capacity
     */
    size_t length;
    /**
     * Capacity of the buffer. You can read this, but only strbuf internal code
     * is allowed to update this.
     */
    size_t capacity;
};

/**
 * Initializes a string buffer with zeroed length and capacity. Beware, the
 * pointer is initialized to NULL.
 */
inline void strbuf_init(struct strbuf *restrict buf)
{
    buf->ptr = NULL;
    buf->length = 0;
    buf->capacity = 0;
}

/**
 * Reserves space for additional characters in the buffer. In case of error,
 * the error parameter is set to allocation error, and no reserve is done.
 */
void strbuf_reserve(
        struct strbuf *restrict buf,
        size_t additional,
        struct error *restrict error);

/**
 * Pushes a character onto the buffer, reserving necessary space. In case of
 * error, the error parameter is set to allocation error, and no push is done.
 */
inline void strbuf_push(
        struct strbuf *restrict buf,
        char ch,
        struct error *restrict error)
{
    if (buf->capacity == buf->length) {
        strbuf_reserve(buf, 1, error);
    }
    if (error->code == error_none) {
        buf->ptr[buf->length] = ch;
        buf->length++;
    }
}

/**
 * Ensures this buffer ends in a nul byte, appending it to the end if
 * necessary.
 */
inline void strbuf_make_cstr(
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    if (buf->length == 0 || buf->ptr[buf->length] != 0) {
        strbuf_push(buf, 0, error);
    }
}

/**
 * Copies the whole given string buffer to a C string, appending the nul byte
 * (0) if necessary. It might or not use the buffer allocation, and then, in
 * this case, the buffer will be reset.
 */
char *strbuf_copy_cstr(
        struct strbuf *restrict buf,
        struct error *restrict error);

/**
 * Frees the pointer of the string buffer. Do not use the buffer after this.
 */
inline void strbuf_destroy(struct strbuf *restrict buf)
{
    free(buf->ptr);
}

#endif
