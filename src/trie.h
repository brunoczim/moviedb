#ifndef MOVIEDB_TRIE_H
#define MOVIEDB_TRIE_H 1

#include <stdbool.h>
#include "error.h"
#include "movie.h"

/**
 * This file provides an interface to a trie tree's implementation.
 */

struct trie_node;

/**
 * A branch of a trie tree.
 */
struct trie_branch {
    /**
     * The key of the branch (i.e. the character that leads to this path).
     */
    char key;
    /**
     * A child of this branch's node, associated with the key above.
     */
    struct trie_node *child;
};

/**
 * A list of branches with their keys.
 */
struct trie_branch_list {
    /**
     * The array of branches.
     */
    struct trie_branch *restrict entries;
    /**
     * Number of branches in this list.
     */
    unsigned short length;
    /**
     * How many branches this list is currently able to store.
     */
    unsigned short capacity;
};

/**
 * A node of a trie tree.
 */
struct trie_node {
    /**
     * Whether this node contains a leaf. **DO NOT** touch this field.
     */
    bool has_leaf;
    /**
     * The leaf data, in this case, the ID of a movie. Only initialized if this
     * node contains a leaf. **DO NOT** touch this field.
     */
    moviedb_id movie;
    /**
     * The branches of this node. **DO NOT** touch this field.
     */
    struct trie_branch_list branches;
};

/**
 * Initializes the root of the trie tree.
 */
inline void trie_root_init(struct trie_node *restrict root)
{
    root->has_leaf = false;
    root->branches.entries = NULL;
    root->branches.length = 0;
    root->branches.capacity = 0;
}

/**
 * Inserts a movie ID into the three, given the name of the movie. The only
 * possible error is an allocation error.
 */
void trie_insert(
        struct trie_node *root,
        char const *restrict name,
        moviedb_id movie,
        struct error *error);

/**
 * Searches for a movie in the trie tree with the given movie name.
 *
 * Returns whether the movie was found. If found, movie_out is filled with the
 * found movie's ID.
 */
bool trie_search(
        struct trie_node const *root,
        char const *restrict name,
        moviedb_id *movie_out);

/**
 * Destroys the given trie tree, freeing all the heap-allocated memory. Note
 * that the given pointer to the root node is not assumed to be heap-allocated.
 * One can (and should) allocate the root node in the stack.
 */
void trie_destroy(struct trie_node *root);

#endif
