#ifndef MOVIEDB_STR_BOX_INLINE_H
#define MOVIEDB_STR_BOX_INLINE_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

inline struct strref strbox_as_ref(struct strbox box)
{
    if (box.chars == NULL) {
        return strref_init("", 0);
    }
    return strref_init(box.chars, box.length);
}

inline struct strbox strbox_from_ref(
        struct strref ref,
        struct error *restrict error)
{
    struct strbox box;
    box.chars = moviedb_alloc(ref.length, error);
    if (error->code == error_none) {
        box.length = ref.length;
        memcpy(box.chars, ref.chars, ref.length);
    } else {
        box.length = 0;
    }
    return box;
}

inline void strbox_destroy(struct strbox box)
{
    moviedb_free(box.chars);
}

#endif
