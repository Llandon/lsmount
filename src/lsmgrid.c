#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "options.h"
#include "ansicodes.h"
#include "lsmount.h"
#include "lsmgrid.h"
#include "lsmcolors.h"
#include "helper.h"

t_grid* grid_create(size_t columns, size_t rows) {

	tofree = NULL;
	tofree_c = 0;

	if(columns > SIZE_MAX / rows) { // not really likely, but...
		fprintf(stderr,_("columns*rows > SIZE_MAX => overflow\n"));
		exit(1);
	}

	t_grid* grid = (t_grid*) malloc(sizeof(t_grid));
	if(NULL == grid) {
		 return NULL;
	}

	grid->elem = (t_elem*) malloc(sizeof(t_elem) * columns * rows);
	if(NULL == grid->elem) {
		free(grid);
		return NULL;
	}

	grid->columns = columns;
	grid->rows    = rows;
	
	for(size_t i = 0; i < columns * rows; ++i) {
		grid->elem[i].value       = "N/A";
	}

	return grid;
}

void grid_destroy(t_grid* grid) {
	for(size_t i=0; i<tofree_c; ++i) {
		free(tofree[i]);
	}

	free(tofree);
	free(grid->elem);
	free(grid);
}

void grid_print(t_grid* grid) {
	size_t rows = grid->rows;
	size_t cols = grid->columns;

	if(use_alignment == 1) { grid_analyse(grid); }
	if(debug == 1) {
		printf("cl: "); 
		for(uint8_t i=0; i<6; ++i) {
			printf("[%lu]", column_length[i]);
		}
		putchar('\n');
	}

	for(size_t r=0; r<rows; ++r) {
		for(size_t c=0; c<cols; ++c) {
			if(show_unused == 0 && c>3) { printf("\n"); break; }
			if(use_color == 1) { printf("%s", colors[c]); }

			printf("%s ", grid->elem[r*cols+c].value);

			if(use_alignment == 1) {
				size_t delta = 
					column_length[c] - strlen(grid->elem[r*cols+c].value);
				if(delta>0) {
					for(size_t d=0; d<delta; ++d) {
						putchar(' ');
					}
				}
			}
			if(
				(
					vertical == 1 && 
					(
						(c != cols-1 && show_unused == 1) || 
						(c != cols-3 && show_unused == 0)
					)
				) || (c == cols-1 && show_unused == 1)
			) { putchar('\n'); };
		}
	}
	if(use_color == 1) { printf(ANSI_ESC_RESET); }
}

void grid_analyse(t_grid* grid) {
	size_t rows = grid->rows;
	size_t cols = grid->columns;

	for(size_t r=0; r<rows; ++r) {
		for(size_t c=0; c<cols; ++c) {
			if(strlen(grid->elem[r*cols+c].value)>column_length[c]) {
				column_length[c] = strlen(grid->elem[r*cols+c].value);
			}
		}
	}
}

int grid_load_from_buf(t_grid* grid, char* buf) {
	size_t rows     = 0;
	size_t elem_cnt = 0;

	char *line_r = NULL;
	char *line   = strtok_r(buf, "\n", &line_r);

	while( line != NULL ) {
		char *save_pointer = NULL; // primary
		char *elem_of_line = strtok_r(line, " ", &save_pointer);
		if(NULL != to_skip && strstr(to_skip, elem_of_line)) { // FIXME undefined behaviour
			line = strtok_r(NULL, "\n", &line_r);
		}else{
			while( elem_of_line != NULL ) {
				if(debug == 1) {
					printf("%s\n",elem_of_line);
				}

				if(resolve_symlinks == 1 && is_symlink(elem_of_line)) {
					char resolvBuf[PATH_MAX] = "";
					if(NULL != realpath(elem_of_line, resolvBuf)) {
						char *dest = malloc(sizeof(char)*strlen(resolvBuf)+1);
						if(NULL == tofree) {
							tofree = malloc(sizeof(char*));
							if(NULL == tofree) {
								fprintf(stderr, _("malloc failed\n"));
								exit(1);
							}
						}else{
							char** np = realloc(tofree, (tofree_c+1)*sizeof(char*));
							if(NULL == np) {
								fprintf(stderr, _("realloc failed\n"));
								exit(1);
							}else{
								tofree = np; 
							}
						}
						tofree[tofree_c++] = dest;
						strncpy(dest, resolvBuf, strlen(resolvBuf)+1);
						grid->elem[elem_cnt].value = dest;
					}
				}else{
					grid->elem[elem_cnt].value = elem_of_line;
				}
				elem_of_line = strtok_r(NULL, " ", &save_pointer);
				++elem_cnt;
			}

			line = strtok_r(NULL, "\n", &line_r);
			++rows;
		}
	}
	if(rows > 0) {
		grid->rows = rows;
		grid->columns = elem_cnt/rows;
	}else{
		fprintf(stderr, _("Can't load at least one row from buffer\n"));
		return 0;
	}
	return 1;
}
