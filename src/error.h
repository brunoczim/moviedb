#ifndef MOVIEDB_ERROR_H
#define MOVIEDB_ERROR_H 1

#include <stdbool.h>
#include <stddef.h>

/**
 * This file provides error related utilities.
 */

/**
 * The code of an error.
 */
enum error_code {
    /**
     * No error occured.
     */
    error_none,
    /**
     * CSV parse error.
     */
    error_csv,
    /**
     * Out of memory.
     */
    error_alloc,
    /**
     * IO error, such as writing or reading from file.
     */
    error_io,
    /**
     * An error happened parsing a movie in a CSV file.
     */
    error_movie,
    /**
     * An error happened parsing the header of a CSV file.
     */
    error_csv_header,
    /**
     * Error found when parsing an ID.
     */
    error_id,
};

/**
 * CSV parse error's data.
 */
struct csv_error {
    /**
     * Line in a file where the error happened. Starts from 1.
     */
    unsigned long line;
    /**
     * Column in a file where the error happened. Starts from 1.
     */
    unsigned long column;
};

/**
 * Allocation/out of memory error's data.
 */
struct alloc_error {
    /**
     * How much was attempted to allocate, in bytes.
     */
    size_t size;
};

/**
 * IO error's data.
 */
struct io_error {
    /**
     * System code for this error (errno).
     */
    int sys_errno;
};

/**
 * Invalid movie error's data.
 */
struct movie_error {
    /**
     * Line in a file where the error happened. Starts from 1.
     */
    unsigned long line;
};

/**
 * Invalid ID error's data.
 */
struct moviedb_id_error {
    /**
     * Whether there is a line in this error.
     */
    bool has_line;
    /**
     * Line in a file where the error happened. Starts from 1.
     */
    unsigned long line;
    /**
     * The string that was attempted to parse.
     */
    char const *string;
    /**
     * Whether to free the string.
     */
    bool free_string;
};

/**
 * Union that might be any error's data.
 */
union error_data {
    /**
     * Data of a CSV parse error.
     */
    struct csv_error csv;
    /**
     * Data of an allocation/OOM error.
     */
    struct alloc_error alloc;
    /**
     * Data of an IO error.
     */
    struct io_error io;
    /**
     * Data of an invalid movie error.
     */
    struct movie_error movie;
    /**
     * Data of an invalid ID error.
     */
    struct moviedb_id_error id;
};

/**
 * An error object. Might be an actual error, or an error_none if there is no
 * error.
 */
struct error {
    /**
     * Code of this error. DO NOT set this directly. Set it through
     * error_set_code. Reading is fine.
     */
    enum error_code code;
    /**
     * Error-specific data. DO NOT edit this before setting the error code
     * through error_set_code. Only after. You should edit the error data
     * corresponding to the code set.
     */
    union error_data data;
    /**
     * The context of the error, for the user. Might be NULL. Might be
     * heap-allocated, might be a string literal. Set free_context accordingly.
     */
    char const *context;
    /**
     * Whether the context should be freed when destroying the error.
     */
    bool free_context;
};

/**
 * Initializes this error to a code of error_none, and to no context (NULL).
 */
void error_init(struct error *error);

/**
 * Sets the code of this error and destroys any data that needs to be destroyed.
 * After calling this function, you should not use the data anymore. In fact,
 * that is the right moment to edit error data.
 */
void error_set_code(struct error *error, enum error_code code);

/**
 * Sets the context of the error and destroys the old context, if necessary.
 * The free_context paramter specifies if the new context should be destroyed
 * when the context is replaced or the error is destroyed.
 */
void error_set_context(
        struct error *error,
        char const *context,
        bool free_context);

/**
 * Frees error data and context.
 */
void error_destroy(struct error *error);

/**
 * Prints the error on the standard error output (stderr).
 */
void error_print(struct error const *error);

/**
 * Prints a string quoting it and escaping non-printable characters.
 */
void error_print_quote(char const *string);

#endif
