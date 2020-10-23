#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "error.h"
#include "alloc.h"

void error_init(struct error *error)
{
    error->code = error_none;
    error->context = NULL;
    error->free_context = false;
}

void error_set_code(struct error *error, enum error_code code)
{
    error->code = code;
}

void error_set_context(
        struct error *error,
        char const *context,
        bool free_context)
{
    if (error->free_context) {
        moviedb_free((void *) (void const *) error->context);
    }
    error->context = context;
    error->free_context = free_context;
}

void error_free(struct error *error)
{
    error_set_code(error, error_none);
    error_set_context(error, NULL, false);
}

void error_print(struct error const *error)
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
                    "Out of memory, allocation size %zu\n",
                    error->data.alloc.size);
            break;

        case error_io:
            fprintf(stderr, "%s\n", strerror(error->data.io.sys_errno));
            break;
    }
}
