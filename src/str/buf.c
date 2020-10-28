#include "../str.h"

extern inline struct strbuf strbuf_init();

extern inline struct strbuf strbuf_from_capacity(
        size_t capacity,
        struct error *restrict error);

extern inline struct strbuf strbuf_from_box(struct strbox box);

extern inline struct strbuf strbuf_from_box_and_cap(
        struct strbox box,
        size_t capacity);

extern inline struct strbuf strbuf_from_ref(
        struct strref ref,
        struct error *restrict error);

extern inline void strbuf_reserve(
        struct strbuf *restrict buf,
        size_t count,
        struct error *restrict error);

extern inline void strbuf_shrink(
        struct strbuf *restrict buf,
        size_t count,
        struct error *restrict error);

extern inline void strbuf_set_capacity(
        struct strbuf *restrict buf,
        size_t new_capacity,
        struct error *restrict error);

extern inline void strbuf_push(
        struct strbuf *restrict buf,
        char ch,
        struct error *restrict error);

extern inline void strbuf_append(struct strbuf *restrict buf,
        struct strref suffix,
        struct error *restrict error);

extern inline struct strref strbuf_as_ref(struct strbuf buf);

extern inline struct strbox strbuf_reuse_as_box(
        struct strbuf *restrict buf,
        struct error *restrict error);

extern inline struct strbox strbuf_move_to_box(struct strbuf buf);

extern inline void strbuf_destroy(struct strbuf buf);
