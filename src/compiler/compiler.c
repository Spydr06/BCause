#include "compiler.h"

#define _XOPEN_SOURCE 700
#include <stdio.h>
#undef _XOPEN_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

static void declarations(struct compiler_args *args, FILE *in, FILE *buffer);

#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
void eprintf(char *arg0, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, COLOR_BOLD_WHITE "%s: " COLOR_BOLD_RED "error: " COLOR_RESET, arg0);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

int compile(struct compiler_args *args)
{
    // create a buffer for the assembly code
    char* buf;
    size_t buf_len;
    FILE* buffer = open_memstream(&buf, &buf_len);

    // open every provided `.b` file and generate assembly for it
    for(int i = 0; i < args->num_input_files; i++) {
        size_t len = strlen(args->input_files[i]);
        if(len >= 2 && args->input_files[i][len - 1] == 'b' && args->input_files[i][len - 2] == '.') {
            FILE* in = fopen(args->input_files[i], "r");
            if(!in) {
                eprintf(args->arg0, "%s: %s\ncompilation terminated.\n", args->input_files[i], strerror(errno));
                return 1;
            }
            declarations(args, in, buffer);
            fclose(in);
        }
    }

    // write the buffer to an assembly file
    fclose(buffer);
    FILE* out = fopen(A_S, "w");
    if(!out) {
        eprintf(args->arg0, "cannot open file " COLOR_BOLD_WHITE "‘%s’:" COLOR_RESET " %s.", A_S, strerror(errno));
        return 1;
    }
    fwrite(buf, buf_len, 1, out);
    fclose(out);
    free(buf);

    if(args->do_assembling) {
        // TODO: assemble here
    }
    
    if(args->do_linking) {
        // TODO: link here
    }

    return 0;
}

static void whitespace(FILE *in) {
    char c;
    while((c = fgetc(in)) != EOF) {
        if(!isspace(c)) {
            ungetc(c, in);
            return;
        }
    }
}

static int identifier(FILE *in, char* buffer, size_t buf_len) 
{
    int read = 0;
    char c;

    whitespace(in);
    memset(buffer, 0, buf_len * sizeof(char));

    while((c = fgetc(in)) != EOF) {
        if((read == 0 && !isalpha(c)) || !isalnum(c)) {
            ungetc(c, in);
            return read;
        }
        buffer[read++] = c;
    }

    return read;
}

static int symbol(FILE *in, char *symbol) {
    int read = 0;
    char c;

    whitespace(in);
    while((c = fgetc(in)) != EOF) {
        if(c != symbol[read++]) {
            ungetc(c, in);
            return read;
        }
    }

    return read;
}

static long number(FILE *in) {
    int read = 0;
    char c;
    long num = 0;

    whitespace(in);
    while((c = fgetc(in)) != EOF) {
        if(!isdigit(c)) {
            ungetc(c, in);
            return num;
        }
        read++;
        num *= 10;
        num += (c - '0');
    }

    if(read == 0)
        return EOF;
    return num;
}

static void function(struct compiler_args *args, FILE *in, FILE *out, char* identifier)
{
    fprintf(out, ".text\n.type %s, @function\n%s:", identifier, identifier);
}

static void declarations(struct compiler_args *args, FILE *in, FILE *out)
{
    char buffer[BUFSIZ];
    char c;
    long num;
    
    while(identifier(in, buffer, BUFSIZ)) {
        fprintf(out, ".globl %s\n", buffer);

        whitespace(in);
        switch(c = fgetc(in)) {
        case '(':
            function(args, in, out, buffer);
            break;
        
        case '[':
            num = number(in);
            if(num == EOF) {
                eprintf(args->arg0, "unexpected end of file, expect vector size after ‘[’\n");
                exit(1);
            }
            fprintf(out, 
                ".data\n.type %s, @object\n"
                ".size %s, %ld\n"
                ".align %d\n"
                "%s:\n"
                "  .zero %ld\n", 
                buffer, buffer, args->word_size * num, args->word_size, buffer, args->word_size * num
            );
            if(fgetc(in) != ']') {
                eprintf(args->arg0, "expect ‘]’ after vector size\n");
                exit(1);
            }
            break;
        
        case EOF:
            eprintf(args->arg0, "unexpected end of file after declaration\n");
            exit(1);
        
        default:
            if(isdigit(c)) {
                ungetc(c, in);
                fprintf(out,
                    ".data\n"
                    ".type %s, @object\n"
                    ".size %s, %d\n"
                    ".align %d\n"
                    "%s:\n"
                    "  .long %ld\n",
                    buffer, buffer, args->word_size, args->word_size, buffer, number(in)
                );
                break;
            }
            
            eprintf(args->arg0, "unexpected character " COLOR_BOLD_WHITE "‘%c’" COLOR_RESET " after declaration\n", c);
            exit(1);
        }
    }

    if(fgetc(in) != EOF) {
        eprintf(args->arg0, "expect identifier at top level.");
        exit(1);
    }
}
