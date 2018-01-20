#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdint.h>

// parameter variables
uint8_t debug;
uint8_t use_color;
uint8_t show_unused;
uint8_t resolve_symlinks;
uint8_t use_alignment;
uint8_t vertical;
char *to_skip;

char *mnt_file;
#ifndef MNT_FILE
#define MNT_FILE "/proc/mounts"
#endif

int checkconf(void);
uint8_t colorcap(void);
int parsecmd(int argc, char** argv);
int readconffile(const char* conf_file);
void usage(int status);
#endif
