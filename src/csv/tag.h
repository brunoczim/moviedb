#ifndef MOVIEDB_CSV_TAG_H
#define MOVIEDB_CSV_TAG_H 1

#include "../csv.h"
#include "../strbuf.h"
#include "../io.h"
#include "../id.h"

/**
 * This file provides items related to tags in the tags CSV file.
 */

/**
 * A tag row in the tags CSV file.
 */
struct tag_csv_row {
    /**
     * ID of the movie this tag refers to.
     */
    moviedb_id_t movieid;
    /**
     * Content of the tag. Should be heap allocated.
     */
    char const *name;
};

/**
 * A tag row parser.
 */
struct tag_parser {
    /**
     * The internal CSV parser. Only tag parser internal code is allowed to
     * touch this.
     */
    struct csv_parser csv_parser;
    /**
     * Column where the user ID will appear. Only tag parser internal code is
     * allowed to touch this.
     */
    unsigned char userid_column;
    /**
     * Column where the movie ID will appear. Only tag parser internal code is
     * allowed to touch this.
     */
    unsigned char movieid_column;
    /**
     * Column where the tag name will appear. Only tag parser internal code is
     * allowed to touch this.
     */
    unsigned char name_column;
    /**
     * Column where the timestamp will appear. Only tag parser internal code is
     * allowed to touch this.
     */
    unsigned char timestamp_column;
};

/**
 * Initializes the tag parser. The file is usable after you are finished with
 * the parser.
 */
void tag_parser_init(
        struct tag_parser *restrict parser,
        FILE *file,
        struct strbuf *restrict buf,
        struct error *restrict error);

/**
 * Attempts to parse a tag row. Returns whether the row was parsed. Returning
 * false and having no error means EOF.
 */
bool tag_row_parse(
        struct tag_parser *restrict parser,
        struct strbuf *restrict buf,
        struct tag_csv_row *restrict row_out,
        struct error *restrict error);

/**
 * Destroy the names of a tag row.
 */
void tag_row_destroy(struct tag_csv_row *restrict row);

#endif
