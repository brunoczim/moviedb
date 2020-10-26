#ifndef MOVIEDB_SHELL
#define MOVIEDB_SHELL 1

#include "error.h"
#include "trie.h"

void shell_run(struct trie_node const *trie_root,
        struct strbuf *buf,
        struct error *error);

#endif
