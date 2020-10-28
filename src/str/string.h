#ifndef MOVIEDB_STR_STRING_H
#define MOVIEDB_STR_STRING_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

#define string_literal(literal) (strref_make_string(strref_literal((literal))))

inline struct string string_from_ref(struct strref ref);

inline struct string string_from_box_move(struct strbox box);

inline struct strref string_as_ref(struct string string);

inline void string_destroy(struct string string);

#endif
