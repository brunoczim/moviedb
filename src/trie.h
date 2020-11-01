#ifndef MOVIEDB_TRIE_H
#define MOVIEDB_TRIE_H 1

#include <stdbool.h>
#include "error.h"
#include "trie/branch.h"
#include "trie/iter.h"
#include "id.h"

/**
 * This file provides an interface to a trie tree's implementation.
 */

/**
 * A node of a trie tree.
 */
struct trie_node {
    /**
     * Whether this node contains a leaf. Only trie internal code is allowed to
     * touch this.
     */
    bool has_leaf;
    /**
     * The leaf data, in this case, the ID of a movie. Only initialized if this
     * node contains a leaf. Only trie internal code is allowed to touch this.
     */
    db_id_t movie;
    /**
     * The branches of this node. Only trie internal code is allowed to touch
     * this.
     */
    struct trie_branch_list branches;
};

/**
 * Initializes the root of the trie tree.
 */
inline void trie_root_init(struct trie_node *restrict root)
{
    root->has_leaf = false;
    trie_branches_init(&root->branches);
}

/**
 * Inserts a movie ID into the three, given the title of the movie. The only
 * possible error is an allocation error.
 */
void trie_insert(
        struct trie_node *root,
        char const *restrict title,
        db_id_t movie,
        struct error *restrict error);

/**
 * Searches for a movie in the trie tree with the given movie title.
 *
 * Returns whether the movie was found. If found, movie_out is filled with the
 * found movie's ID.
 */
bool trie_search(
        struct trie_node const *root,
        char const *restrict title,
        db_id_t *movie_out);

/**
 * Searches for the movies in the trie tree with the given title prefix.
 *
 * Initializes the output iterator parameter iter_out so it is iterable.
 */
void trie_search_prefix(
        struct trie_node const *root,
        char const *restrict prefix,
        struct trie_iter *iter_out);

/**
 * Destroys the given trie tree, freeing all the heap-allocated memory. Note
 * that the given pointer to the root node is not assumed to be heap-allocated.
 * One can (and should) allocate the root node in the stack.
 */
void trie_destroy(struct trie_node *root);


#endif
