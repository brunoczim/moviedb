#ifndef TOPNDB_QUERY_TOPN_H
#define TOPNDB_QUERY_TOPN_H 1

#include "../database.h"

/**
 * This file declares utilities related to the 'topN' query.
 */

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


#endif
