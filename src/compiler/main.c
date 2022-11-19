#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

#include "compiler.h"

#ifndef BCAUSE_VERSION
    #define BCAUSE_VERSION "0.1"
#endif

static inline void version(char *arg0)
{
    printf("%s " BCAUSE_VERSION "\n"
        "Copyright (C) 2022 Spydr06\n"
        "This is free software; see the source for copying conditions.\n"
        "There is NO warranty.\n",
        arg0
    );
}

static inline void help(char *arg0) 
{
    printf("Usage: %s [options] file...\n"
        "Options:\n"
        "--help      Display this information.\n"
        "--version   Display compiler version information.\n"
        "-o <file>   Place the output into <file>.\n"
		"-S          Compile only; do not assemble or link.\n"
        "-c          Compile and assemble, but do not link.\n"
        "-save-temps Do not delete intermediate files.\n",
        arg0
    );
}

/* default compiler settings */
static inline void set_default_args(struct compiler_args *args, const char *arg0, char **input_files)
{
    memset(args, 0, sizeof(struct compiler_args));

    args->arg0 = arg0;
    args->output_file = A_OUT;
    args->input_files = input_files;
    args->do_assembling = args->do_linking = true;
    args->word_size = X86_64_WORD_SIZE;
}

int main(int argc, char **argv)
{
    char *input_files[argc - 1]; /* we can only have a maximum of argc input files */

    struct compiler_args c_args;
    set_default_args(&c_args, argv[0], input_files);

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "--help") == 0) {
            help(argv[0]);
            return 0;
        }
        else if(strcmp(argv[i], "--version") == 0) {
            version(argv[0]);
            return 0;
        }
        else if(strcmp(argv[i], "-o") == 0) {
            if(argc - i <= 1) {
                eprintf(argv[0], "missing filename after " QUOTE_FMT("%s") "\n", argv[i]);
            }
            c_args.output_file = argv[++i];
        }
        else if(strcmp(argv[i], "-S") == 0) {
            c_args.do_assembling = false;
            c_args.do_linking = false;
        }
        else if(strcmp(argv[i], "-c") == 0)
			c_args.do_linking = false;
        else if(strcmp(argv[i], "-save-temps") == 0)
            c_args.save_temps = true;
        else if(argv[i][0] == '-') {
            eprintf(argv[0], "unrecognized command-line option " QUOTE_FMT("%s") "\n", argv[i]);
            return 1;
        }
        else
            c_args.input_files[c_args.num_input_files++] = argv[i];
    }

    if(!c_args.num_input_files) {
        eprintf(argv[0], "no input files\ncompilation terminated.\n");
        return 1;
    }

    return compile(&c_args);
}
