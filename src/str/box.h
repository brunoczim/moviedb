#ifndef MOVIEDB_STR_BOX_H
#define MOVIEDB_STR_BOX_H 1

#ifndef MOVIEDB_STR_H
#   error "Please, do not include this file directly, include str.h instead"
#endif

inline struct strref strbox_as_ref(struct strbox box);

inline struct strbox strbox_from_ref(struct strref ref, struct error *error);

inline void strbox_destroy(struct strbox box);

#endif
