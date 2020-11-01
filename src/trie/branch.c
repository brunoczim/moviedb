#include "branch.h"
#include "../alloc.h"

extern inline void trie_branches_init(
        struct trie_branch_list *restrict branches);

bool trie_branches_search(
        struct trie_branch_list const *restrict list,
        char key,
        size_t *restrict pos)
{
    size_t low, mid, high;
    bool found;

    found = false;
    low = 0;
    high = list->length;

    while (low < high && !found) {
        mid = low + (high - low) / 2;

        if (list->entries[mid].key < key) {
            low = mid + 1;
        } else if (list->entries[mid].key > key) {
            high = mid;
        } else {
            low = mid;
            found = true;
        }
    }

    *pos = low;

    return found;
}

void trie_branches_insert(
        struct trie_branch_list *restrict branches,
        char key,
        struct trie_node *child,
        size_t branch_pos,
        struct error *restrict error)
{
    size_t i;
    size_t new_cap;
    struct trie_branch *new_entries;

    if (branches->length >= branches->capacity) {
        new_cap = branches->capacity * 2;
        if (new_cap == 0) {
            new_cap = 1;
        } else if (new_cap > 255) {
            new_cap = 255;
        }

        new_entries = db_realloc(
                branches->entries,
                sizeof(struct trie_branch) * new_cap,
                error);
        if (error->code == error_none) {
            branches->entries = new_entries;
            branches->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        for (i = branches->length; i > branch_pos; i--) {
            branches->entries[i] = branches->entries[i - 1];
        }

        branches->entries[branch_pos].key = key;
        branches->entries[branch_pos].child = child;
        branches->length++;
    }
}

void trie_branches_destroy(struct trie_branch_list const *restrict branches)
{
    db_free(branches->entries);
}
