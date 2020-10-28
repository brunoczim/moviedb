#ifndef MOVIEDB_STR_BUF_INLINE_H
#define MOVIEDB_STR_BUF_INLINE_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

inline struct strbuf strbuf_init()
{
    struct strbuf buf;
    buf.box.length = 0;
    buf.box.capacity = 0;
    buf.box.chars = NULL;
    return buf;
}

inline struct strbuf strbuf_from_capacity(
        size_t capacity,
        struct error *restrict error)
{
    struct strbuf buf;
    buf.box.length = 0;
    buf.box.chars = moviedb_alloc(capacity, error);
    if (error->code == error_none) {
        buf.capacity = capacity;
    } else {
        buf.capacity = 0;
    }
    return buf;
}

inline struct strbuf strbuf_from_box(struct strbox box)
{
    return strbuf_from_box_and_cap(box, box.length);
}

inline struct strbuf strbuf_from_box_and_cap(
        struct strbox box,
        size_t capacity)
{
    struct strbuf buf;
    buf.box = box;
    buf.capacity = capacity;
    return buf;
}

inline struct strbuf strbuf_from_ref(
        struct strref ref,
        struct error *restrict error)
{
    struct strbuf buf;
    buf.box = strbox_from_ref(ref, error);
    buf.capacity = buf.box.length;
    return buf;
}

inline void strbuf_reserve(
        struct strbuf *restrict buf,
        size_t additional,
        struct error *restrict error)
{
    size_t new_cap = buf->capacity + additional;
    char const *restrict new_chars = moviedb_realloc(
            buf->chars,
            new_cap,
            error);

    if (error->code == error_none) {
        buf->chars = new_chars;
        buf->capacity = new_cap;
    }
}

inline void strbuf_shrink(
        struct strbuf *restrict buf,
        size_t amount,
        struct error *restrict error)
{
    size_t new_cap = buf->capacity + additional;
    char const *new_chars = moviedb_realloc(
            buf->chars,
            new_cap,
            error);

    if (error->code == error_none) {
        buf->chars = new_chars;
        buf->capacity = new_cap;
        
        if (buf->capacity < buf->length) {
            buf->length = buf->capacity;
        }
    }
}


inline void strbuf_set_capacity(
        struct strbuf *restrict buf,
        size_t new_capacity,
        struct error *restrict error)
{
    if (new_capacity > buf->capacity) {
        strbuf_reserve(buf, new_capacity - buf->capacity, error);
    } else if (new_capacity < buf->capacity) {
        strbuf_shrink(buf, buf->capacity - new_capacity, error);
    }
}

inline void strbuf_push(
        struct strbuf *restrict buf,
        char ch,
        struct error *restrict error)
{
    if (buf->length == buf->capacity) {
        strbuf_reserve(buf, 1, error);
    }

    if (error->code == error_none) {
        buf->chars[buf->length] = ch;
        buf->length += 1;
    }
}

inline void strbuf_append(struct strbuf *restrict buf,
        struct strref suffix,
        struct error *restrict error)
{
    size_t new_length = buf->length + suffix.length;

    if (new_length > buf->capacity) {
        strbuf_reserve(buf, new_length - buf->capacity, error);
    }

    if (error->code == error_none) {
        memcpy(buf->chars + buf->length, suffix.chars, suffix.length);
        buf->length = new_length;
    }
}

inline struct strref strbuf_as_ref(struct strbuf buf)
{
    if (buf.chars == NULL) {
        return strref_init("", 0);
    }
    return strref_init(buf.chars, buf.length);
}

inline struct strbox strbuf_reuse_as_box(
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    struct strbox box;
    if (buf.box.length == buf.capacity) {
        box = buf.box;
        buf.box.chars = NULL;
        buf.box.length = 0;
    } else {
        box = strbox_from_ref(strbuf_as_ref(buf), error);
    }
    return box;
}

inline struct strbox strbuf_move_to_box(struct strbuf buf)
{
    return buf.box;
}

inline void strbuf_destroy(struct strbuf buf)
{
    moviedb_free(buf.chars);
}

#endif
