#include "strbuf.h"
#include "alloc.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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

int strbuf_cmp_cstr(
        struct strbuf const *restrict buf,
        char const *restrict cstr)
{
    size_t i = 0;
    int cmp = 0;

    while (cmp == 0 && (i < buf->length || cstr[i] != 0)) {
        if (i >= buf->length) {
            cmp = -1;
        } else if (cstr[i] == 0) {
            cmp = 1;
        } else {
            if (buf->ptr[i] < cstr[i]) {
                cmp = -1;
            } else if (buf->ptr[i] > cstr[i]) {
                cmp = 1;
            }
            i++;
        }
    }

    return cmp;
}

int strbuf_icmp_cstr(
        struct strbuf const *restrict buf,
        char const *restrict cstr)
{
    size_t i = 0;
    int cmp = 0;
    char this_ch, other_ch;

    while (cmp == 0 && (i < buf->length || cstr[i] != 0)) {
        if (i >= buf->length) {
            cmp = -1;
        } else if (cstr[i] == 0) {
            cmp = 1;
        } else {
            this_ch = tolower(buf->ptr[i]);
            other_ch = tolower(cstr[i]);
            if (this_ch < other_ch) {
                cmp = -1;
            } else if (this_ch > other_ch) {
                cmp = 1;
            }
            i++;
        }
    }

    return cmp;
}

char *strbuf_make_cstr(struct strbuf *restrict buf, struct error *error)
{
    char *cstr = NULL;

    if (buf->ptr == NULL) {
        cstr = moviedb_alloc(1, error);
        if (error->code == error_none) {
            *cstr = 0;
        }
    } else if (buf->ptr[buf->length - 1] == 0) {
        if (buf->capacity == buf->length) {
            cstr = buf->ptr;
            strbuf_init(buf);
        } else {
            cstr = moviedb_alloc(buf->length, error);
            if (error->code == error_none) {
                memcpy(cstr, buf->ptr, buf->length);
            }
        }
    } else if (buf->capacity == buf->length + 1) {
        cstr = buf->ptr;
        cstr[buf->length] = 0;
        strbuf_init(buf);
    } else {
        cstr = moviedb_alloc(buf->length + 1, error);
        if (error->code == error_none) {
            memcpy(cstr, buf->ptr, buf->length);
            cstr[buf->length] = 0;
        }
    }
    return cstr;
}

extern inline inline void strbuf_destroy(struct strbuf *restrict buf);
