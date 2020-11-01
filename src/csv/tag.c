#include "../alloc.h"
#include "tag.h"
#include <string.h>

#define COLUMNS 4

void tag_parser_init(
        struct tag_parser *restrict parser,
        FILE *file,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    bool found_userid = false;
    bool found_movieid = false;
    bool found_name = false;
    bool found_timestamp = false;
    bool found_all;
    bool row_boundary;
    size_t column = 0;

    csv_parser_init(&parser->csv_parser, file);

    do {
        csv_parse_field(&parser->csv_parser, buf, error);

        if (error->code == error_none) {
            strbuf_make_cstr(buf, error);
        }

        if (error->code == error_none) {
            if (strcmp(buf->ptr, "userId") == 0) {
                if (found_userid) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_userid = true;
                    parser->userid_column = column;
                }
            } else if (strcmp(buf->ptr, "movieId") == 0) {
                if (found_movieid) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_movieid = true;
                    parser->movieid_column = column;
                }
            } else if (strcmp(buf->ptr, "tag") == 0) {
                if (found_name) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_name = true;
                    parser->name_column = column;
                }
            } else if (strcmp(buf->ptr, "timestamp") == 0) {
                if (found_timestamp) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_timestamp = true;
                    parser->timestamp_column = column;
                }
            } else {
                error_set_code(error, error_csv_header);
            }
            column++;
            row_boundary = csv_is_row_boundary(&parser->csv_parser);
        }
    } while (error->code == error_none && !row_boundary);

    found_all = found_userid && found_movieid && found_name && found_timestamp;

    if (error->code == error_none && !found_all) {
        error_set_code(error, error_csv_header);
    }
}

bool tag_row_parse(
        struct tag_parser *restrict parser,
        struct strbuf *restrict buf,
        struct tag_csv_row *restrict row_out,
        struct error *restrict error)
{
    size_t column = 0;
    bool end_of_file = false;
    bool row_boundary = false;

    row_out->id = 0;
    row_out->title = NULL;
    row_out->genres = NULL;

    while (column < COLUMNS && error->code == error_none && !end_of_file) {
        csv_parse_field(&parser->csv_parser, buf, error);
        end_of_file = csv_is_end_of_file(&parser->csv_parser) && column == 0;
        if (!end_of_file && error->code == error_none) {
            row_boundary = csv_is_row_boundary(&parser->csv_parser);
            if (column < COLUMNS - 1 && row_boundary) {
                error_set_code(error, error_tag);
                error->data.csv_tag.line = parser->csv_parser.line - 1;
            } else if (column == parser->movieid_column) {
                strbuf_make_cstr(buf, error);
                if (error->code == error_none) {
                    row_out->movie = moviedb_id_parse(buf->ptr, error);
                }
            } else if (column == parser->name_column) {
                row_out->name = strbuf_copy_cstr(buf, error);
            }
            /* userid and timestamp ignored */
        }
        column++;
    }

    if (error->code != error_none) {
        tag_row_destroy(row_out);
        if (error->code == error_id) {
            error->data.id.has_line = true;
            if (csv_is_row_boundary(&parser->csv_parser)) {
                error->data.id.line = parser->csv_parser.line - 1;
            } else {
                error->data.id.line = parser->csv_parser.line;
            }
        }
    } else if (!csv_is_row_boundary(&parser->csv_parser)) {
        error_set_code(error, error_tag);
        error->data.csv_tag.line = parser->csv_parser.line;
    }

    return !end_of_file && error->code == error_none;
}

void tag_row_destroy(struct tag_csv_row *restrict row)
{
    moviedb_free((void *) (void const *) row->name);
}