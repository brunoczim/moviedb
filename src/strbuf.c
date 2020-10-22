#include "strbuf.h"
#include <stdio.h>

extern inline void strbuf_init(struct strbuf *buf);

void strbuf_reserve(struct strbuf *buf, size_t additional)
{
    size_t new_capacity = buf->capacity + additional;
    char *new_alloc = realloc(buf->ptr, new_capacity);
    if (new_capacity != 0 && new_alloc == NULL) {
        fprintf(stderr, "Out of memory allocating %zu bytes\n", new_capacity);
        free(buf->ptr);
        abort();
    }
    buf->capacity = new_capacity;
    buf->ptr = new_alloc;
}

extern inline void strbuf_push(struct strbuf *buf, char ch);

extern inline inline void strbuf_free(struct strbuf *buf);
