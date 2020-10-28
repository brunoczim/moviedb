#ifndef MOVIEDB_STR_TYPES_H
#define MOVIEDB_STR_TYPES_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

struct strref {
    char const *restrict chars;
    size_t length;
};

struct strbox {
    char *restrict chars;
    size_t length;
};

struct strbuf {
    struct strbox box;
    size_t capacity;
};

enum string_kind {
    string_ref,
    string_box
};

union string_data {
    struct strref ref;
    struct strbox box;
};

struct string {
    enum string_kind kind;
    union string_data data;
};

#endif