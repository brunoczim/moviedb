#ifndef MOVIEDB_CSV_H
#define MOVIEDB_CSV_H 1

#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "strbuf.h"

/**
 * Defines CSV parsing utilities.
 */

/**
 * The internal state of a CSV parser. Only internal CSV code uses this. Some
 * states are equivalent, but need to be kept either for documentation purposes
 * or for getting metadata, such as line and columns.
 */
enum csv_state {
    /**
     * State when the end of the file has been found.
     */
    csv_end_of_file,
    /**
     * State when a linefeed (LF, \n) is found outside of a field, and not after
     * a carriage return (CR).
     */
    csv_linefeed,
    /**
     * State when a carriage return (CR, \r) is found outside a field.
     */
    csv_car_return,
    /**
     * Initial state of the parser.
     */
    csv_initial,
    /**
     * State when a comma is found outside a field.
     */
    csv_comma,
    /**
     * State when a linefeed (LF, \n) is found after a carriage return (CR),
     * outside of a field.
     */
    csv_crlf,
    /**
     * State when parsing an unquoted field.
     */
    csv_unquoted,
    /**
     * State when parsing a quoted field, a quote has not been found inside the
     * quoted field yet.
     */
    csv_quoted,
    /**
     * State when parsing a quoted field, the previous character inside the
     * field was a carriage return (CR), and a quote has not been found inside
     * the quoted field yet.
     */
    csv_quoted_cr,
    /**
     * State when parsing a quoted field, and a backslash has been found
     * _inside_ the field.
     */
    csv_quoted_backslash,
    /**
     * State when parsing a quoted field, and a quote has been found _inside_
     * the field.
     */
    csv_prev_quote,
    /**
     * State when parsing a quoted field, and a backslash following a quote has
     * been found _inside_ the field.
     */
    csv_prev_quote_backslash,
    /**
     * State when an error has been found. Line and column number should be
     * checked.
     */
    csv_error
};

/**
 * A CSV parser's data.
 */
struct csv_parser { 
    /**
     * The file from which the parser is reading. While the parser is being
     * used, only CSV parser internal code is allowed to touch this, * including
     * reading from this file pointer.
     */
    FILE *file;
    /**
     * Current line of the file, starting from 1. This can be read, but only
     * CSV parser internal code is allowed to update this.
     */
    unsigned long line;
    /**
     * Current column of the file, starting from 1. This can be read, but only
     * CSV parser internal code is allowed to update this.
     */
    unsigned long column;
    /**
     * Internal state of the parser. Only CSV parser internal code is allowed to
     * touch this.
     */
    enum csv_state state;
};

/**
 * Initializes a parser from the given FILE object. The given FILE must be
 * readable, and the caller should not use the FILE while using the parser.
 */
inline void csv_parser_init(struct csv_parser *restrict parser, FILE *csv_file)
{
    parser->file = csv_file;
    parser->line = 1;
    parser->column = 1;
    parser->state = csv_initial;
}

/**
 * Returns whether an error occured. Line and column should be checked.
 */
inline bool csv_is_error(struct csv_parser const *restrict parser)
{
    return parser->state == csv_error;
}

/**
 * Returns whether the parser is in a row boundary. Essentially, this means a
 * new row starts from now, or a row just ended.
 */
inline bool csv_is_row_boundary(struct csv_parser const *restrict parser)
{
    switch (parser->state) {
        case csv_car_return:
        case csv_initial:
        case csv_linefeed:
        case csv_end_of_file:
            return true;
        default:
            return false;
    }
}

/**
 * Returns whether the end of file has been reached.
 */
inline bool csv_is_end_of_file(struct csv_parser const *restrict parser)
{
    return parser->state == csv_end_of_file;
}

/**
 * Parses a field from the CSV file. If end of file has been reached, this
 * will simply set the output buffer length to 0. In order to disambiguate this
 * from an empty field, one should call `csv_is_end_of_file`.
 *
 * This function **DOES NOT** appends a nul (\0) char to the string. You can use
 * the function strbuf(buf, 0) to append, though.
 *
 * Possible errors are allocation error and IO errors.
 */
void csv_parse_field(
        struct csv_parser *restrict parser,
        struct strbuf *restrict out,
        struct error *restrict error);

/**
 * Parses a double precision floating point number. Intended to be used when
 * parsing CSV, but can be used with anything.
 */
double csv_parse_double(
        char const *restrict string,
        struct error *restrict error);

#endif
