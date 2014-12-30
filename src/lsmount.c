#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
// stat S_ISLINK lstat ..
#include <sys/stat.h>
// getpwuid
#include <pwd.h>
#include <libintl.h>
#include "lsmount.h"
#include "lsmgrid.h"
#include "options.h"

int main(int argc, char** argv) {
	setlocale(LC_ALL, "");
	bindtextdomain("lsmount", "/usr/share/locale");
	textdomain("lsmount");

	// init parameter variables
	debug            = 0;
	use_color        = 0;
	show_rootfs      = 0;
	shrink_eighty    = 0;
	show_tmpfs       = 0;
	show_unused      = 0;
	resolve_symlinks = 0;
	use_alignment    = 0;
	vertical         = 0;

	mnt_file               = "/proc/mounts";
	size_t file_buf_size   = 1024;
	const char* conf_file1 = "/etc/lsmountrc";

	char* conf_file2;
	const struct passwd* pw = getpwuid(getuid());
	const char*  homedir    = pw->pw_dir;

	// get absolute conf_file2 path in homedir
	if(-1 == asprintf(&conf_file2, "%s%s", homedir, "/.config/lsmount.rc")) {
		fprintf(stderr,_("failure by generating conf_file2 path\n"));
		free(conf_file2);
		exit(1);
	}

	// get options
	if(!access(conf_file1, R_OK)) { readconffile(conf_file1); }
	if(!access(conf_file2, R_OK)) { readconffile(conf_file2); }
	free(conf_file2);
	if(0 != parsecmd(argc, argv)) { exit(1); }
	checkconf(); // check if configuration is valid

	if(NULL == mnt_file) {
		mnt_file = MNT_FILE;
	}

	// open file stream
	FILE *stream = fopen(mnt_file, "r");
	if(NULL == stream) {
		fprintf(stderr, _("reading from %s failed :: %s\n"), 
			mnt_file, strerror(errno));
		exit(1);
	}

	// allocate and init file_buf
	char* file_buf = malloc(sizeof(char)*(long unsigned int)file_buf_size);
	if(NULL == file_buf) {
		fprintf(stderr, _("malloc failed"));
		exit(1);
	}
	memset(file_buf, '\0', sizeof(char)*(long unsigned int)file_buf_size);
	
	// copy file into file_buf and count lines / rows ========================= 
	size_t mcl[6]      = {0,0,0,0,0,0}; // maximum column length
	size_t msl[6]      = {0,0,0,0,0,0}; // maximum seperable length
	size_t sep         = 0;             // seperable counter
	size_t col_or      = 0;             // column char counter (alignment)
	size_t columns_oar = 0;             // columns over all rows
	size_t columns     = 1;             // columns (space seperated)
	size_t rows        = 0;             // rows

	int c = '\0';
	while( (c=fgetc(stream)) != EOF ) {
		if(' '  == c && 1 == debug) { printf("%4zu ", col_or); } // new column
		if('\n' == c && 1 == debug) { printf("%4zu\n",col_or); } // new row

		// calculate msl
		if(',' == c || '\n' == c || ' ' == c) { sep=0; }
		++sep;
		if(sep > msl[columns_oar%6]) {
			msl[columns_oar%6] = sep;
		}
		
		// calculate mcl
		if(' ' == c || '\n' == c) { // new column or row
			if(mcl[columns_oar%6] < col_or) { // mcl < colc
				mcl[columns_oar%6] = col_or-1;
			}
			col_or=0;
			columns_oar++;
		}
		++col_or;

		if(' ' == c && rows == 0) { ++columns; } // count columns (first row)
		if('\n' == c) { ++rows; } // new row

		// expand buffer if necessary
		if( (size_t)file_buf_size == (size_t)strlen(file_buf)+1 ) {
			char* np = realloc(file_buf, strlen(file_buf)+(size_t)file_buf_size);
			if( NULL == file_buf ) {
				free(file_buf);
				fprintf(stderr, _("realloc failed\n"));
				exit(1);
			}else{
				file_buf = np;
				file_buf_size = strlen(file_buf)+(size_t)file_buf_size;
			}
		}

		// add next char to file_buf
		char buf[2];
		snprintf(buf, 2, "%c", c);
		file_buf = strncat(file_buf, buf, 1);
	}

	// shrink buffer 
	file_buf = realloc(file_buf,sizeof(char)*strlen(file_buf)+1);

	if(debug == 1) {
		printf("=============================\n");
		printf("rows: %zu columns: %zu culumns_oar: %zu\n",
			rows,columns,columns_oar);
	}

	// create grid ============================================================
	t_grid* grid = grid_create(columns, rows, mcl, msl);

	// print maximum column length
	if(debug == 1) {
		printf("max_col_len: %4zu %4zu %4zu %4zu\n",
			grid->max_col_len[0],
			grid->max_col_len[1],
			grid->max_col_len[2],
			grid->max_col_len[3]
		);
	}

	if(debug == 1) {
		printf("max_sep_len: %4zu %4zu %4zu %4zu\n",
			grid->max_sep_len[0],
			grid->max_sep_len[1],
			grid->max_sep_len[2],
			grid->max_sep_len[3]
		);
	}

	// tokenize file_buf (file->lines->element)
	char*  line_r = NULL;
	char*  line   = strtok_r(file_buf,"\n", &line_r);
	size_t elem_c = 0; // element counter
	size_t line_c = 0; // line counter

	while( line != NULL ) { // run over lines
		char* saveptr=NULL; // primary
		char* elem_l = strtok_r(line," ",&saveptr); // primary elem lokal

		while( elem_l != NULL ) { // primary
			grid->elem[elem_c].value = elem_l;
			++elem_c;
			elem_l = strtok_r(NULL," ",&saveptr); // primary
		}
		line = strtok_r(NULL,"\n",&line_r); // primary
		++line_c;
	}

	grid_print(grid);
	grid_destroy(grid);
	free(file_buf); // is used by strtok elements, do not free earlier

	// close file stream
	if(0 != fclose(stream)) {
		fprintf(stderr, _("fclose error :: %s\n"), strerror(errno));
		exit(1);
	}
	exit(0);
}
