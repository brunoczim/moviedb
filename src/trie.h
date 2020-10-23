#ifndef MOVIEDB_TRIE_H
#define MOVIEDB_TRIE_H 1

#include <stdbool.h>

struct trie_entry;

struct trie_table {
    struct trie_entry *entries;
    unsigned char size;
    unsigned char capacity;
};

struct trie_node {
    bool has_leaf;
    unsigned long movieid;
    struct trie_table table;
};

struct trie_entry {
    int ch;
    struct trie_node child;
};


void trie_root_init(struct trie_node *root);

#endif
