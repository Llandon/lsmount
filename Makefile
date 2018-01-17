.PHONY: all clean install uninstall manpage

CC      = /usr/bin/gcc
CFLAGS += -std=gnu99 -lconfig -D_GNU_SOURCE
CFLAGS += `pkg-config --cflags libconfig`
LDLIBS += `pkg-config --libs libconfig`
LDFLAGS = -ltermcap
BIN     = lsmount
OBJ     = lsmount.o lsmgrid.o options.o lsmcolors.o helper.o
VPATH   = src

all: CFLAGS += -O2 -Wall -Werror -pedantic
all: $(BIN)

debug: CFLAGS += -g -Og -DDEBUG -Wextra -pedantic  -Wcast-qual -Wcast-align -Wformat -Wformat-nonliteral -Wformat-security  -Winit-self -Wmissing-include-dirs -Wno-suggest-attribute=noreturn -Wno-write-strings -Wpointer-arith -Wredundant-decls -Wundef -Wpacked -Wunreachable-code  -Wno-unused-parameter -Wconversion -Wshadow -Wstrict-prototypes -Wbad-function-cast -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wnested-externs
debug: $(BIN)

lsmount:   lsmount.o lsmgrid.o options.o lsmcolors.o helper.o
	$(CC) -o $@ $(OBJ) $(LDLIBS) $(LDFLAGS)
lsmount.o: lsmount.c lsmount.h ansicodes.h
	$(CC) $(CFLAGS) -c -o $@ $<
lsmgrid.o: lsmgrid.c lsmgrid.h
	$(CC) $(CFLAGS) -c -o $@ $<
lsmcolors.o: lsmcolors.c lsmcolors.h
	$(CC) $(CFLAGS) -c -o $@ $<
options.o: options.c options.h
	$(CC) $(CFLAGS) -c -o $@ $<
helper.o: helper.c helper.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJ)
	rm -f doc/lsmount.1.gz

install: lsmount manpage
	cp lsmount /usr/bin/$(BIN)
	cp share/lsmount.rc.example /etc/lsmountrc
	cp doc/lsmount.1.gz /usr/share/man/man1/

uninstall:
	rm -f /usr/bin/$(BIN)
	rm -f /etc/lsmountrc
	rm -f /usr/share/man/man1/lsmount.1.gz

manpage: doc/lsmount.txt
	a2x -f manpage doc/lsmount.txt
	gzip -f doc/lsmount.1
