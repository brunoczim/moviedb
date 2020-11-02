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

    new_capacity = buf->capacity + additional;
    new_alloc = db_realloc(buf->ptr, new_capacity, error);
    if (error->code == error_none) {
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
        cstr = db_alloc(1, error);
        if (error->code == error_none) {
            *cstr = 0;
        }
    } else if (buf->ptr[buf->length - 1] == 0) {
        if (buf->capacity == buf->length) {
            cstr = buf->ptr;
            strbuf_init(buf);
        } else {
            cstr = db_alloc(buf->length, error);
            if (error->code == error_none) {
                memcpy(cstr, buf->ptr, buf->length);
            }
        }
    } else if (buf->capacity == buf->length + 1) {
        cstr = buf->ptr;
        cstr[buf->length] = 0;
        strbuf_init(buf);
    } else {
        cstr = db_alloc(buf->length + 1, error);
        if (error->code == error_none) {
            memcpy(cstr, buf->ptr, buf->length);
            cstr[buf->length] = 0;
        }
    }
    return cstr;
}

extern inline inline void strbuf_destroy(struct strbuf *restrict buf);
