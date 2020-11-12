#include "strbuf.h"
#include "alloc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern inline void strbuf_init(struct strbuf *restrict buf);

void strbuf_reserve(
        struct strbuf *restrict buf,
        size_t additional,
        struct error *restrict error)
{
    size_t new_capacity;
    char *new_alloc;

    /* Tries to reallocate. */
    new_capacity = buf->capacity + additional;
    new_alloc = moviedb_realloc(buf->ptr, new_capacity, error);

    if (error->code == error_none) {
        /* If no error happened, publish the new allocation. */
        buf->capacity = new_capacity;
        buf->ptr = new_alloc;
    }
}

extern inline void strbuf_push(
        struct strbuf *restrict buf,
        char ch,
        struct error *restrict error);

extern inline void strbuf_make_cstr(
        struct strbuf *restrict buf,
        struct error *restrict error);

char *strbuf_copy_cstr(
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    char *cstr = NULL;

    if (buf->length == 0) {
        /* Empty string buffer. */
        cstr = moviedb_alloc(1, error);
        if (error->code == error_none) {
            *cstr = 0;
        }
    } else if (buf->ptr[buf->length - 1] == 0) {
        /* Last string buffer character is \0. */
        if (buf->capacity == buf->length) {
            /* And at full capacity. So, it reuses the allocation. */
            cstr = buf->ptr;
            strbuf_init(buf);
        } else {
            /* With extra space. So, it makes a new allocation. */
            cstr = moviedb_alloc(buf->length, error);
            if (error->code == error_none) {
                memcpy(cstr, buf->ptr, buf->length);
            }
        }
    } else if (buf->capacity == buf->length + 1) {
        /*
         * Last string buffer character is not \0 AND adding \0 would make the
         * buffer to be at full capacity.
         */
        cstr = buf->ptr;
        cstr[buf->length] = 0;
        strbuf_init(buf);
    } else {
        /*
         * Last string buffer character is not \0 AND adding \0 would NOT make
         * the buffer to be at full capacity. It would be either below or above.
         */
        cstr = moviedb_alloc(buf->length + 1, error);
        if (error->code == error_none) {
            memcpy(cstr, buf->ptr, buf->length);
            cstr[buf->length] = 0;
        }
    }
    return cstr;
}

extern inline inline void strbuf_destroy(struct strbuf *restrict buf);
