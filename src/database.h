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
 * A row of the "user" query.
 */
struct user_query_row {
    /**
     * The rating given by the specific user.
     */
    double user_rating;
    /**
     * Title of the rated movie.
     */
    char const *title;
    /**
     * Global rating of the rated movie, i.e. mean of all rates given to the
     * movie.
     */
    double global_rating;
    /**
     * Number of ratings given to the movie.
     */
    size_t ratings;
};

/**
 * Iterator over the rows of a "user" query.
 */
struct user_query_iter {
    /**
     * The database being looked at. Only internal database code is allowed to
     * touch this.
     */
    struct database const *database;
    /**
     * The user whose ratings are being iterated over. Only internal database
     * code is allowed to touch this.
     */
    struct user const *user;
    /**
     * Current rating being iterated. Only internal database code is allowed to
     * touch this.
     */
    size_t current;
};

/**
 * Buffer used by the topN query.
 */
struct topn_query_buf {
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
     * How many rows can be stored. This likely won't change. Only internal
     * database code is allowed to touch this.
     */
    size_t capacity;
};

struct tags_query_input {
    struct tag const **tags;
    size_t length;
    size_t capacity;
};

/**
 * Buffer used by the tags query.
 */
struct tags_query_buf {
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
     * How many rows can be stored. This likely won't change. Only internal
     * database code is allowed to touch this.
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
void movie_query_print_row(struct movie const *restrict row);

/**
 * Prints a header and the rows found in the movie query.
 */
void movie_query_print(struct movie_query_buf const *restrict query_buf);

/**
 * Destroys the movie query buffer.
 */
inline void movie_query_destroy(struct movie_query_buf *restrict buf)
{
    db_free(buf->rows);
}

/**
 * Initializes the user query's row iterator.
 */
void user_query_init(
        struct user_query_iter *restrict iter_out,
        struct database const *database,
        db_id_t userid);

/**
 * Builds the next row and returns whether there is a next row. If there is a
 * next row, it is placed at the row_out parameter.
 */
bool user_query_next(
        struct user_query_iter *restrict iter_out,
        struct user_query_row *restrict row_out);

/**
 * Prints a user query's header to the screen.
 */
void user_query_print_header(void);

/**
 * Prints a user query's row to the screen.
 */
void user_query_print_row(struct user_query_row const *restrict row);

/**
 * Iterates through the user query rows and print them. Prints a header too.
 */
void user_query_print(struct user_query_iter *restrict iter);

/**
 * Initializes the topN query buffer to the given capacity. The query will NOT
 * increase the capacity, it is intended to return only the N best.
 */
void topn_query_init(
        struct topn_query_buf *restrict buf,
        size_t capacity,
        struct error *restrict error);

/**
 * Performs the topN query. Searches for the best rated movies of the given
 * genre and with at least min_ratings count of ratings. The buffer must be
 * initalized and might be reused before being destroyed.
 */
void topn_query(
        struct database const *restrict database,
        char const *restrict genre,
        size_t min_ratings,
        struct topn_query_buf *restrict query_buf);

/**
 * Prints a topN query's header to the screen.
 */
void topn_query_print_header(void);

/**
 * Prints a topN query's row to the screen.
 */
void topn_query_print_row(struct movie const *restrict row);

/**
 * Prints a header and the rows found in the topN query.
 */
void topn_query_print(struct topn_query_buf const *restrict query_buf);

/**
 * Destroys the buffer of a topN query.
 */
inline void topn_query_destroy(struct topn_query_buf *restrict buf)
{
    db_free(buf->rows);
}

void tags_query_input_init(
        struct tags_query_input *restrict query_input,
        size_t capacity,
        struct error *restrict error);

void tags_query_input_add(
        struct tags_query_input *restrict query_input,
        struct database const *restrict database,
        char const *restrict name,
        struct error *restrict error);


inline void tags_query_input_destroy(
        struct tags_query_input *restrict query_input)
{
    db_free(query_input->tags);
}

/**
 * Initializes the tags query buffer to the given capacity. The query will NOT
 * increase the capacity, it is intended to return only the N best.
 */
inline void tags_query_init(struct tags_query_buf *restrict buf)
{
    buf->rows = NULL;
    buf->capacity = 0;
    buf->length = 0;
}

/**
 * Performs the tags query. Searches for the best rated movies of the given
 * genre and with at least min_ratings count of ratings. The buffer must be
 * initalized and might be reused before being destroyed.
 */
void tags_query(
        struct database const *restrict database,
        struct tags_query_input const *restrict query_input,
        struct tags_query_buf *restrict query_buf,
        struct error *restrict error);

/**
 * Prints a tags query's header to the screen.
 */
void tags_query_print_header(void);

/**
 * Prints a tags query's row to the screen.
 */
void tags_query_print_row(struct movie const *restrict row);

/**
 * Prints a header and the rows found in the tags query.
 */
void tags_query_print(struct tags_query_buf const *restrict query_buf);

/**
 * Destroys the buffer of a tags query.
 */
inline void tags_query_destroy(struct tags_query_buf *restrict buf)
{
    db_free(buf->rows);
}

#endif
