#ifndef MOVIEDB_FMT_H
#define MOVIEDB_FMT_H 1

#include <limits.h>
#include "io.h"
#include "str.h"

#define FMT_INT_BUF_SIZE (sizeof(long long) * CHAR_BIT / 2 + 1)
#define FMT_UINT_BUF_SIZE (sizeof(long long unsigned) * CHAR_BIT / 2)

enum fmt_case {
    fmt_lower,
    fmt_upper
};

enum fmt_writer_kind {
    fmt_writer_file,
    fmt_writer_strbuf
};

union fmt_writer_data {
    FILE *file;
    struct strbuf *restrict strbuf;
};

struct fmt_writer {
    enum fmt_writer_kind kind;
    union fmt_writer_data data;
};

inline struct fmt_writer fmt_writer_from_file(FILE *file);

inline struct fmt_writer fmt_writer_from_strbuf(struct strbuf *restrict strbuf);

inline struct fmt_writer fmt_stdout();

inline struct fmt_writer fmt_stderr();

inline void fmt_write(
        struct fmt_writer writer,
        struct strref data,
        struct error *restrict error);

void fmt_char(
        struct fmt_writer writer,
        char ch,
        struct error *restrict error);

void fmt_int_raw(
        char buf[FMT_INT_BUF_SIZE],
        size_t *restrict start,
        long long integer,
        int base,
        enum fmt_case digit_case);

inline void fmt_int(
        struct fmt_writer writer,
        long long integer,
        struct error *restrict error);

void fmt_int_base(
        struct fmt_writer writer,
        long long integer,
        int base,
        enum fmt_case digit_case,
        struct error *restrict error);

void fmt_uint_raw(
        char buf[FMT_UINT_BUF_SIZE],
        size_t *restrict start,
        long long unsigned integer,
        int base,
        enum fmt_case digit_case);
        

inline void fmt_uint(
        struct fmt_writer writer,
        long long unsigned integer,
        struct error *restrict error);

void fmt_uint_base(
        struct fmt_writer writer,
        long long unsigned integer,
        int base,
        enum fmt_case digit_case,
        struct error *restrict error);

void fmt_quote(
        struct fmt_writer writer,
        struct strref data,
        struct error *restrict error);

inline struct fmt_writer fmt_writer_from_file(FILE *file)
{
    struct fmt_writer writer;
    writer.kind = fmt_writer_file;
    writer.data.file = file;
    return writer;
}

inline struct fmt_writer fmt_writer_from_strbuf(struct strbuf *restrict strbuf)
{
    struct fmt_writer writer;
    writer.kind = fmt_writer_strbuf;
    writer.data.strbuf = strbuf;
    return writer;
}

inline struct fmt_writer fmt_stdout()
{
    return fmt_writer_from_file(stdout);
}

inline struct fmt_writer fmt_stderr()
{
    return fmt_writer_from_file(stderr);
}

inline void fmt_write(
        struct fmt_writer writer,
        struct strref data,
        struct error *restrict error)
{
    switch (writer.kind) {
        case fmt_writer_file:
            io_write(writer.data.file, data, error);
            break;
        case fmt_writer_strbuf:
            strbuf_append(writer.data.strbuf, data, error);
            break;
    }
}

inline void fmt_int(
        struct fmt_writer writer,
        long long integer,
        struct error *restrict error)
{
    fmt_int_base(writer, integer, 10, fmt_lower, error);
}

inline void fmt_uint(
        struct fmt_writer writer,
        long long unsigned integer,
        struct error *restrict error)
{
    fmt_uint_base(writer, integer, 10, fmt_lower, error);
}

#endif
