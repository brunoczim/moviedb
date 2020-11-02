#ifndef MOVIEDB_DB_H
#define MOVIEDB_DB_H 1

#include "error.h"
#include "alloc.h"
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
 * Buffer to store the result of a movie query.
 */
struct movie_query_buf {
    /**
     * The pointer to pointers to rows, i.e. array of pointers to rows. Only
     * internal database code is allowed to write to this. Reading is fine.
     */
    struct movie const **rows;
    /**
     * How many rows the query returned. Only internal database code is allowed
     * to write to this. Reading is fine.
     */
    size_t length;
    /**
     * How many rows can be stored. Only internal database code is allowed to
     * touch this.
     */
    size_t capacity;
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

/**
 * Initializes a movie query's buffer.
 */
inline void movie_query_init(struct movie_query_buf *restrict buf)
{
    buf->rows = NULL;
    buf->length = 0;
    buf->capacity = 0;
}

/**
 * Executes a movie query. The movie query returns all the movies with the given
 * prefix in their names, sorted by ID. query_buf must be initialized, and
 * might even contain data, but the data will be overwritten with the query
 * result.
 */
void movie_query(
        struct database const *restrict database,
        char const *restrict prefix,
        struct movie_query_buf *restrict query_buf,
        struct error *restrict error);

/**
 * Prints a movie query's header to the screen.
 */
void movie_query_print_header(void);

/**
 * Prints a movie query's row to the screen.
 */
void movie_query_print_row(struct movie const *restrict movie);

/**
 * Prints the rows found in the query.
 */
void movie_query_print(struct movie_query_buf const *restrict query_buf);

/**
 * Destroys the movie query buffer.
 */
inline void movie_query_destroy(struct movie_query_buf *restrict buf)
{
    db_free(buf->rows);
}

#endif
