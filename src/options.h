#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdint.h>

// parameter variables
uint8_t debug;
uint8_t use_color;
uint8_t show_rootfs;
uint8_t shrink_eighty;
uint8_t show_tmpfs;
uint8_t show_unused;
uint8_t resolve_symlinks;
uint8_t use_alignment;
uint8_t vertical;
uint8_t use_other_file;

char* mnt_file;
#ifndef MNT_FILE
#define MNT_FILE "/proc/mounts"
#endif

int checkconf(void);
int colorcap(void);
int parsecmd(int argc, char** argv);
int readconffile(const char* conf_file);
void usage(int status);
#endif
