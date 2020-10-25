#ifndef MOVIEDB_ID_H
#define MOVIEDB_ID_H 1

#include "strbuf.h"

typedef long unsigned moviedb_id;

moviedb_id moviedb_id_parse(struct strbuf *restrict buf, struct error *error);

#endif
