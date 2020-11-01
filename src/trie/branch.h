#ifndef MOVIEDB_TRIE_BRANCH_H
#define MOVIEDB_TRIE_BRANCH_H 1

#include "../error.h"

/**
 * This file defines items related to branches of a trie. Only trie internal
 * code is allowed to touch this.
 */

struct trie_node;

/**
 * A branch of a trie tree. Only trie internal code is allowed to touch this.
 */
struct trie_branch {
    /**
     * The key of the branch (i.e. the character that leads to this path). Only
     * trie internal code is allowed to touch this.
     */
    char key;
    /**
     * A child of this branch's node, associated with the key above.
     */
    struct trie_node *child;
};

/**
 * A list of branches with their keys. Only trie internal code is allowed to
 * touch this.
 */
struct trie_branch_list {
    /**
     * The array of branches. Only trie internal code is allowed to touch this.
     */
    struct trie_branch *restrict entries;
    /**
     * Number of branches in this list. Only trie internal code is allowed to
     * touch this.
     */
    unsigned short length;
    /**
     * How many branches this list is currently able to store. Only trie
     * internal code is allowed to touch this.
     */
    unsigned short capacity;
};

/**
 * Initializes the list of branches. Only trie internal code is allowed to touch
 * this.
 */
inline void trie_branches_init(struct trie_branch_list *restrict branches)
{
    branches->entries = NULL;
    branches->length = 0;
    branches->capacity = 0;
}

/**
 * Searches for the given key in a branch list. Returns whether if the given key
 * was found. Fills pos with the position of the key. If not found, pos is
 * filled with the position where the key should be inserted. Only trie internal
 * code is allowed to touch this.
 */
bool trie_branches_search(
        struct trie_branch_list const *restrict list,
        char key,
        size_t *restrict pos);

/**
 * Inserts the given child into the given list of branches, using the given key.
 * Only trie internal code is allowed to touch this.
 */
void trie_branches_insert(
        struct trie_branch_list *restrict branches,
        char key,
        struct trie_node *child,
        size_t branch_pos,
        struct error *restrict error);
/**
 * Destroys a given branch list. Only trie internal code is allowed to touch
 * this.
 */
void trie_branches_destroy(struct trie_branch_list const *restrict branches);

#endif
