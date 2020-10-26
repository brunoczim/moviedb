#ifndef MOVIEDB_STRREF_H
#define MOVIEDB_STRREF_H 1

#include <stdlib.h>
#include "error.h"

/**
 * This file defines a "string reference" type, which is a reference to a
 * borrowed string, such as those allocated by a string buffer (strbuf).
 */

/** 
 * A string reference's data.
 */
struct strref {
    /**
     * The pointer to the data. Be careful updating this pointer.
     */
    char const *ptr;
    /**
     * Length of the data. Be careful updating this.
     */
    size_t length;
};

/**
 * Slices the reference using a given range of the form [start, end) (i.e., end
 * is exclusive).
 */
inline void strref_slice(
        struct strref *restrict ref,
        size_t start,
        size_t end)
{
    if (ref->ptr != NULL) {
        if (start > ref->length) {
            start = ref->length;
        }
        if (end < start) {
            end = start;
        }
        if (end - start > ref->length) {
            end = ref->length;
        }
        ref->ptr += start;
        ref->length = end - start;
    }
}

/**
 * Makes a C string from the given string reference, appending the nul byte
 * (0) if necessary. It will always allocate a new buffer, but it might fail.
 */
char *strref_make_cstr(struct strref *restrict ref, struct error *error);

/**
 * Compares the string reference with a C string. Returns 1 if greater than the
 * C string, returns -1 if less than the C string, and 0 if equal.
 */
int strref_cmp_cstr(
        struct strref const *restrict ref,
        char const *restrict cstr);

/**
 * Like ref_cmp_cstr, but case-insensitive.
 */
int strref_icmp_cstr(
        struct strref const *restrict ref,
        char const *restrict cstr);


#endif
