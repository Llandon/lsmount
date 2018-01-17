#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lsmcolors.h"
#include "ansicodes.h"
#include "lsmount.h"

void initcolors(void) {
	colors[0] = ANSI_ESC_RED;
	colors[1] = ANSI_ESC_GREEN;
	colors[2] = ANSI_ESC_BLUE;
	colors[3] = ANSI_ESC_YELLOW;
	colors[4] = ANSI_ESC_WHITE;
	colors[5] = ANSI_ESC_WHITE;
}

char* colortoesc(char* cstr) {
	if(!strcmp(cstr, "black")) {
		return ANSI_ESC_BLACK;
	}else if(!strcmp(cstr, "red")) {
		return ANSI_ESC_RED;
	}else if(!strcmp(cstr, "green")) {
		return ANSI_ESC_GREEN;
	}else if(!strcmp(cstr, "yellow")) {
		return ANSI_ESC_YELLOW;
	}else if(!strcmp(cstr, "blue")) {
		return ANSI_ESC_BLUE;
	}else if(!strcmp(cstr, "magenta")) {
		return ANSI_ESC_MAGENTA;
	}else if(!strcmp(cstr, "cyan")) {
		return ANSI_ESC_CYAN;
	}else if(!strcmp(cstr, "white")) {
		return ANSI_ESC_WHITE;
	}else{
		printf(_("unknown color\n"));
		exit(1);
	}
}
