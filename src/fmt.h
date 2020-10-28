#ifndef MOVIEDB_FMT_H
#define MOVIEDB_FMT_H 1

#include <stdfmt.h>

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

inline fmt_writer fmt_writer_from_file(FILE *file)
{
    struct fmt_writer writer;
    writer.kind = fmt_writer_file;
    writer.file = file;
    return writer;
}

inline fmt_writer fmt_writer_from_strbuf(struct strbuf *restrict strbuf)
{
    struct fmt_writer writer;
    writer.kind = fmt_writer_strbuf;
    writer.strbuf = strbuf;
    return writer;
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

#endif
