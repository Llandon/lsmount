#ifndef LSMGRID_H
#define LSMGRID_H

#include <stdint.h>

#define COLOR1 ANSI_ESC_RED
#define COLOR2 ANSI_ESC_GREEN
#define COLOR3 ANSI_ESC_BLUE
#define COLOR4 ANSI_ESC_YELLOW
#define COLOR5 ANSI_ESC_WHITE
#define COLOR6 ANSI_ESC_WHITE

char** tofree;
size_t tofree_c;

typedef struct t_grid t_grid;
typedef struct t_elem t_elem;
typedef struct t_cbuf t_cbuf;

struct t_grid {
    size_t  columns; // should be fix
    size_t  rows;
    size_t* max_col_len; 
    size_t* max_sep_len;
    t_elem* elem;
};

struct t_elem {
    char*  value;
};

t_grid* grid_create(size_t columns, size_t rows, size_t* max_col_len, 
                    size_t* max_sep_len);

void grid_destroy(t_grid* grid);
void grid_print(t_grid* grid);
int  grid_print_split(char* line, const char* delim, size_t max);

int is_symlink(const char* filename);

#endif
