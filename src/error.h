#ifndef MOVIEDB_ERROR_H
#define MOVIEDB_ERROR_H 1

#include <stdbool.h>
#include <stddef.h>
#include "id/def.h"

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
     * An error happened parsing a rating in a CSV file.
     */
    error_rating,
    /**
     * An error happened parsing a tag association in a CSV file.
     */
    error_tag,
    /**
     * An error happened parsing the header of a CSV file.
     */
    error_csv_header,
    /**
     * Error found when parsing an ID.
     */
    error_id,
    /**
     * An error happened parsing a double in a CSV file.
     */
    error_double,
    /**
     * Error found if there are two movies with the same ID.
     */
    error_dup_movie_id,
    /**
     * Error found if there are two movies with the same title.
     */
    error_dup_movie_title,
    /**
     * Error that happens when something that is not a quoted is inserted at the
     * beginning of an argument expected to be quoted.
     */
    error_bad_quote,
    /**
     * Error that happens when reading a quoted argument in shell mode and the
     * argument was not terminated.
     */
    error_open_quote,
    /**
     * Error that happens when an argument is expected in shell mode.
     */
    error_expected_arg,
    /**
     * Error that happens when no more arguments are expected in shell mode.
     */
    error_expected_end,
    /**
     * Error that happens when an N in topN is not a valid number.
     */
    error_topn_count,
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
struct csv_movie_error {
    /**
     * Line in a file where the error happened. Starts from 1.
     */
    unsigned long line;
};

/**
 * Invalid rating error's data.
 */
struct csv_rating_error {
    /**
     * Line in a file where the error happened. Starts from 1.
     */
    unsigned long line;
};

/**
 * Invalid tag association error's data.
 */
struct csv_tag_error {
    /**
     * Line in a file where the error happened. Starts from 1.
     */
    unsigned long line;
};

/**
 * Duplicated movie ID error's data.
 */
struct dup_movie_id_error {
    /**
     * The duplicated ID.
     */
    moviedb_id_t id;
};

/**
 * Duplicated movie title error's data.
 */
struct dup_movie_title_error {
    /**
     * The duplicated title.
     */
    char const *title;
    /**
     * Whether to free the title.
     */
    bool free_title;
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
 * Invalid double error's data.
 */
struct double_error {
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
 * Open quote error's data.
 */
struct open_quote_error {
    /**
     * The given string.
     */
    char const *string;
    /**
     * Whether to free the string.
     */
    bool free_string;
};

/**
 * Bad quote error's data.
 */
struct bad_quote_error {
    /**
     * The character that was found instead of the quote.
     */
    char found;
};

/**
 * TopN count error's data.
 */
struct topn_count_error {
    /**
     * The given string.
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
    struct csv_movie_error csv_movie;
    /**
     * Data of an invalid rating error.
     */
    struct csv_rating_error csv_rating;
    /**
     * Data of an invalid tag association error.
     */
    struct csv_tag_error csv_tag;
    /**
     * Data of an invalid double error.
     */
    struct double_error double_f;
    /**
     * Data of an invalid ID error.
     */
    struct moviedb_id_error id;
    /**
     * Data of a duplicated movie ID error.
     */
    struct dup_movie_id_error dup_movie_id;
    /**
     * Data of a duplicated movie title error.
     */
    struct dup_movie_title_error dup_movie_title;
    /**
     * Data of open quote error.
     */
    struct open_quote_error open_quote;
    /**
     * Data of bad quote error.
     */
    struct bad_quote_error bad_quote;
    /**
     * Data of topN bad count error.
     */
    struct topn_count_error topn_count;
};

/**
 * An error object. Might be an actual error, or an error_none if there is no
 * error.
 */
struct error {
    /**
     * Code of this error. You can read this, only error internal code is
     * allowed to write to this directly. Set it through  error_set_code.
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
void error_init(struct error *restrict error);

/**
 * Sets the code of this error and destroys any data that needs to be destroyed.
 * After calling this function, you should not use the data anymore. In fact,
 * that is the right moment to edit error data.
 */
void error_set_code(struct error *restrict error, enum error_code code);

/**
 * Sets the context of the error and destroys the old context, if necessary.
 * The free_context paramter specifies if the new context should be destroyed
 * when the context is replaced or the error is destroyed.
 */
void error_set_context(
        struct error *restrict error,
        char const *context,
        bool free_context);

/**
 * Frees error data and context.
 */
void error_destroy(struct error *restrict error);

/**
 * Prints the error on the standard error output (stderr).
 */
void error_print(struct error const *restrict error);

/**
 * Prints a string quoting it and escaping non-printable characters.
 */
void error_print_quote(char const *string);

#endif
