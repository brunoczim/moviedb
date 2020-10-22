#ifndef MOVBASE_CSV_H
#define MOVBASE_CSV_H 1

#include <stdio.h>
#include "strbuf.h"

enum csv_sep {
    csv_end_col,
    csv_end_row,
    csv_end_file
};

enum csv_sep csv_read_col(FILE *csv_file, struct strbuf *buf);

#endif
