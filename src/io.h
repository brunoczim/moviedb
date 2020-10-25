#ifndef MOVIEDB_IO_H
#define MOVIEDB_IO_H

#include <stdio.h>
#include <errno.h>
#include "error.h"

inline FILE *input_file_open(char const *restrict path, struct error *error)
{
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        error_set_code(error, error_io);
        error->data.io.sys_errno = errno;
    }

    return file;
}

inline int input_file_read(FILE *file, struct error *error)
{
    int ch = fgetc(file);

    if (ch == EOF && !feof(file)) {
        error_set_code(error, error_io);
        error->data.io.sys_errno = errno;
    }

    return ch;
}

inline void input_file_close(FILE *file)
{
    fclose(file);
}

#endif
