#ifndef MOVIEDB_ALLOC_H
#define MOVIEDB_ALLOC_H 1

#include "error.h"
#include <stdlib.h>

/**
 * This file provides wrappers over allocation functions, so that failure is
 * handled such that a struct error is used.
 */

/**
 * Allocates a memory region of size given by size. If an error happens,
 * NULL is returned and the error parameter is set to allocation error.
 *
 * NULL might still be returned in case of a zero-sized allocation.
 */
void *moviedb_alloc(
        size_t elem_size,
        size_t elements,
        struct error *restrict error);

/**
 * Reallocates a memory region of size given by size, and a previous
 * allocation given by mem. If an error happens, NULL is returned and the error
 * parameter is set to allocation error. No memory is freed or reallocated in
 * case of error.
 *
 * NULL might still be returned in case of a zero-sized allocation.
 */
void *moviedb_realloc(
        void *mem,
        size_t elem_size,
        size_t elements,
        struct error *restrict error);

/**
 * Frees memory allocated by moviedb_alloc and moviedb_realloc.
 */
inline void moviedb_free(void *mem)
{
    free(mem);
}

#endif
