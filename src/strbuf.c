#include "strbuf.h"
#include "alloc.h"
#include <stdio.h>

extern inline void strbuf_init(struct strbuf *restrict buf);

void strbuf_reserve(
        struct strbuf *restrict buf,
        size_t additional,
        struct error *error)
{
    size_t new_capacity;
    char *new_alloc;

    new_capacity = buf->capacity + additional;
    new_alloc = moviedb_realloc(buf->ptr, new_capacity, error);
    if (error->code == error_none) {
        buf->capacity = new_capacity;
        buf->ptr = new_alloc;
    }
}

extern inline void strbuf_push(
        struct strbuf *restrict buf,
        char ch,
        struct error *error);

extern inline inline void strbuf_free(struct strbuf *restrict buf);
