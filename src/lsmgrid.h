#ifndef LSMGRID_H
#define LSMGRID_H

#include <stdint.h>

char** tofree;
size_t tofree_c;

size_t column_length[7];

typedef struct t_grid t_grid;
typedef struct t_elem t_elem;

struct t_grid {
    size_t columns; // should be fix
    size_t rows;
    t_elem* elem;
};

struct t_elem {
    char *value;
};

t_grid *grid_create(size_t columns, size_t rows);

void grid_destroy(t_grid* grid);
void grid_print(t_grid* grid);
void grid_analyse(t_grid* grid);
int  grid_load_from_buf(t_grid* grid, char* buf);

#endif
