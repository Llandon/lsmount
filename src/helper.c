#include "helper.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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

char* read_file_to_buf(const char* file) {
	size_t filebuf_size = 1024; // initial size

	// open read only file stream
	FILE* stream = fopen(file, "r");
	if(NULL == stream) {
		fprintf(stderr, _("fopen on %s failed (%s)\n"), file, strerror(errno));
		exit(1);
	}

	// create buffer
	char* filebuf = malloc(sizeof(char)*(long unsigned int)filebuf_size);
	if(NULL == filebuf) {
		fprintf(stderr, _("malloc failed\n"));
		exit(1);
	}

	// init filebuf
	memset(filebuf, '\0', sizeof(char)*(long unsigned int)filebuf_size);

	int c = '\0'; // init char for fgetc
	while( (c=fgetc(stream)) != EOF ) {
		// expand filebuf by filebuf_size if necessary 
		// n^2 growth
		if( (size_t)filebuf_size == (size_t)strlen(filebuf)+1) {
			char* new_filebuf = realloc(
				filebuf,
				strlen(filebuf)+(size_t)filebuf_size // double size
			);
			if( NULL == new_filebuf ) { // check if realloc was successful
				free(filebuf);
				fprintf(stderr, _("realloc failed\n"));
				exit(1);
			}else{
				filebuf = new_filebuf;
				filebuf_size = strlen(filebuf)+(size_t)filebuf_size;
			}
		}

		char buf[2]; // buffer for snprintf
		snprintf(buf, 2, "%c", c); // read c to buf as char with 0-Byte
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wstringop-truncation"
		// buf is definitely null terminated accordingly we can ignore the warning here
		filebuf = strncat(filebuf, buf, 1); // write char to filebuf
		#pragma GCC diagnostic pop
	}

	// shrink buffer to really needed size
	char* shrk_filebuf = realloc(filebuf, sizeof(char)*strlen(filebuf)+1);
	if( NULL == shrk_filebuf ) { // check if realloc was successful
		free(filebuf);
		fprintf(stderr, _("realloc failed\n"));
		exit(1);
	}else{
		filebuf = shrk_filebuf;
		filebuf_size = strlen(filebuf)+(size_t)filebuf_size;
	}

	if(0 != fclose(stream)) {
		fprintf(stderr, _("fclose error (%s)\n"), strerror(errno));
		exit(1);
	}

	return filebuf;
}

size_t get_buf_lines(const char* buf) {
	size_t i=0;
	size_t lines=0;
	while(i<=strlen(buf)) {
		if(buf[i] == '\n') {
			++lines;
		}
		++i;
	}
	return lines;
}

