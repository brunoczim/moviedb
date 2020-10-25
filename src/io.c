#include "io.h"

extern inline FILE *input_file_open(
        char const *restrict path,
        struct error *error);

extern inline void input_file_setbuf(
        FILE *file,
        char *buffer,
        size_t size,
        struct error *error);

extern inline int input_file_read(FILE *file, struct error *error);

extern inline void input_file_close(FILE *file);

void stdin_read_line(struct strbuf *restrict buf, struct error *restrict error)
{
    int ch = input_file_read(stdin, error);

    while (error->code == error_none && ch != EOF && ch != '\n') {
        strbuf_push(buf, ch, error);
        if (error->code == error_none) {
            ch = input_file_read(stdin, error);
        }
    }

    if (ch != EOF && error->code == error_none) {
        strbuf_push(buf, ch, error);
    }
}
