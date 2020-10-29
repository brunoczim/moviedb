#include "alloc.h"

extern inline void *moviedb_alloc(size_t bytes, struct error *restrict error);

extern inline void *moviedb_realloc(
        void *mem,
        size_t bytes,
        struct error *restrict error);

extern inline void moviedb_free(void *mem);
