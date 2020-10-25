#ifndef MOVIEDB_SHELL
#define MOVIEDB_SHELL 1

#include "error.h"
#include "trie.h"

void moviedb_shell(
        struct trie_node const *trie_root,
        struct error *fatal_error);

#endif
