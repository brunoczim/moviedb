#ifndef MOVIEDB_TAGS_MOVIES_H
#define MOVIEDB_TAGS_MOVIES_H 1

#include "../id.h"
#include "../alloc.h"

/**
 * This file provides utilites related to movie lists of tags.
 */

/**
 * The movies associated with a tag.
 */
struct tag_movie_list {
    /**
     * Array of entries. Only internal tags hash table code is allowed
     * to update this value. Reading is fine.
     */
    db_id_t *entries;
    /**
     * How much entries are stored here. Only internal tags hash table code is
     * allowed to update this value. Reading is fine.
     */
    size_t length;
    /**
     * How many entries we can currently store. Only internal tags hash table
     * code is allowed to touch this value.
     */
    size_t capacity;
};

/**
 * Range of a list in a sort implementation. Only tag movies sort internal code
 * is allowed to touch this.
 */
struct tag_movies_sort_range {
    /**
     * Start index, inclusive.
     */
    size_t start;
    /**
     * End index, exclusive.
     */
    size_t end;
};

/**
 * Stack used by the quicksort-related functions.
 */
struct tag_movies_sort_stack {
    /**
     * The elements of the stack. Only tag movies sort internal code is allowed
     * to touch this.
     */
    struct tag_movies_sort_range *ranges;
    /**
     * Number of elements in the stack. Only tag movies sort internal code is
     * allowed to touch this.
     */
    size_t length;
    /**
     * How many elements the stack can store. Only tag movies sort internal code
     * is allowed to touch this.
     */
    size_t capacity;
};

/**
 * Tests if the given movie ID is in the movies list.
 *
 * ASSUMES THE LIST IS SORTED.
 */
bool tag_movies_contain(
        struct tag_movie_list const *restrict movies,
        db_id_t movieid);

/**
 * Sorts the given list of movies.
 */
void tag_movies_sort(
        struct tag_movie_list *restrict list,
        struct tag_movies_sort_stack *restrict stack,
        struct error *restrict error);

/**
 * Initalizes the sort stack with the given capacity.
 */
void tag_movies_sort_init(
        struct tag_movies_sort_stack *restrict stack,
        size_t capacity,
        struct error *error);

/**
 * Destroys the sort stack.
 */
inline void tag_movies_sort_destroy(
        struct tag_movies_sort_stack *restrict stack)
{
    db_free(stack->ranges);
}

#endif
