#include "strbuf.h"
#include "alloc.h"
#include <stdio.h>

extern inline void strbuf_init(struct strbuf *buf);

void strbuf_reserve(struct strbuf *buf, size_t additional, struct error *error)
{
    size_t new_capacity = buf->capacity + additional;
    char *new_alloc = moviedb_realloc(buf->ptr, new_capacity, error);
    if (new_alloc != NULL) {
        buf->capacity = new_capacity;
        buf->ptr = new_alloc;
    }
}

extern inline void strbuf_push(
        struct strbuf *buf,
        char ch,
        struct error *error);

extern inline inline void strbuf_free(struct strbuf *buf);
