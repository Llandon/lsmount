#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libconfig.h>
#include <getopt.h>
#include <curses.h> // because terminfo manpages said so...
#include <term.h>
#include <string.h>
#include "lsmount.h"
#include "options.h"
#include "lsmcolors.h"

int parsecmd(int argc, char** argv) {
	int opt;

	static struct option long_options[] = {
		{"use-alignment",           no_argument,       NULL, 'a'},
		{"dont-use-alignment",      no_argument,       NULL, 'A'},
		{"use-color",               optional_argument, NULL, 'c'},
		{"dont-use-color",          no_argument,       NULL, 'C'},
		{"debug",                   no_argument,       NULL, 'd'},
		{"dont-debug",              no_argument,       NULL, 'D'},
		{"shrink-eighty",           no_argument,       NULL, 'e'},
		{"dont-shrink-eighty",      no_argument,       NULL, 'E'},
		{"use-file",                required_argument, NULL, 'f'},
		{"no-file",                 no_argument,       NULL, 'F'},
		{"help",                    no_argument,       NULL, 'h'},
		{"dont-help",               no_argument,       NULL, 'H'},
		{"resolv-symlinks",         no_argument,       NULL, 'l'},
		{"dont-resolv-symlinks",    no_argument,       NULL, 'L'},
		{"skip",                    required_argument, NULL, 's'},
		{"dont-skip",               no_argument,       NULL, 'S'},
		{"show-unused",             no_argument,       NULL, 'u'},
		{"dont-show-unused",        no_argument,       NULL, 'U'},
		{"print-vertical",          no_argument,       NULL, 'v'},
		{"dont-print-vertical",     no_argument,       NULL, 'V'},
		{"set-colors",              required_argument, NULL, 'x'},
		{"dont-set-colors",         no_argument,       NULL, 'X'},
		{NULL, 0, NULL, '\0'}
	};

	while((opt = getopt_long(argc, argv, "aAcCdDeEf:FhHlLs:SuUvVx:X", 
	                         long_options, NULL)) != -1) {
		switch(opt) {
			case 'a':
				use_alignment = 1;
				break;
			case 'A':
				use_alignment = 0;
				break;
			case 'c':
				if(argv && argv[optind] && strcmp(argv[optind],"auto") == 0) {
					use_color = colorcap();
				}else{
					use_color = 1;
				}
				break;
			case 'C':
				use_color = 0;
				break;
			case 'd':
				debug = 1;
				break;
			case 'D':
				debug = 0;
				break;
			case 'e':
				shrink_eighty = 1;
				show_unused = 0;
				break;
			case 'E':
				shrink_eighty = 0;
				break;
			case 'f':
				mnt_file = optarg;
				use_other_file = 1;
				break;
			case 'F':
				use_other_file = 0;
				mnt_file = MNT_FILE;
				break;
			case 'h':
				usage(0);
				break;
			case 'H': // just to be consistent
				exit(1);
				break;
			case 'l':
				resolve_symlinks = 1;
				break;
			case 'L':
				resolve_symlinks = 0;
				break;
			case 's':
				if(optarg) {
					size_t optsize = strlen(optarg)+1;
					to_skip = (char*)malloc(optsize); //FIXME free me
					strncpy(to_skip, optarg,optsize);
				}
				break;
			case 'S':
				to_skip = "";
				break;
			case 'u':
				if(1 == shrink_eighty) {
					show_unused = 0;
				}else{
					show_unused = 1;
				}
				break;
			case 'U': 
				show_unused = 0;
				break;
			case 'v':
				vertical = 1;
				break;
			case 'V':
				vertical = 0;
				break;
			case 'x':
				if(optarg) {
					char* value;
					char* subopts;
					char* const tokens[] = {
						"use-color",
						NULL
					};

					subopts = optarg;
					uint8_t i=0;

					while(*subopts != '\0') {
						getsubopt(&subopts, tokens, &value);
						colors[i] = colortoesc(value);
						++i;
					}

					if(i == 1) {
						printf(_("unknown argument for option set-color(c)\n"));
						exit(1);
					}else if(i != 6) {
						printf(_("wrong number of arguments for option use-color(c)\n"));
						exit(1);
					}
				}
				break;
			case 'X':
				initcolors();
				break;
			default:
				usage(1);
		}
	}
	return 0;
}

int readconffile(const char* config_file) {
    config_t cfg;
    int value;
	const char* strvalue = NULL;

    config_init(&cfg);

    if (!config_read_file(&cfg, config_file)) {
        printf(
            _("Can't read config-file %s:%d - %s\n"), 
            config_error_file(&cfg), 
            config_error_line(&cfg), 
            config_error_text(&cfg)
        );
        config_destroy(&cfg);
        return -1; 
    }   
    if(config_lookup_bool(&cfg, "debug", &value)) {
        debug = (uint8_t)value;
    }
	if(config_lookup_string(&cfg, "skip", &strvalue)) {
		if(strvalue) {
			to_skip = (char*)malloc(strlen(strvalue)+1); //FIXME free me
			strcpy(to_skip,strvalue);
		}
	}
    if(config_lookup_bool(&cfg, "use-color", &value)) {
        use_color = (uint8_t)value;
    }else if(config_lookup_string(&cfg, "use-color", &strvalue)) {
		if(strvalue) {
			if(!strcmp(strvalue,"auto")) {
				use_color = colorcap();
			}else{
				printf(_("unknown argument %s for option use-color(c)\n"), strvalue);
				exit(1);
			}
		}else{
			use_color = 0;
		}
	}
	if(config_lookup_string(&cfg, "set-colors", &strvalue)) {
		if(strvalue) {
			uint16_t i = 0;
			char* strvalue_cpy = strdup(strvalue);
			char* token = NULL;
			
			token = strtok(strvalue_cpy, ",");
			while(NULL != token) {
				colors[i] = colortoesc(token);
				token = strtok(NULL, ",");
				++i;
			}
			free(strvalue_cpy);
		}
	}
    if(config_lookup_bool(&cfg, "shrink-eighty", &value)) {
        shrink_eighty = (uint8_t)value;
		show_unused = 0;
    }   
    if(config_lookup_bool(&cfg, "show-unused", &value)) {
        if(shrink_eighty == 0) {
			show_unused = (uint8_t)value;
		}else{
			show_unused = 0;
		}
    }   
    if(config_lookup_bool(&cfg, "resolve-symlinks", &value)) {
        resolve_symlinks = (uint8_t)value;
    }   
    if(config_lookup_bool(&cfg, "use-alignment", &value)) {
        use_alignment = (uint8_t)value;
    }   
    if(config_lookup_bool(&cfg, "vertical", &value)) {
        vertical = (uint8_t)value;
    }
	config_destroy(&cfg);
	return 0;
}

int checkconf(void) {
	// TODO check if config makes sense
    // for now it isn't be necessary 
	/*
	if(show_unused == 1 && shrink_eighty == 1) {
		printf(_("compining -u and -s isn't possible\n"));
		exit(1);
	}
	if(vertical == 1 && shrink_eighty == 1) {
		printf(_("compining -v and -s isn't possible\n"));
		exit(1);
	}
	*/
	return 0;
}

uint8_t colorcap(void) {
	int* errret = NULL;
	int  ret    = setupterm(NULL, 1, errret); // will leak mem (curses sucks)

	if(0 != ret) {
		if(NULL != errret) {
			if(1 == *errret) {
				fprintf(stderr, _("Terminal is a hardcopy type\n"));
			}else if(0 == *errret) {
				fprintf(stderr, _("Terminal could not be found, or it is a generic type\n"));
			}else if(-1 == *errret) {
				fprintf(stderr, _("terminfo database could not be found\n"));
			}else{
				fprintf(stderr, _("something strange happend while evaluating terminfo\n"));
			}
			del_curterm(cur_term);
			return 0;
		}else{
			fprintf(stderr, _("something strange happend while evaluating terminfo\n"));
			del_curterm(cur_term);
			return 0;
		}
	}else{   
		if(tigetnum("colors") >= 8) {
			del_curterm(cur_term);
			return 1;
		}else{
			del_curterm(cur_term);
			return 0;
		}
	}
}

void usage (int status) {
	puts(_("Usage: lsmount [options]\n"
	       "\n"
	       "Options:\n"
	       "  -a, --use-alignment          align columns\n"
	       "  -c, --use-color              use colors\n"
	       "  -d, --debug                  show debug outputs\n"
	       "  -e, --shrink-eighty          try shrinking to 80 chars\n"
	       "  -f, --use-file               use another input file\n"
	       "  -h, --help                   show this help\n"
	       "  -l, --resolv-symlinks        resolv device symlinks\n"
	       "  -s, --skip                   skip filesystems\n"
	       "  -u, --show-unused            show unused columns\n"
	       "  -v, --print-vertical         vertical output\n"
	       "  -x, --set-colors             set output colors\n"
	       "\n"
	       "all short options can be inverted by using the uppercase letter,\n"
	       "the longopts can be inverted by adding dont- in front.\n"
	       "\n"
	       "Version: v0.2.0\n"
	       "License: ISC\n")
	);
	free(to_skip);
	exit(status);
}
