#ifndef MOVIEDB_CSV_RATING_H
#define MOVIEDB_CSV_RATING_H 1

#include "../csv.h"
#include "../strbuf.h"
#include "../io.h"
#include "../id.h"

/**
 * This file provides items related to ratings in the ratings CSV file.
 */

/**
 * A rating row in the ratings CSV file.
 */
struct rating_csv_row {
    moviedb_id_t userid;
    moviedb_id_t movieid;
    double value;
};

struct rating_parser {
    struct csv_parser csv_parser;
    unsigned char userid_column;
    unsigned char movieid_column;
    unsigned char value_column;
    unsigned char timestamp_column;
};

void rating_parser_init(
        struct rating_parser *restrict parser,
        FILE *file,
        struct strbuf *restrict buf,
        struct error *restrict error);

/**
 * Attempts to parse a rating row. Returns whether the row was parsed. Returning
 * false and having no error means EOF.
 */
bool rating_row_parse(
        struct rating_parser *restrict parser,
        struct strbuf *restrict buf,
        struct rating_csv_row *restrict row_out,
        struct error *restrict error);

#endif
