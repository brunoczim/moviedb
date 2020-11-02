#ifndef MOVIEDB_IO_H
#define MOVIEDB_IO_H

#include <stdio.h>
#include <errno.h>
#include "error.h"
#include "strbuf.h"

/**
 * Terminal green foreground color. Needs to be printed.
 */
#define TERMINAL_GREEN "\e[92m"

/**
 * Terminal yellow foreground color. Needs to be printed.
 */
#define TERMINAL_YELLOW "\e[93m"

/**
 * Terminal blue foreground color. Needs to be printed.
 */
#define TERMINAL_BLUE "\e[94m"

/**
 * Terminal red foreground color. Needs to be printed.
 */
#define TERMINAL_RED "\e[91m"

/**
 * Terminal magenta foreground color. Needs to be printed.
 */
#define TERMINAL_MAGENTA "\e[95m"

/**
 * Resets the terminal colors to the default. Needs to be printed.
 */
#define TERMINAL_CLEAR "\e[0m"

/**
 * This file provides some IO utilities for the application.
 */

/**
 * Opens an input file from the given path, handling any error into the error
 * out parameter.
 */
inline FILE *input_file_open(
        char const *restrict path,
        struct error *restrict error)
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
        struct error *restrict error)
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
inline int input_file_read(FILE *file, struct error *restrict error)
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
