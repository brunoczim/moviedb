#include "io.h"

extern inline size_t io_write(
        FILE *file,
        struct strref data,
        struct error *restrict error);

extern inline FILE *io_open(
        char const *restrict path,
        struct error *error);

extern inline void io_setbuf(
        FILE *file,
        char *buffer,
        size_t size,
        struct error *error);

extern inline int io_read(FILE *file, struct error *error);

extern inline void io_close(FILE *file);
