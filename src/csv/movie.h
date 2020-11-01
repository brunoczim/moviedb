#ifndef MOVIEDB_CSV_MOVIE_H
#define MOVIEDB_CSV_MOVIE_H 1

#include "../csv.h"
#include "../strbuf.h"
#include "../io.h"
#include "../id.h"

/**
 * This file provides items related to movies in the movies CSV file.
 */

/**
 * A movie row in the movies CSV file.
 */
struct movie_csv_row {
    /**
     * ID of the movie.
     */
    db_id_t id;
    /**
     * Title of the movie. Should be heap allocated.
     */
    char const *title;
    /**
     * Genres of the movie. Should be heap allocated.
     */
    char const *genres;
};

/**
 * A movie row parser.
 */
struct movie_parser {
    /**
     * The internal CSV parser. Only movie parser internal code is allowed to
     * touch this.
     */
    struct csv_parser csv_parser;
    /**
     * Column where the ID will appear. Only movie parser internal code is
     * allowed to touch this.
     */
    unsigned char id_column;
    /**
     * Column where the title will appear. Only movie parser internal code is
     * allowed to touch this.
     */
    unsigned char title_column;
    /**
     * Column where the genres will appear. Only movie parser internal code is
     * allowed to touch this.
     */
    unsigned char genres_column;
};

/**
 * Initializes the movie parser. The file is usable after you are finished with
 * the parser.
 */
void movie_parser_init(
        struct movie_parser *restrict parser,
        FILE *file,
        struct strbuf *restrict buf,
        struct error *restrict error);

/**
 * Attempts to parse a movie row. Returns whether the row was parsed. Returning
 * false and having no error means EOF.
 */
bool movie_row_parse(
        struct movie_parser *restrict parser,
        struct strbuf *restrict buf,
        struct movie_csv_row *restrict row_out,
        struct error *restrict error);

/**
 * Destroy the contents of a movie row.
 */
void movie_row_destroy(struct movie_csv_row *restrict row);

#endif
