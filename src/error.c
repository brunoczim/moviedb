#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "error.h"
#include "str.h"
#include "io.h"
#include "alloc.h"

static inline void destroy_data(struct error *restrict error);

extern inline struct error error_init();

void error_set_code(struct error *restrict error, enum error_code code)
{
    destroy_data(error);
    error->code = code;
}

void error_set_context(struct error *restrict error, struct string context)
{
    string_destroy(error->context);
    error->context = context;
}

void error_destroy(struct error *restrict error)
{
    destroy_data(error);
    string_destroy(error->context);
}

void error_print(struct error const *restrict error)
{
    struct error ignore = error_init();
    struct strref context = string_as_ref(error->context);

    if (context.chars != NULL) {
        io_write(stdout, context, &ignore);
    }

    switch (error->code) {
        case error_none:
            io_write(stdout, strref_lit("ok\n"), &ignore);
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

static void destroy_data(struct error *restrict error)
{
    switch (error->code) {
        case error_dup_movie_title:
            string_destroy(error->data.dup_movie_title.title);
            break;
        default:
            break;
    }
}
