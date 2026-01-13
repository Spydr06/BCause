CFLAGS = -std=gnu99 -Wall -Wextra -pedantic -g -O

CFLAGS_LIBB = -nostdlib -c 					\
	-Wno-incompatible-library-redeclaration \
	-Wno-builtin-declaration-mismatch       \
	-ffreestanding -fno-stack-protector

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
	ar rv $@ $<
	ranlib $@

libb.o:
	${CC} ${CFLAGS} ${CFLAGS_LIBB} ${LIBB_FILES} -o $@

.PHONY: clean
clean:
	rm -rf *.o *.a *.out ${BCAUSE_EXEC} build

.PHONY: test
test:   build
	make -C build btest
	make -C build test

build:
	cmake -B build tests
