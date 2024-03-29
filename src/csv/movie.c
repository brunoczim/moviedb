#include "../alloc.h"
#include "movie.h"
#include <string.h>

#define COLUMNS 3

void movie_parser_init(
        struct movie_parser *restrict parser,
        FILE *file,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    bool found_id = false, found_title = false, found_genres = false;
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
            if (strcmp(buf->ptr, "movieId") == 0) {
                /* Registers movieId column number, does not allow repeat. */
                if (found_id) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_id = true;
                    parser->id_column = column;
                }
            } else if (strcmp(buf->ptr, "title") == 0) {
                /* Registers title column number, does not allow repeat. */
                if (found_title) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_title = true;
                    parser->title_column = column;
                }
            } else if (strcmp(buf->ptr, "genres") == 0) {
                /* Registers genres column number, does not allow repeat. */
                if (found_genres) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_genres = true;
                    parser->genres_column = column;
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

    found_all = found_id && found_title && found_genres;

    /* Sets an error if we did not find all of the columns. */
    if (error->code == error_none && !found_all) {
        error_set_code(error, error_csv_header);
    }
}

bool movie_row_parse(
        struct movie_parser *restrict parser,
        struct strbuf *restrict buf,
        struct movie_csv_row *restrict row_out,
        struct error *restrict error)
{
    size_t column = 0;
    bool end_of_file = false;
    bool row_boundary = false;

    row_out->id = 0;
    row_out->title = NULL;
    row_out->genres = NULL;

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
                error_set_code(error, error_movie);
                error->data.csv_movie.line = parser->csv_parser.line - 1;
            } else if (column == parser->id_column) {
                /* Parses an ID. */
                strbuf_make_cstr(buf, error);
                if (error->code == error_none) {
                    row_out->id = moviedb_id_parse(buf->ptr, error);
                }
            } else if (column == parser->title_column) {
                /* Copies the title. */
                row_out->title = strbuf_copy_cstr(buf, error);
            } else if (column == parser->genres_column) {
                /* Copies the genres. */
                row_out->genres = strbuf_copy_cstr(buf, error);
            }
        }
        column++;
    }

    row_boundary = csv_is_row_boundary(&parser->csv_parser);

    /* We must end the row with a row boundary (duh). */
    if (error->code == error_none && !row_boundary) {
        error_set_code(error, error_movie);
        error->data.csv_movie.line = parser->csv_parser.line;
    }

    /* If error, destroys row data. */
    if (error->code != error_none) {
        movie_row_destroy(row_out);
        if (error->code == error_id) {
            /* Gets the line for an ID error. */
            error->data.id.has_line = true;
            if (csv_is_row_boundary(&parser->csv_parser)) {
                error->data.id.line = parser->csv_parser.line - 1;
            } else {
                error->data.id.line = parser->csv_parser.line;
            }
        }
    }
    return !end_of_file && error->code == error_none;
}

void movie_row_destroy(struct movie_csv_row *restrict row)
{
    moviedb_free((void *) (void const *) row->title);
    moviedb_free((void *) (void const *) row->genres);
}
