#ifndef MOVIEDB_STR_REF_H
#define MOVIEDB_STR_REF_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

#define strref_lit(literal) (strref_init((literal), sizeof(literal) - 1))

inline struct strref strref_init(char const *restrict chars, size_t length);

inline struct strref strref_from_cstr(char const *restrict cstr);

inline struct strref strref_range(struct strref ref, size_t start, size_t end);

inline struct strref strref_range(struct strref ref, size_t start, size_t end);

int strref_cmp(struct strref ref_a, struct strref ref_b);

int strref_icmp(struct strref ref_a, struct strref ref_b);

#endif
