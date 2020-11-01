#include "alloc.h"

extern inline void *db_alloc(size_t bytes, struct error *restrict error);

extern inline void *db_realloc(
        void *mem,
        size_t bytes,
        struct error *restrict error);

extern inline void db_free(void *mem);
