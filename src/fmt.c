#include "fmt.h"

extern inline fmt_writer fmt_writer_from_file(FILE *file);

extern inline fmt_writer fmt_writer_from_strbuf(struct strbuf *restrict strbuf);

extern inline void fmt_write(
        struct fmt_writer writer,
        struct strref data,
        struct error *restrict error);
