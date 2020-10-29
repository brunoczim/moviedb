#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "error.h"
#include "alloc.h"

void error_init(struct error *restrict error)
{
    error->code = error_none;
    error->context = NULL;
    error->free_context = false;
}

void error_set_code(struct error *restrict error, enum error_code code)
{
    char const *ptr;

    switch (error->code) {
        case error_dup_movie_title:
            if (error->data.dup_movie_title.free_title) {
                ptr = error->data.dup_movie_title.title;
                moviedb_free((void *) (void const *) ptr);
            }
            break;
        default:
            break;
    }

    error->code = code;
}

void error_set_context(
        struct error *restrict error,
        char const *context,
        bool free_context)
{
    if (error->free_context) {
        moviedb_free((void *) (void const *) error->context);
    }
    error->context = context;
    error->free_context = free_context;
}

void error_destroy(struct error *restrict error)
{
    error_set_code(error, error_none);
    error_set_context(error, NULL, false);
}

void error_print(struct error const *restrict error)
{
    if (error->context != NULL) {
        fprintf(stderr, "%s: ", error->context);
    }

    switch (error->code) {
        case error_none:
            fputs("ok\n", stderr);
            break;

        case error_csv:
            fprintf(stderr,
                    "CSV parser error, line %lu, column %lu\n",
                    error->data.csv.line,
                    error->data.csv.column);
            break;

        case error_alloc:
            fprintf(stderr,
                    "out of memory, allocation size %zu\n",
                    error->data.alloc.size);
            break;

        case error_io:
            fprintf(stderr, "%s\n", strerror(error->data.io.sys_errno));
            break;

        case error_movie:
            fprintf(stderr,
                    "invalid movie, line %lu\n",
                    error->data.csv_movie.line);
            break;

        case error_id:
            fputs("invalid ID ", stderr);
            error_print_quote(error->data.id.string);
            if (error->data.id.has_line) {
                fprintf(stderr, ", line %lu", error->data.id.line);
            }
            fputc('\n', stderr);
            break;

        case error_dup_movie_id:
            fprintf(stderr,
                    "duplicated movie ID %llu\n",
                    (long long unsigned) error->data.dup_movie_id.id);
            break;

        case error_dup_movie_title:
            fputs("duplicated movie title ", stderr);
            error_print_quote(error->data.dup_movie_title.title);
            fputc('\n', stderr);
            break;

        case error_csv_header:
            fputs("invalid CSV header\n", stderr);
            break;
    }
}

void error_print_quote(char const *string)
{
    char const *cursor;

    fputc('"', stderr);

    for (cursor = string; *cursor != 0; cursor++) {
        switch (*cursor) {
            case '\n':
                fputs("\\n", stderr);
                break;
            case '\r':
                fputs("\\r", stderr);
                break;
            case '"':
                fputs("\\\"", stderr);
                break;
            case '\\':
                fputs("\\\\", stderr);
                break;
            default:
                if (*cursor >= 32 && *cursor <= 126) {
                    fputc(*cursor, stderr);
                } else {
                    fprintf(stderr, "\\%hhu", *cursor);
                }
                break;
        }
    }

    fputc('"', stderr);
}
