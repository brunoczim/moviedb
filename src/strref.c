#include <ctype.h>
#include <string.h>
#include "strref.h"
#include "alloc.h"

extern inline void strref_slice(
        struct strref *restrict ref,
        size_t start,
        size_t end);

int strref_cmp_cstr(
        struct strref const *restrict ref,
        char const *restrict cstr)
{
    size_t i = 0;
    int cmp = 0;

    while (cmp == 0 && (i < ref->length || cstr[i] != 0)) {
        if (i >= ref->length) {
            cmp = -1;
        } else if (cstr[i] == 0) {
            cmp = 1;
        } else {
            if (ref->ptr[i] < cstr[i]) {
                cmp = -1;
            } else if (ref->ptr[i] > cstr[i]) {
                cmp = 1;
            }
            i++;
        }
    }

    return cmp;
}

int strref_icmp_cstr(
        struct strref const *restrict ref,
        char const *restrict cstr)
{
    size_t i = 0;
    int cmp = 0;
    char this_ch, other_ch;

    while (cmp == 0 && (i < ref->length || cstr[i] != 0)) {
        if (i >= ref->length) {
            cmp = -1;
        } else if (cstr[i] == 0) {
            cmp = 1;
        } else {
            this_ch = tolower(ref->ptr[i]);
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

char *strref_make_cstr(struct strref *restrict ref, struct error *error)
{
    size_t length;
    char *cstr = NULL;

    if (ref->ptr == NULL) {
        cstr = moviedb_alloc(1, error);
        if (error->code == error_none) {
            *cstr = 0;
        }
    } else {
        length = ref->length;

        while (length > 0 && ref->ptr[length - 1] == 0) {
            length--;
        }

        cstr = moviedb_alloc(length + 1, error);
        if (error->code == error_none) {
            memcpy(cstr, ref->ptr, length);
            cstr[length] = 0;
        }
    }

    return cstr;
}
