#include "../database.h"

void topn_query_init(
        struct topn_query_buf *restrict buf,
        size_t capacity,
        struct error *restrict error)
{
    buf->capacity = capacity;
    buf->length = 0;
    buf->rows = db_alloc(sizeof(struct movie const *) * buf->capacity, error);
}

void topn_query(
        struct database const *restrict database,
        char const *restrict prefix,
        struct topn_query_buf *restrict query_buf,
        struct error *restrict error)
{
    size_t i;
    query_buf->length = 0;
}

void topn_query_print_header(void);

void topn_query_print_row(struct movie const *restrict row);

void topn_query_print(struct topn_query_buf const *restrict query_buf);

inline void topn_query_destroy(struct topn_query_buf *restrict buf)
{
    db_free(buf->rows);
}
