#include "trie.h"

static size_t capacities[] = {
    1, 2, 3, 5, 7, 9, 11, 13, 17, 19, 23, 25, 29, 31, 37, 41, 43, 47, 49, 53,
    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 121, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 169, 173, 179, 181, 191, 193, 197, 199,
    211, 223, 227, 229, 233, 239, 241, 251, 257
};

static inline void init_table(struct trie_table *table);

void trie_root_init(struct trie_node *root)
{
    root->has_leaf = false;
    init_table(&root->table);
}

static inline void init_table(struct trie_table *table)
{
    size_t capacity;
    table->capacity = 0;
    capacity = capacities[table->capacity];
    table->size = 0;
    table->entries = malloc(capacity * sizeof(struct trie_entry));

    if (table->entries == NULL) {
        
    }
}
