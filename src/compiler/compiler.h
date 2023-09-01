#ifndef BCAUSE_COMPILER_H
#define BCAUSE_COMPILER_H

#include <stdbool.h>
#include "list.h"

#define A_OUT "a.out"
#define A_S   "a.S"
#define A_O   "a.o"

#define COLOR_RESET      "\033[0m"
#define COLOR_BOLD_RED   "\033[1m\033[31m"
#define COLOR_BOLD_WHITE "\033[1m\033[37m"

#define QUOTE_FMT(str) COLOR_BOLD_WHITE "‘" str "’" COLOR_RESET

#define X86_64_WORD_SIZE sizeof(intptr_t)

struct compiler_args {
    const char *arg0; /* name of the executable */
    char *output_file; /* output file */
    char **input_files; /* input files */
    int num_input_files; /* number of input files */

    unsigned char word_size; /* size of the B data type */

    bool do_linking;    /* should the compiler link? */
    bool do_assembling; /* should the compiler assemble? */
    bool save_temps;    /* should temporary files get deleted? */

    struct list locals; /* local variables */
    unsigned long stack_offset; /* local variable offset */
    struct list extrns; /* extrn variables */

    struct list strings; /* string table */
};

#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
void eprintf(const char *arg0, const char *fmt, ...);

int compile(struct compiler_args *args);

#endif
