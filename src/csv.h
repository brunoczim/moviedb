#ifndef MOVBASE_CSV_H
#define MOVBASE_CSV_H 1

#include <stdio.h>
#include <stdbool.h>
#include "strbuf.h"

enum csv_state {
    csv_car_return,
    csv_initial,
    csv_comma,
    csv_linefeed,
    csv_crlf,
    csv_unquoted,
    csv_quoted,
    csv_prev_quote,
    csv_end_of_file,
    csv_error
};

struct csv_parser {
    FILE *file;
    unsigned long line;
    unsigned long col;
    enum csv_state state;
};

inline void csv_parser_init(struct csv_parser *parser, FILE *csv_file)
{
    parser->file = csv_file;
    parser->line = 1;
    parser->col = 0;
    parser->state = csv_initial;
}

inline bool csv_is_row_boundary(struct csv_parser *parser)
{
    switch (parser->state) {
        case csv_initial:
        case csv_linefeed:
        case csv_car_return:
        case csv_end_of_file:
            return true;
        default:
            return false;
    }
}

inline bool csv_is_end_of_file(struct csv_parser *parser)
{
    return parser->state == csv_end_of_file;
}

void csv_parse_field(struct csv_parser *parser, struct strbuf *out);

#endif
