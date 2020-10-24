#ifndef MOVIEDB_TRIE_H
#define MOVIEDB_TRIE_H 1

#include <stdbool.h>
#include "error.h"

struct trie_node;

struct trie_branch {
    char key;
    struct trie_node *child;
};

struct trie_branch_list {
    struct trie_branch *restrict entries;
    unsigned short length;
    unsigned short capacity;
};

struct trie_node {
    bool has_leaf;
    unsigned long movieid;
    struct trie_branch_list branches;
};

inline void trie_root_init(struct trie_node *restrict root)
{
    root->has_leaf = false;
    root->branches.entries = NULL;
    root->branches.length = 0;
    root->branches.capacity = 0;
}

void trie_insert(
        struct trie_node *root,
        char const *restrict name,
        long unsigned movieid,
        struct error *error);

bool trie_search(
        struct trie_node const *root,
        char const *restrict name,
        unsigned long *movieid_out);

void trie_destroy(struct trie_node *root);

#endif
