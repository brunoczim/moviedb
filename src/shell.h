#ifndef MOVIEDB_SHELL
#define MOVIEDB_SHELL 1

#include "error.h"
#include "database.h"

/**
 * This file defines the interface of the shell/console mode of the application.
 */

/**
 * Runs the shell for the given trie tree root.
 */
void shell_run(struct database const *restrict database,
        struct strbuf *restrict buf,
        struct error *restrict error);

#endif
