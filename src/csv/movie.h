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
    moviedb_id id;
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
     * The internal CSV parser. Do not touch this!!
     */
    struct csv_parser csv_parser;
    /**
     * Column where the ID will appear. Do not write to this!!
     */
    unsigned char id_column;
    /**
     * Column where the title will appear. Do not write to this!!
     */
    unsigned char title_column;
    /**
     * Column where the genres will appear. Do not write to this!!
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
        struct strbuf *buf,
        struct error *error);

/**
 * Attempts to parse a movie row. Returns whether the row was parsed. Returning
 * false and having no error means EOF.
 */
bool movie_parse_row(
        struct movie_parser *restrict parser,
        struct strbuf *buf,
        struct movie_csv_row *row_out,
        struct error *error);

/**
 * Destroy the contents of a movie row.
 */
void movie_destroy_row(struct movie_csv_row *row);

#endif