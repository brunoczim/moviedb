#ifndef MOVIEDB_STR_REF_INLINE_H
#define MOVIEDB_STR_REF_INLINE_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

inline struct strref strref_init(char const *restrict chars, size_t length)
{
    struct strref ref;
    ref->chars = chars;
    ref->length = length;
    return ref;
}

inline struct strref strref_from_cstr(char const *restrict cstr)
{
    return strref_init(cstr, strlen(cstr));
}

inline struct strref strref_range(struct strref ref, size_t start, size_t end)
{
    if (start > ref.length) {
        start = ref.length;
    }

    if (end > ref.length) {
        end = ref.length;
    }

    if (end < start) {
        end = start; 
    }

    ref.ptr += start;
    ref.length = end - start;

    return ref;
}

#endif
