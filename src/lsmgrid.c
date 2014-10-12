#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include "options.h"
#include "ansicodes.h"
#include "lsmount.h"
#include "lsmgrid.h"

t_grid* grid_create(size_t columns, size_t rows, size_t* max_col_len, 
                    size_t* max_sep_len) {

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
	grid->max_col_len = max_col_len;
	grid->max_sep_len = max_sep_len;
	
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

int is_symlink(const char* filename) {
	struct stat p_statbuf;

	if(lstat(filename, &p_statbuf)) {
		// no file => no symlink
		return 0;
	}else{
		if(S_ISLNK(p_statbuf.st_mode) == 1) {
			return 1;
		}else{
			return 0;
		}
	}
}

void grid_print(t_grid* grid) {
	const size_t columns = grid->columns;
	const size_t rows    = grid->rows;

	if(resolve_symlinks == 1) {
		size_t resolv_c = 1;

		for(size_t k=0; k<rows; ++k) {
			if(is_symlink(grid->elem[k*columns].value)) {
				char buf[PATH_MAX] = "";

				if(NULL != realpath(grid->elem[k*columns].value, buf)) {
					char* dest = malloc(sizeof(char)*strlen(buf)+1);

					strncpy(dest, buf, strlen(buf)+1);
					grid->elem[k*columns].value = dest;

					if(NULL == tofree) {
						tofree = malloc(sizeof(char*));

						if(NULL == tofree) {
							fprintf(stderr, _("malloc failed\n"));
							exit(1);
						}
					}else{
						char** np = realloc(tofree, resolv_c*sizeof(char*));

						if(NULL == np) {
							fprintf(stderr, _("realloc failed\n"));
							exit(1);
						}else{
							tofree = np;
						}
					}

					tofree[resolv_c-1] = dest;
					++tofree_c;
					++resolv_c;
				}
			}
		}

		// recalculate maximum column length
		size_t mcl_l=0;
		for(size_t k=0; k<rows; ++k) {
			if(NULL != grid->elem[k*columns].value) {
				if(mcl_l < strlen(grid->elem[k*columns].value)) {
					mcl_l = strlen(grid->elem[k*columns].value);
				}
			}
		}
		grid->max_col_len[0] = mcl_l;
	}

	// column colors array
	const char* colors[6] = {
		COLOR1,
		COLOR2, 
		COLOR3, 
		COLOR4, 
		COLOR5,
		COLOR6
	};

	for(size_t i=0; i<rows; ++i) {
		if(show_tmpfs==0 && !strcmp("tmpfs",grid->elem[i*columns+2].value)) {
			continue; 
		}
		if(show_rootfs==0 && !strcmp("rootfs",grid->elem[i*columns+2].value)) {
			continue; 
		}

		for(size_t j=0; j<columns; ++j) {
			if(show_unused == 0 && j >= 4) { break; }
			if(shrink_eighty == 1 && vertical == 0 && j>=4) { break; }
			if(use_color == 1) { printf("%s",colors[j]); }
			if(vertical == 1) {
				printf("%s\n", grid->elem[i*columns+j].value);
			}else{
				if(shrink_eighty == 1 && 3 == j) {
					size_t space = 80 - (
						grid->max_col_len[0]+
						grid->max_col_len[1]+
						grid->max_col_len[2]+
						4
					);
					//if(show_unused == 1) { space -= 4; } // TODO show_unused
					grid_print_split(grid->elem[i*columns+j].value,",",space);
				}else{
					printf("%s ", grid->elem[i*columns+j].value);
				}
			}

			size_t used_columns = columns-2;

			if(show_unused == 1) { used_columns = columns; }

			// dont fill up after last column	
			if(use_alignment == 1 && vertical != 1 && j < used_columns-1) {
				if(0 == shrink_eighty || j < 3) {
					size_t diff = grid->max_col_len[j] - 
						strlen(grid->elem[i*columns+j].value);
					for(size_t l=0; l<diff; ++l) {
						putchar(' ');
					}
				}else{
					// TODO use_alignment + show_unused
					// even if it's doesn't make sense
				}
			}
			if(use_color == 1) { printf(ANSI_ESC_RESET); }
		}
		putchar('\n');
	}
}

int grid_print_split(char* line, const char* delim, size_t max) {
	if(
		NULL == line || !strcmp("",line) || 
		NULL == delim || !strcmp("",delim) ||
		max  == 0
	) {
		return -1;
	}

	char* token  = NULL;
	char* lcpy   = strdup(line);
	char* save_lcpy_ptr = lcpy;

	uint8_t tokcnt  = 0; // tokens
	size_t  chrpl   = 0; // chars per line
	size_t  linecnt = 1; // line counter
    
	while (NULL != (token = strsep(&lcpy, delim))) {
		if(chrpl+strlen(token)+1 > UINT16_MAX-1) { break; }
		size_t chrpl_old = chrpl; // save value
		chrpl+=strlen(token)+1;

		if(chrpl < max) { // enough space for next token
			if(0 != tokcnt) { putchar(','); }
			printf("%s",token);
		}else{ // begin new line
			if(0 != tokcnt) { putchar(','); }
			
			// fixme
			if(1==linecnt) {
				for(size_t i=0; i<max-chrpl_old; ++i) { putchar(' '); }
				if(1 == show_unused && 1 == use_color) { printf(COLOR5); }
				if(1 == show_unused) { printf("%s %s"," 0","0"); }
				if(1 == show_unused && 1 == use_color) { printf(COLOR4); }
			}
			++linecnt;

			putchar('\n');
			chrpl=0;
			chrpl+=strlen(token)+1;
			if(chrpl < max) { // token < max => can fill up
				for(uint8_t k=0; k<80-max-1; ++k) {
					putchar(' ');
				}
				printf("%s",token);
			}else{ // token > max
				if(chrpl <= 80) { // token <= 80 chars
					for(uint8_t k=0; k<80-chrpl; ++k) {
						putchar(' ');
					}
					printf("%s",token);
				}else{
					printf("%s",token);
					if(1 == debug ) {
						fprintf(stderr,_("string length of \"%s\" > 80\n"),token);
					}
				}
			}
		}
		if(UINT8_MAX-1 == tokcnt) { 
			fprintf(stderr, _("to many tokens\n"));
			break;
		}
		++tokcnt;
	}
	free(save_lcpy_ptr);
	return 0;
}
