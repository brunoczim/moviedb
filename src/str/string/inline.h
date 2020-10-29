#ifndef MOVIEDB_STR_STRING_INLINE_H
#define MOVIEDB_STR_STRING_INLINE_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

inline struct string string_from_ref(struct strref ref)
{
    struct string string;
    string.kind = string_ref;
    string.data.ref = ref;
    return string;
}

inline struct string string_from_box_move(struct strbox box)
{
    struct string string;
    string.kind = string_box;
    string.data.box = box;
    return string;
}

inline struct strref string_as_ref(struct string string)
{
    switch (string.kind) {
        case string_ref:
            return string.data.ref;
        case string_box:
            return strbox_as_ref(string.data.box);
        default:
            return strref_init("", 0);
    }
}

inline void string_destroy(struct string string)
{
    switch (string.kind) {
        case string_ref:
            break;
        case string_box:
            strbox_destroy(string.data.box);
            break;
    }
}

#endif
