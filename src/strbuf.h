#ifndef MOVBASE_STRBUF_H
#define MOVBASE_STRBUF_H 1

#include <stdlib.h>

struct strbuf {
    char *ptr;
    size_t length;
    size_t capacity;
};

inline void strbuf_init(struct strbuf *buf)
{
    buf->ptr = NULL;
    buf->length = 0;
    buf->capacity = 0;
}

void strbuf_reserve(struct strbuf *buf, size_t additional);

inline void strbuf_push(struct strbuf *buf, char ch)
{
    if (buf->capacity == buf->length) {
        strbuf_reserve(buf, 1);
    }
    buf->ptr[buf->length] = ch;
    buf->length += 1;
}

inline void strbuf_push_nul(struct strbuf *buf)
{
    strbuf_push(buf, 0);
}

inline void strbuf_free(struct strbuf *buf)
{
    free(buf->ptr);
}

#endif
