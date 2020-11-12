#include "alloc.h"

void *moviedb_alloc(
        size_t elem_size,
        size_t elements,
        struct error *restrict error)
{
    size_t size = elem_size * elements;
    bool fail = true;
    void *mem = NULL;

    if (elem_size == 0 || size / elem_size == elements) {
        mem = malloc(size);
        fail = mem == NULL && size != 0;
    }

    if (fail) {
        error_set_code(error, error_alloc);
        error->data.alloc.elem_size = elem_size;
        error->data.alloc.elements = elements;
    }

    return mem;
}

void *moviedb_realloc(
        void *mem,
        size_t elem_size,
        size_t elements,
        struct error *restrict error)
{
    size_t size = elem_size * elements;
    bool fail = true;
    void *new_mem = NULL;

    if (elem_size == 0 || size / elem_size == elements) {
        new_mem = realloc(mem, size);
        fail = new_mem == NULL && size != 0;
    }

    if (fail) {
        error_set_code(error, error_alloc);
        error->data.alloc.elem_size = elem_size;
        error->data.alloc.elements = elements;
    }

    return new_mem;
}

extern inline void moviedb_free(void *mem);
