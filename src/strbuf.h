#ifndef MOVIEDB_STRBUF_H
#define MOVIEDB_STRBUF_H 1

#include <stdlib.h>
#include "error.h"
#include "strref.h"

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
     * write into the memory this pointer refers to, but **DO NOT** change the
     * pointer's * addresss manually.
     */
    char *ptr;
    /**
     * Length of the string.
     */
    size_t length;
    /**
     * Capacity of the buffer.
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
        struct error *error);

/**
 * Pushes a character onto the buffer, reserving necessary space. In case of
 * error, the error parameter is set to allocation error, and no push is done.
 */
inline void strbuf_push(
        struct strbuf *restrict buf,
        char ch,
        struct error *error)
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
 * Makes a string reference using the buffer pointer, thus no extra-allocation
 * is done. However, pushing or reserving space in the string buffer might make
 * the strref's pointer invalid, so beware!
 */
inline void strbuf_as_ref(
        struct strbuf const *restrict buf,
        struct strref *restrict ref_out)
{
    ref_out->ptr = buf->ptr;
    ref_out->length = buf->length;
}

/**
 * Makes a C string from the whole given string buffer, appending the nul byte
 * (0) if necessary. It might or not use the buffer allocation, and then, in
 * this case, the buffer will be reset.
 */
char *strbuf_make_cstr(struct strbuf *restrict buf, struct error *error);

/**
 * Frees the pointer of the string buffer. Do not use the buffer after this.
 */
inline void strbuf_destroy(struct strbuf *restrict buf)
{
    free(buf->ptr);
}

#endif
