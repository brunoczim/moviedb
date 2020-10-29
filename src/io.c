#include "io.h"

extern inline FILE *input_file_open(
        char const *restrict path,
        struct error *restrict error);

extern inline void input_file_setbuf(
        FILE *file,
        char *buffer,
        size_t size,
        struct error *restrict error);

extern inline int input_file_read(FILE *file, struct error *restrict error);

extern inline void input_file_close(FILE *file);
