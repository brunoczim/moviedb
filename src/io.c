#include "io.h"

extern inline FILE *input_file_open(
        char const *restrict path,
        struct error *error);

extern inline int input_file_read(FILE *file, struct error *error);

extern inline void input_file_close(FILE *file);
