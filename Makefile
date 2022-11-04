SHELL = /bin/sh

CFLAGS = -std=c99 -Wall -Wextra -pedantic -fPIC -O2
CFLAGS_EXT = -nostdlib -Wno-incompatible-library-redeclaration -Wno-builtin-declaration-mismatch -c
COMPILER_FILES = $(shell find src/compiler -name '*.c')
LIBB_FILES = $(shell find src/libb -name '*.c')

LIBB_BIN = libb.a
BCAUSE_EXEC = bcause

BINDIR = ${SYSROOT}/bin
LIBDIR = ${SYSROOT}/lib64

.PHONY: all
all: ${BCAUSE_EXEC} ${LIBB_BIN}

.PHONY: install
install: all
	install ${LIBB_BIN} ${LIBDIR}/${LIBB_BIN}
	install -m 557 ${BCAUSE_EXEC} ${BINDIR}/${BCAUSE_EXEC}

${BCAUSE_EXEC}:
	${CC} ${CFLAGS} ${COMPILER_FILES} -o $@

.PHONY: libb
libb: libb.a
${LIBB_BIN}: libb.o
	ar ruv $@ $<
	ranlib $@

libb.o:
	${CC} ${CFLAGS} ${CFLAGS_EXT} ${LIBB_FILES} -o $@

.PHONY: clean
clean:
	rm -rf *.o *.a *.out ${BCAUSE_EXEC}
