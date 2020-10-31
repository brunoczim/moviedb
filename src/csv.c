#include <stdbool.h>
#include <string.h>
#include "io.h"
#include "csv.h"
#include "alloc.h"

/**
 * Given a character (symbol) read from a file, updates line and column
 * accordingly. Uses the internal state to handle LF, CRLF and CR line endings.
 */
static void update_line_column(struct csv_parser *restrict parser, int symbol);

extern inline void csv_parser_init(
        struct csv_parser *restrict parser,
        FILE *csv_file);

extern inline bool csv_is_error(struct csv_parser const *restrict parser);

extern inline bool csv_is_row_boundary(
        struct csv_parser const *restrict parser);

extern inline bool csv_is_end_of_file(struct csv_parser const *restrict parser);

double csv_parse_double(
        char const *restrict string,
        struct error *restrict error)
{
    size_t i;
    char *error_string;
    char *end;
    double value;
    
    i = 0;

    while (string[i] == ' ') {
        i++;
    }
   
    value = strtod(string + i, &end);

    while (*end == ' ') {
        end++;
    }

    if (*end != 0) {
        error_string = moviedb_alloc(strlen(string) + 1, error);
        if (error->code == error_none) {
            strcpy(error_string, string);
            error_set_code(error, error_double);
            error->data.double_f.has_line = false;
            error->data.double_f.string = error_string;
            error->data.double_f.free_string = true;
        }
    }

    return value;
}

/**
 * Performs the transtion of states, given a character (symbol) read from a
 * file. Also writes the correct characters to the output buffer.
 */
static void transition(
        struct csv_parser *restrict parser,
        int symbol,
        struct strbuf *restrict out,
        struct error *restrict error);

void csv_parse_field(
        struct csv_parser *restrict parser,
        struct strbuf *restrict out,
        struct error *restrict error)
{
    int symbol;
    bool done = false;

    out->length = 0;

    while (!done) {
        symbol = input_file_read(parser->file, error);
        if (error->code == error_none) {
            update_line_column(parser, symbol);
            transition(parser, symbol, out, error);
        }

        if (error->code != error_none) {
            done = true;
        } else if (parser->state == csv_error) {
            error_set_code(error, error_csv);
            error->data.csv.line = parser->line;
            error->data.csv.column = parser->column;
            done = true;
        } else {
            done = parser->state == csv_comma;
            done = done || csv_is_row_boundary(parser);
            done = done || error->code != error_none;
        }
    }
}

static void update_line_column(struct csv_parser *restrict parser, int symbol)
{
    switch (symbol) {
        case '\n':
            switch (parser->state) {
                case csv_car_return:
                case csv_quoted_cr:
                    break;
                default:
                    parser->line++;
                    parser->column = 1;
                    break;
            }
            break;
        case '\r':
            parser->line++;
            parser->column = 1;
            break;
        default:
            if (symbol != EOF) {
                parser->column++;
            }
            break;
    }
}

static void transition(
        struct csv_parser *restrict parser,
        int symbol,
        struct strbuf *restrict out,
        struct error *restrict error)
{
    switch (parser->state) {
        /* Group of states that begin the reading of a field. */
        case csv_car_return:
            if (symbol == '\n') {
                parser->state = csv_crlf;
                break;
            }
        case csv_initial:
        case csv_comma:
        case csv_linefeed:
        case csv_crlf:
            switch (symbol) {
                case '"':
                    parser->state = csv_quoted;
                    break;
                case '\r':
                    parser->state = csv_car_return;
                    break;
                case '\n':
                    parser->state = csv_linefeed;
                    break;
                case EOF:
                    parser->state = csv_end_of_file;
                    break;
                default:
                    parser->state = csv_unquoted;
                    strbuf_push(out, symbol, error);
                    break;
            }
            break;

        /* State that is reading an unquoted field. */
        case csv_unquoted:
            switch (symbol) {
                case '"':
                    parser->state = csv_error;
                    break;
                case '\r':
                    parser->state = csv_car_return;
                    break;
                case '\n':
                    parser->state = csv_linefeed;
                    break;
                case ',':
                    parser->state = csv_comma;
                    break;
                case EOF:
                    parser->state = csv_end_of_file;
                    break;
                default:
                    strbuf_push(out, symbol, error);
                    break;
            }
            break;

        /*
         * Group of states that is reading a quoted field while a quote does not
         * happen inside the field.
         */
        case csv_quoted_cr:
            parser->state = csv_quoted;
        case csv_quoted:
            switch (symbol) {
                case '"':
                    parser->state = csv_prev_quote;
                    break;
                case EOF:
                    parser->state = csv_error;
                    break;
                case '\\':
                    parser->state = csv_prev_backslash;
                    break;
                case '\r':
                    parser->state = csv_quoted_cr;
                default:
                    strbuf_push(out, symbol, error);
                    break;
            }
            break;

        /*
         * Group of states that is reading a quoted field when a quote happens
         * inside the field.
         */
        case csv_prev_quote:
            switch (symbol) {
                case '"':
                    parser->state = csv_quoted;
                    strbuf_push(out, symbol, error);
                    break;
                case '\r':
                    parser->state = csv_car_return;
                    break;
                case '\n':
                    parser->state = csv_linefeed;
                    break;
                case ',':
                    parser->state = csv_comma;
                    break;
                case EOF:
                    parser->state = csv_end_of_file;
                    break;
                default:
                    parser->state = csv_error;
                    break;
            }
            break;

        /*
         * Group of states that is reading a quoted field when a backslash
         * happens inside the field.
         */
        case csv_prev_backslash:
            switch (symbol) {
                case EOF:
                    parser->state = csv_error;
                    break;
                case '\\':
                    parser->state = csv_prev_backslash;
                    break;
                case '\r':
                    parser->state = csv_quoted_cr;
                    strbuf_push(out, symbol, error);
                    break;
                default:
                    parser->state = csv_quoted;
                    strbuf_push(out, symbol, error);
                    break;
            }
            break;

        /* Group of states that won't change no matter what. */
        case csv_end_of_file:
        case csv_error:
            break;
    }
}
