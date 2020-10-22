#ifndef MOVBASE_STRBUF_H
#define MOVBASE_STRBUF_H 1

#include <stdlib.h>

/**
 * Defines string buffer. It handles memory allocations for a growable string.
 */

/**
 * A stirng buffer. Handles allocation and allows the string to grow. The
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
inline void strbuf_init(struct strbuf *buf)
{
    buf->ptr = NULL;
    buf->length = 0;
    buf->capacity = 0;
}

/**
 * Reserves space for additional characters in the buffer. Aborts if allocation
 * fails.
 */
void strbuf_reserve(struct strbuf *buf, size_t additional);

/**
 * Pushes a character onto the buffer, reserving necessary space. Aborts if
 * allocation fails.
 */
inline void strbuf_push(struct strbuf *buf, char ch)
{
    if (buf->capacity == buf->length) {
        strbuf_reserve(buf, 1);
    }
    buf->ptr[buf->length] = ch;
    buf->length += 1;
}

/**
 * Frees the pointer of the string buffer. Do not use the buffer after this.
 */
inline void strbuf_free(struct strbuf *buf)
{
    free(buf->ptr);
}

#endif
