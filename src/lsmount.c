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
#include "lsmcolors.h"
#include "options.h"
#include "helper.h"

int main(int argc, char** argv) {
	setlocale(LC_ALL, "");
	bindtextdomain("lsmount", "/usr/share/locale");
	textdomain("lsmount");

	// init color array
	initcolors();

	// init parameter variables
	debug            = 0;
	use_color        = 0;
	show_unused      = 0;
	resolve_symlinks = 0;
	use_alignment    = 0;
	vertical         = 0;

	mnt_file               = "/proc/mounts";
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

	if(NULL == mnt_file) {
		mnt_file = MNT_FILE;
	}

	if(debug == 1) {
		printf(
			"debug:            %d\n"
			"use_color:        %d\n"
			"show_unused:      %d\n"
			"resolve_symlinks: %d\n"
			"use_alignment:    %d\n"
			"vertical:         %d\n"
			"to_skip:          %s\n"
			"mnt_file:         %s\n",
			debug, use_color, show_unused, resolve_symlinks, use_alignment, 
			vertical, to_skip, mnt_file
		);
	}

	// read file to buffer
	char* filebuf = read_file_to_buf(mnt_file, 1024);

	// create grid
	t_grid* grid = grid_create(9, 40);
	// load data to grid
	if(!grid_load_from_buf(grid, filebuf)) {
		fprintf(stderr, _("load to grid failed (%s)\n"), strerror(errno));
		exit(1);
	}
	grid_print(grid);
	grid_destroy(grid);
	free(filebuf); // is used by strtok elements, do not free earlier
	free(to_skip);
	exit(0);
}

