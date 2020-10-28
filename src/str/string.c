#include "../str.h"

extern inline struct string string_from_ref(struct strref ref);

extern inline struct string string_from_box_move(struct strbox box);

extern inline struct strref string_as_ref(struct string string);

extern inline void string_destroy(struct string string);
