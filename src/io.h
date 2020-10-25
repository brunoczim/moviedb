#ifndef MOVIEDB_IO_H
#define MOVIEDB_IO_H

#include <stdio.h>
#include <errno.h>
#include "error.h"

/**
 * This file provides some IO utilities for the application.ç
 */

/**
 * Opens an input file from the given path, handling any error into the error
 * out parameter.
 */
inline FILE *input_file_open(char const *restrict path, struct error *error)
{
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        error_set_code(error, error_io);
        error->data.io.sys_errno = errno;
    } 

    return file;
}

/**
 * Sets the buffer of an input file to the given buffer, of given size. If an
 * error happens, the buffer is not used.
 */
inline void input_file_setbuf(
        FILE *file,
        char *buffer,
        size_t size,
        struct error *error)
{
    if (setvbuf(file, buffer, _IOFBF, size) != 0) {
        error_set_code(error, error_io);
        error->data.io.sys_errno = errno;
    }
}

/**
 * Reads a byte from the file. Returns EOF in case of end of file. Writes an
 * error into the error out paramteter.
 */
inline int input_file_read(FILE *file, struct error *error)
{
    int ch = fgetc(file);

    if (ch == EOF && !feof(file)) {
        error_set_code(error, error_io);
        error->data.io.sys_errno = errno;
    }

    return ch;
}

/**
 * Closes the given input file.
 */
inline void input_file_close(FILE *file)
{
    fclose(file);
}

#endif
