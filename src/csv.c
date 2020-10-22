#include <stdbool.h>
#include "csv.h"

extern inline void csv_parser_init(struct csv_parser *parser, FILE *csv_file);

extern inline bool csv_is_error(struct csv_parser const *parser);

extern inline bool csv_is_row_boundary(struct csv_parser const *parser);

extern inline bool csv_is_end_of_file(struct csv_parser const *parser);

/**
 * Given a character (symbol) read from a file, updates line and column
 * accordingly. Uses the internal state to handle LF, CRLF and CR line endings.
 */
static void update_line_column(struct csv_parser *parser, int symbol);

/**
 * Performs the transtion of states, given a character (symbol) read from a
 * file. Also writes the correct characters to the output buffer.
 */
static void transition(
        struct csv_parser *parser,
        int symbol,
        struct strbuf *out);

void csv_parse_field(struct csv_parser *parser, struct strbuf *out)
{
    int symbol;

    out->length = 0;

    do {
        symbol = fgetc(parser->file);
        update_line_column(parser, symbol);
        transition(parser, symbol, out);
    } while (parser->state != csv_comma && !csv_is_row_boundary(parser));
}

static void update_line_column(struct csv_parser *parser, int symbol)
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
        struct csv_parser *parser,
        int symbol,
        struct strbuf *out)
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
                    strbuf_push(out, symbol);
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
                    strbuf_push(out, symbol);
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
                case '\r':
                    parser->state = csv_quoted_cr;
                default:
                    strbuf_push(out, symbol);
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
                    strbuf_push(out, symbol);
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

        /* Group of states that won't change no matter what. */
        case csv_end_of_file:
        case csv_error:
            break;
    }
}
