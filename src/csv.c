#include "csv.h"

enum csv_sep csv_read_col(FILE *csv_file, struct strbuf *buf)
{
    if (feof(csv_file)) {
        return csv_end_file;
    }
}
