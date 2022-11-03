SHELL = /bin/sh

CFLAGS = -std=c99 -Wall -Wextra -pedantic -fPIC -O2
CFLAGS_EXT = -nostdlib -Wno-incompatible-library-redeclaration -Wno-builtin-declaration-mismatch -g -c
COMPILER_FILES = $(shell find src/compiler -name '*.c')
LIBB_FILES = $(shell find src/libb -name '*.c')

.PHONY: all
all: bcause.out libb.a

.PHONY: bcause
bcause: bcause.out
bcause.out:
	${CC} ${CFLAGS} ${COMPILER_FILES} -o $@

.PHONY: libb
libb: libb.a
libb.a: libb.o
	ar ruv $@ $<

libb.o:
	${CC} ${CFLAGS} ${CFLAGS_EXT} ${LIBB_FILES} -o $@

.PHONY: clean
clean:
	rm -rf *.o *.a *.out
