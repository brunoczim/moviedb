#include "../alloc.h"
#include "rating.h"
#include <string.h>

#define COLUMNS 4

void rating_parser_init(
        struct rating_parser *restrict parser,
        FILE *file,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    bool found_userid = false;
    bool found_movieid = false;
    bool found_value = false;
    bool found_timestamp = false;
    bool found_all;
    bool row_boundary;
    size_t column = 0;

    csv_parser_init(&parser->csv_parser, file);

    do {
        csv_parse_field(&parser->csv_parser, buf, error);

        /* Converts the string buffer into a C String. */
        if (error->code == error_none) {
            strbuf_make_cstr(buf, error);
        }

        if (error->code == error_none) {
            if (strcmp(buf->ptr, "userId") == 0) {
                /* Registers userId column number, does not allow repeat. */
                if (found_userid) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_userid = true;
                    parser->userid_column = column;
                }
            } else if (strcmp(buf->ptr, "movieId") == 0) {
                /* Registers movieId column number, does not allow repeat. */
                if (found_movieid) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_movieid = true;
                    parser->movieid_column = column;
                }
            } else if (strcmp(buf->ptr, "rating") == 0) {
                /* Registers rating column number, does not allow repeat. */
                if (found_value) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_value = true;
                    parser->value_column = column;
                }
            } else if (strcmp(buf->ptr, "timestamp") == 0) {
                /*
                 * Registers timestamp column number, does not allow repeat.
                 * Timestamp is not actually used.
                 */
                if (found_timestamp) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_timestamp = true;
                    parser->timestamp_column = column;
                }
            } else {
                error_set_code(error, error_csv_header);
            }
            /* Accounts next column number. */
            column++;
            /* We will stop at row boundary. */
            row_boundary = csv_is_row_boundary(&parser->csv_parser);
        }
    } while (error->code == error_none && !row_boundary);

    found_all = found_userid && found_movieid && found_value && found_timestamp;

    /* Sets an error if we did not find all of the columns. */
    if (error->code == error_none && !found_all) {
        error_set_code(error, error_csv_header);
    }
}

bool rating_row_parse(
        struct rating_parser *restrict parser,
        struct strbuf *restrict buf,
        struct rating_csv_row *restrict row_out,
        struct error *restrict error)
{
    size_t column = 0;
    bool end_of_file = false;
    bool row_boundary = false;

    row_out->movieid = 0;
    row_out->userid = 0;
    row_out->value = 0.0;

    /*
     * Loops while minimum column number has not been reached (and no error and
     * no eof).
     */
    while (column < COLUMNS && error->code == error_none && !end_of_file) {
        csv_parse_field(&parser->csv_parser, buf, error);
        end_of_file = csv_is_end_of_file(&parser->csv_parser) && column == 0;
        if (!end_of_file && error->code == error_none) {
            row_boundary = csv_is_row_boundary(&parser->csv_parser);
            if (column < COLUMNS - 1 && row_boundary) {
                /* Error if row boundary is found to early. */
                error_set_code(error, error_rating);
                error->data.csv_movie.line = parser->csv_parser.line - 1;
            } else if (column != parser->timestamp_column) {
                /* We will ignore timestamp! */
                strbuf_make_cstr(buf, error);
            }

            if (error->code == error_none) {
                if (column == parser->userid_column) {
                    /* Parses an ID. */
                    row_out->userid = moviedb_id_parse(buf->ptr, error);
                } else if (column == parser->movieid_column) {
                    /* Parses an ID. */
                    row_out->movieid = moviedb_id_parse(buf->ptr, error);
                } else if (column == parser->value_column) {
                    /* Parses a double. */
                    row_out->value = csv_parse_double(buf->ptr, error);
                }
            }
        }
        column++;
    }

    /* We must end the row with a row boundary (duh). */
    if (error->code == error_none && !row_boundary) {
        error_set_code(error, error_movie);
        error->data.csv_movie.line = parser->csv_parser.line;
    }

    /* Gets the line for an ID error. */
    if (error->code == error_id) {
        error->data.id.has_line = true;
        if (csv_is_row_boundary(&parser->csv_parser)) {
            error->data.id.line = parser->csv_parser.line - 1;
        } else {
            error->data.id.line = parser->csv_parser.line;
        }
    }
    return !end_of_file && error->code == error_none;
}
