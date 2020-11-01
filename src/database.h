#ifndef MOVIEDB_DB_H
#define MOVIEDB_DB_H 1

#include "error.h"
#include "strbuf.h"
#include "trie.h"
#include "movies.h"
#include "users.h"
#include "tags.h"

/**
 * This file exports items to operate on the whole movie database.
 */

/**
 * All data structures of the movie database.
 */
struct database {
    /**
     * The trie mapping movie name -> movie id.
     */
    struct trie_node trie_root;
    /**
     * The hash table mapping movie id -> movie data.
     */
    struct movies_table movies;
    /**
     * The hash table mapping user id -> user data (their ratings).
     */
    struct users_table users;
    /**
     * The hash table mapping user tag name -> tag data (associated movies).
     */
    struct tags_table tags;
};

/**
 * Initializes and loads a database. database_out should not be initialized, but
 * buf and error should.
 */
void database_load(
        struct database *restrict database_out,
        struct strbuf *restrict buf,
        struct error *restrict error);

/**
 * Destroys the database, by destroying every data structure it holds.
 */
void database_destroy(struct database *restrict database);

#endif
