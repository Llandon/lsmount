#ifndef HELPER_H
#define HELPER_H

#define _(STRING) gettext(STRING)

#include <stdint.h>
#include <stddef.h>
#include <libintl.h>

int   is_symlink(const char* filename);
char* read_file_to_buf(const char* file, size_t initial_buf_size);

#endif
