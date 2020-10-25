#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../io.h"
#include "../csv.h"
#include "../strbuf.h"
#include "../error.h"

/** 
 * Tests the parser using a set of similar files. Error is never expected.
 */

void test_file(char const *path);

int main(int argc, char const *argv[])
{
    test_file("src/test/csv-lf.csv");
    test_file("src/test/csv-crlf.csv");
    test_file("src/test/csv-cr.csv");

    puts("Ok");

    return 0;
}

void test_file(char const *path)
{
    FILE *file;
    struct strbuf buf;
    struct error error;
    struct csv_parser parser;

    error_init(&error);
    strbuf_init(&buf);

    printf("Testing %s\n", path);
    file = input_file_open(path, &error);
    if (error.code != error_none) {
        error_set_context(&error, path, false);
        error_print(&error);
        error_destroy(&error);
        exit(1);
    }

    csv_parser_init(&parser, file);

    assert(parser.line == 1);
    assert(parser.column == 1);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "abc") == 0);

    assert(parser.line == 1);
    assert(parser.column == 5);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "def") == 0);

    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, " ghj") == 0);

    assert(parser.line == 2);
    assert(parser.column == 1);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "test\nthis") == 0
            || strcmp(buf.ptr, "test\r\nthis") == 0
            || strcmp(buf.ptr, "test\rthis") == 0);

    assert(parser.line == 3);
    assert(parser.column == 7);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "and \"this\"") == 0);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "3") == 0);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "5") == 0);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "this too") == 0);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "") == 0);

    assert(!csv_is_error(&parser));
    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, &buf, &error);
    strbuf_push(&buf, 0, &error);
    assert(error.code == error_none);
    assert(strcmp(buf.ptr, "") == 0);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(csv_is_end_of_file(&parser));
    assert(parser.line == 5);
    assert(parser.column == 1);

    input_file_close(file);

    strbuf_destroy(&buf);
}
