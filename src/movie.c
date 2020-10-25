#include "alloc.h"
#include "movie.h"

#define COLUMNS 3

void movie_parser_init(
        struct movie_parser *restrict parser,
        FILE *file,
        struct strbuf *buf,
        struct error *error)
{
    bool found_id = false, found_title = false, found_genres = false;
    bool row_boundary;
    size_t column = 0;

    csv_parser_init(&parser->csv_parser, file);

    do {
        csv_parse_field(&parser->csv_parser, buf, error);
        if (error->code == error_none) {
            if (strbuf_icmp_cstr(buf, "movieid") == 0) {
                if (found_id) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_id = true;
                    parser->id_column = column;
                }
            } else if (strbuf_icmp_cstr(buf, "title") == 0) {
                if (found_title) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_title = true;
                    parser->title_column = column;
                }
            } else if (strbuf_icmp_cstr(buf, "genres") == 0) {
                if (found_genres) {
                    error_set_code(error, error_csv_header);
                } else {
                    found_genres = true;
                    parser->genres_column = column;
                }
            } else {
                error_set_code(error, error_csv_header);
            }
            column++;
            row_boundary = csv_is_row_boundary(&parser->csv_parser);
        }
    } while (error->code == error_none && !row_boundary);
}

bool movie_parse_row(
        struct movie_parser *restrict parser,
        struct strbuf *buf,
        struct movie_csv_row *row_out,
        struct error *error)
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
                error_set_code(error, error_movie);
                error->data.movie.line = parser->csv_parser.line - 1;
            } else if (column == parser->id_column) {
                row_out->id = moviedb_id_parse(buf, error);
            } else if (column == parser->title_column) {
                row_out->title = strbuf_make_cstr(buf, error);
            } else if (column == parser->genres_column) {
                row_out->genres= strbuf_make_cstr(buf, error);
            }
        }
        column++;
    }

    if (error->code != error_none) {
        free((void *) (void const *) row_out->title);
        free((void *) (void const *) row_out->genres);
        if (error->code == error_id) {
            error->data.id.has_line = true;
            if (csv_is_row_boundary(&parser->csv_parser)) {
                error->data.id.line = parser->csv_parser.line - 1;
            } else {
                error->data.id.line = parser->csv_parser.line;
            }
        }
    } else if (!csv_is_row_boundary(&parser->csv_parser)) {
        error_set_code(error, error_movie);
        error->data.movie.line = parser->csv_parser.line;
    }

    return !end_of_file && error->code == error_none;
}

void movie_destroy_row(struct movie_csv_row *row)
{
    moviedb_free((void *) (void const *) row->title);
    moviedb_free((void *) (void const *) row->genres);
}
