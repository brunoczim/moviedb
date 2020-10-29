#ifndef MOVIEDB_SHELL
#define MOVIEDB_SHELL 1

#include "error.h"
#include "trie.h"
#include "movies.h"

/**
 * This file defines the interface of the shell/console mode of the application.
 */

/**
 * Runs the shell for the given trie tree root.
 */
void shell_run(struct trie_node const *restrict trie_root,
        struct movies_table const *restrict movies,
        struct strbuf *restrict buf,
        struct error *restrict error);

#endif
