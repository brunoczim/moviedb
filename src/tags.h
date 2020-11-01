#ifndef MOVIEDB_TAGS_H
#define MOVIEDB_TAGS_H 1

#include "id.h"
#include "csv/tag.h"

/**
 * This file exports items related to tags.
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
 * A tag's data.
 */
struct tag {
    /**
     * Name of this tag. Only internal tags hash table code is allowed to update
     * this value. Reading is fine.
     */
    char const *name;
    /**
     * List of the movies associated with this tag. Only internal tags hash
     * table code is allowed to update this value. Reading is fine.
     */
    struct tag_movie_list movies;
};

/**
 * A hash table mapping tag IDs to tags.
 */
struct tags_table {
    /**
     * Array of entries, more specifically array of pointers to tags. Only
     * internal tags hash table code is allowed to touch this value.
     */
    struct tag **entries;
    /**
     * How many elements are stored. Only internal tags hash table code is
     * allowed to touch this value.
     */
    size_t length;
    /**
     * How many elements we can currently store. Only internal tags hash table
     * code is allowed to touch this value.
     */
    size_t capacity;
};

/**
 * Initializes the tag hash table to the given initial capacity. This capacity
 * is rounded up to next prime.
 */
void tags_init(
        struct tags_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error);

/**
 * Inserts the given tag-movie association, creating an entry for the tag in the
 * table if necessary. Name field should be heap-allocated, the given tag_row
 * should not be used after this function is called.
 */
void tags_insert(
        struct tags_table *restrict table,
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error);

/**
 * Searches for a tag's entry in the table. Returns NULL if not found.
 */
struct tag const *tags_search(
        struct tags_table const *restrict table,
        char const *restrict name);

/**
 * Destroys the given tags table, freeing all memory.
 */
void tags_destroy(struct tags_table *restrict table);

#endif
