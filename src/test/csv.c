#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../csv.h"
#include "../strbuf.h"

void test_file(char const *path, struct strbuf *buf);

int main(int argc, char const *argv[])
{
    struct strbuf buf;
    strbuf_init(&buf);

    test_file("src/test/csv-lf.csv", &buf);
    test_file("src/test/csv-crlf.csv", &buf);
    test_file("src/test/csv-cr.csv", &buf);

    puts("Ok");

    return 0;
}

void test_file(char const *path, struct strbuf *buf)
{
    FILE *file;
    struct csv_parser parser;

    printf("Testing %s\n", path);
    file = fopen(path, "r");
    if (file == NULL) {
        perror(path);
        exit(1);
    }

    csv_parser_init(&parser, file);

    assert(parser.line == 1);
    assert(parser.col == 0);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, "abc") == 0);

    assert(parser.line == 1);
    assert(parser.col == 4);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, "def") == 0);

    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, " ghj") == 0);

    assert(parser.line == 2);
    assert(parser.col == 0);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, "test\nthis") == 0
            || strcmp(buf->ptr, "test\r\nthis") == 0
            || strcmp(buf->ptr, "test\rthis") == 0);

    assert(parser.line == 3);
    assert(parser.col == 6);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, "and \"this\"") == 0);

    assert(!csv_is_error(&parser));
    assert(!csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, "3") == 0);

    assert(!csv_is_error(&parser));
    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(!csv_is_end_of_file(&parser));
    csv_parse_field(&parser, buf);
    strbuf_push(buf, 0);
    assert(strcmp(buf->ptr, "") == 0);

    assert(!csv_is_error(&parser));
    assert(csv_is_row_boundary(&parser));
    assert(csv_is_end_of_file(&parser));
    assert(parser.line == 4);
    assert(parser.col == 0);

    fclose(file);
}
