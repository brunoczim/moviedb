#include "fmt.h"

static inline char digit_to_char(int digit, enum fmt_case digit_case);

extern inline struct fmt_writer fmt_writer_from_file(FILE *file);

extern inline struct fmt_writer fmt_writer_from_strbuf(
        struct strbuf *restrict strbuf);

extern inline struct fmt_writer fmt_stdout();

extern inline struct fmt_writer fmt_stderr();

inline void fmt_write(
        struct fmt_writer writer,
        struct strref data,
        struct error *restrict error);

void fmt_char(
        struct fmt_writer writer,
        char ch,
        struct error *restrict error)
{
    char buffer[1] = { ch };
    struct strref ref = strref_init(buffer, 1);
    fmt_write(writer, ref, error);
}

void fmt_int_raw(
        char buffer[FMT_INT_BUF_SIZE],
        size_t *restrict start,
        long long integer,
        int base,
        enum fmt_case digit_case)
{
    bool negative = integer < 0;

    if (negative) {
        integer = -integer;
    }

    *start = FMT_INT_BUF_SIZE;

    if (base < 2) {
        base = 2;
    } else if (base > 36) {
        base = 36;
    }

    do {
        (*start)--;
        buffer[*start] = digit_to_char(integer % base, digit_case);        
        integer = integer / base;
    } while (integer > 0);

    if (negative) {
        (*start)--;
        buffer[*start] = '-';
    }
}

extern inline void fmt_int(
        struct fmt_writer writer,
        long long integer,
        struct error *restrict error);

void fmt_int_base(
        struct fmt_writer writer,
        long long integer,
        int base,
        enum fmt_case digit_case,
        struct error *restrict error)
{
    char buffer[FMT_INT_BUF_SIZE];
    size_t start;
    struct strref ref;

    fmt_int_raw(buffer, &start, integer, base, digit_case);
    ref = strref_init(buffer + start, FMT_INT_BUF_SIZE - start);
    fmt_write(writer, ref, error);
}

void fmt_uint_raw(
        char buffer[FMT_UINT_BUF_SIZE],
        size_t *restrict start,
        long long unsigned integer,
        int base,
        enum fmt_case digit_case)
{
    *start = FMT_UINT_BUF_SIZE;

    if (base < 2) {
        base = 2;
    } else if (base > 36) {
        base = 36;
    }

    do {
        (*start)--;
        buffer[*start] = digit_to_char(integer % base, digit_case);        
        integer = integer / base;
    } while (integer > 0);
}

extern inline void fmt_uint(
        struct fmt_writer writer,
        long long unsigned integer,
        struct error *restrict error);

void fmt_uint_base(
        struct fmt_writer writer,
        long long unsigned integer,
        int base,
        enum fmt_case digit_case,
        struct error *restrict error)
{
    char buffer[FMT_UINT_BUF_SIZE];
    size_t start;
    struct strref ref;

    fmt_uint_raw(buffer, &start, integer, base, digit_case);
    ref = strref_init(buffer + start, FMT_UINT_BUF_SIZE - start);
    fmt_write(writer, ref, error);
}

void fmt_quote(
        struct fmt_writer writer,
        struct strref data,
        struct error *restrict error)
{
    size_t i = 0;

    fmt_write(writer, strref_lit("\""), error);

    while (i < data.length && error->code == error_none) {
        switch (data.chars[i]) {
            case '\n':
                fmt_write(writer, strref_lit("\\n"), error);
                break;
            case '\r':
                fmt_write(writer, strref_lit("\\r"), error);
                break;
            case '"':
                fmt_write(writer, strref_lit("\\\""), error);
                break;
            case '\\':
                fmt_write(writer, strref_lit("\\\\"), error);
                break;
            default:
                if (data.chars[i] >= 32 && data.chars[i] <= 126) {
                    fmt_write(writer, strref_range(data, i, i + 1), error);
                } else {
                    fmt_uint(writer, data.chars[i], error);
                }
                break;
        }
        i++;
    } 

    if (error->code == error_none) {
        fmt_write(writer, strref_lit("\""), error);
    }
}

static inline char digit_to_char(int digit, enum fmt_case digit_case)
{
    if (digit < 10) {
        return digit + '0';
    }

    if (digit_case == fmt_lower) {
        return digit - 10 + 'a';
    }

    return digit - 10 + 'A';
}
