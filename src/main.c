#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "error.h"
#include "alloc.h"
#include "strbuf.h"
#include "io.h"
#include "csv/movie.h"
#include "csv/rating.h"
#include "csv/tag.h"
#include "database.h"
#include "shell.h"

int main(int argc, char const *argv[])
{
    int exit_code = 0;
    struct database database;
    struct error error;
    struct strbuf buf;
    clock_t then, now;
    double secs;

    puts("Loading data...");

    error_init(&error);
    strbuf_init(&buf);

    then = clock();
    database_load(&database, &buf, &error);
    now = clock();

    if (error.code == error_none) {
        secs = (now - then) / (double) CLOCKS_PER_SEC;
        printf("Data loaded in %.3lf seconds\n", secs);
        puts("Entering in shell/console mode...");

        shell_run(&database, &buf, &error);
    }

    if (error.code != error_none) {
        error_print(&error);
    }

    database_destroy(&database);
    strbuf_destroy(&buf);
    error_destroy(&error);

    return exit_code;
}
