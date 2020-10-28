#include <ctype.h>
#include "../str.h"

extern inline struct strref strref_init(
        char const *restrict chars,
        size_t length);

extern inline struct strref strref_from_cstr(char const *restrict cstr);

extern inline struct strref strref_range(
        struct strref ref,
        size_t start,
        size_t end);

int strref_cmp(struct strref ref_a, struct strref ref_b)
{
    int cmp = 0;
    size_t i = 0;

    while (cmp == 0 && (i < ref_a.length || i < ref_b.length)) {
        if (i >= ref_a.length) {
            cmp = -1;
        } else if (i >= ref_b.length) {
            cmp = 1;
        } else if (ref_a->chars[i] < ref_b->chars[i]) {
            cmp = 1;
        } else if (ref_a->chars[i] > ref_b->chars[i]) {
            cmp = -1;
        } else {
            i++;
        }
    }

    return cmp;
}

int strref_icmp(struct strref ref_a, struct strref ref_b)
{
    int cmp = 0;
    char ch_a, ch_b;
    size_t i = 0;

    while (cmp == 0 && (i < ref_a.length || i < ref_b.length)) {
        if (i >= ref_a.length) {
            cmp = -1;
        } else if (i >= ref_b.length) {
            cmp = 1;
        } else {
            ch_a = tolower(ref_a->chars[i]);
            ch_b = tolower(ref_b->chars[i]);
            if (ch_a < ch_b) {
                cmp = 1;
            } else if (ch_a > ch_b) {
                cmp = -1;
            } else {
                i++;
            }
        }   
    }

    return cmp;
}
