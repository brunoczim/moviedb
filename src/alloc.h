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
inline void *moviedb_alloc(size_t size, struct error *error)
{
    void *mem = malloc(size);
    if (mem == NULL && size != 0) {
        error_set_code(error, error_alloc);
        error->data.alloc.size = size;
    }
    return mem;
}

/**
 * Reallocates a memory region of size given by size, and a previous
 * allocation given by mem. If an error happens, NULL is returned and the error
 * parameter is set to allocation error. No memory is freed or reallocated in
 * case of error.
 *
 * NULL might still be returned in case of a zero-sized allocation.
 */
inline void *moviedb_realloc(void *mem, size_t size, struct error *error)
{
    void *new_mem = realloc(mem, size);
    if (new_mem == NULL && size != 0) {
        error_set_code(error, error_alloc);
        error->data.alloc.size = size;
    }
    return new_mem;
}

/**
 * Frees memory allocated by moviedb_alloc and moviedb_realloc.
 */
inline void moviedb_free(void *mem)
{
    free(mem);
}

#endif
