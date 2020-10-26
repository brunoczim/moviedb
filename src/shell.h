#ifndef MOVIEDB_SHELL
#define MOVIEDB_SHELL 1

#include "error.h"
#include "trie.h"

/**
 * This file defines the interface of the shell/console mode of the application.
 */

/**
 * Runs the shell for the given trie tree root.
 */
void shell_run(struct trie_node const *trie_root,
        struct strbuf *buf,
        struct error *error);

#endif
