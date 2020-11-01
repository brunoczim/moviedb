#ifndef MOVIEDB_USERS_H
#define MOVIEDB_USERS_H 1

#include "id.h"
#include "csv/rating.h"

/**
 * This file exports items related to users.
 */

/**
 * Rating given by a user.
 */
struct user_rating {
    /**
     * Value of the rating given by the user to a movie. Only internal users
     * hash table code is allowed to update this value. Reading is fine.
     */
    double value;
    /**
     * The movie that was rated. Only internal users hash table code is allowed
     * to update this value. Reading is fine.
     */
    db_id_t movie;
};

/**
 * The ratings given by a user.
 */
struct user_rating_list {
    /**
     * Array of entries. Only internal users hash table code is allowed
     * to update this value. Reading is fine.
     */
    struct user_rating *entries;
    /**
     * How much entries are stored here. Only internal users hash table code is
     * allowed to update this value. Reading is fine.
     */
    size_t length;
    /**
     * How many entries we can currently store. Only internal users hash table
     * code is allowed to touch this value.
     */
    size_t capacity;
};

/**
 * A user's data.
 */
struct user {
    /**
     * ID of the user. Only internal users hash table code is allowed to update
     * this value. Reading is fine.
     */
    db_id_t id;
    /**
     * List of the ratings made by this user. Only internal users hash table
     * code is allowed to update this value. Reading is fine.
     */
    struct user_rating_list ratings;
};

/**
 * A hash table mapping user IDs to users.
 */
struct users_table {
    /**
     * Array of entries, more specifically array of pointers to users. Only
     * internal users hash table code is allowed to touch this value.
     */
    struct user **entries;
    /**
     * How many elements are stored. Only internal users hash table code is
     * allowed to touch this value.
     */
    size_t length;
    /**
     * How many elements we can currently store. Only internal users hash table
     * code is allowed to touch this value.
     */
    size_t capacity;
};

/**
 * Initializes the user hash table to the given initial capacity. This capacity
 * is rounded up to next prime.
 */
void users_init(
        struct users_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error);

/**
 * Inserts the given rating made by the given user, creating an entry for the
 * user in the table if necessary.
 */
void users_insert_rating(
        struct users_table *restrict table,
        struct rating_csv_row *restrict rating_row,
        struct error *restrict error);

/**
 * Searches for a user's entry in the table. Returns NULL if not found.
 */
struct user const *users_search(
        struct users_table const *restrict table,
        db_id_t userid);

/**
 * Destroys the given users table, freeing all memory.
 */
void users_destroy(struct users_table *restrict table);

#endif
