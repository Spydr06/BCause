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

static long character(struct compiler_args *args, FILE *in) {
    char c;
    int i;
    long value = 0;

    for(i = 0; i < args->word_size; i++) {
        if(c != '*' && (c = fgetc(in)) == '\'')
            return value;
        
        if(c == '*') {
            switch(c = fgetc(in)) {
                case '0':
                case 'e':
                    c = '\0';
                    break;
                case '(':
                case ')':
                case '*':
                case '\'':
                case '"':
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'n':
                    c = '\n';
                    break;
                default:
                    eprintf(args->arg0, "undefined escape character ‘*%c’", c);
                    exit(1);
            }
        }
        value |= c << (i * 8);
    }

    if(fgetc(in) != '\'') {
        eprintf(args->arg0, "unclosed char literal\n");
        exit(1);
    }

    return value;
}

static void ival(struct compiler_args *args, FILE *in, FILE *out)
{
    static char buffer[BUFSIZ];
    long value;
    char c = fgetc(in);

    if(isalpha(c)) {
        ungetc(c, in);
        if(identifier(in, buffer, BUFSIZ) == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .long %s\n", buffer);
    }
    else if(c == '\'') {
        if((value = character(args, in)) == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .long %lu\n", value);
    }
    else {
        ungetc(c, in);
        if((value = number(in)) == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .long %lu\n", value);
    }
}

static void function(struct compiler_args *args, FILE *in, FILE *out, char *identifier)
{
    fprintf(out, ".text\n.type %s, @function\n%s:", identifier, identifier);
    
}

static void global(struct compiler_args *args, FILE *in, FILE *out, char *identifier)
{
    fprintf(out,
        ".data\n"
        ".type %s, @object\n"
        ".align %d\n"
        "%s:\n",
        identifier, args->word_size, identifier
    );
    
    char c;
    if((c = fgetc(in)) != ';') {
        ungetc(c, in);
        do {
            whitespace(in);
            ival(args, in, out);
            whitespace(in);
        } while((c = fgetc(in)) == ',');

        if(c != ';') {
            eprintf(args->arg0, "expect ‘;’ at end of declaration\n");
            exit(1);
        }
    }
    else
        fprintf(out, "  .zero %d\n", args->word_size);
}

static void vector(struct compiler_args *args, FILE *in, FILE *out, char *identifier)
{
    long num = 0;
    char c;

    whitespace(in);
    if((c = fgetc(in)) != ']') {
        ungetc(c, in);
        num = number(in);
        if(num == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect vector size after ‘[’\n");
            exit(1);
        }
        whitespace(in);

        if(fgetc(in) != ']') {
            eprintf(args->arg0, "expect ‘]’ after vector size\n");
            exit(1);
        }
    }

    fprintf(out, 
        ".data\n.type %s, @object\n"
        ".align %d\n"
        "%s:\n",
        identifier, args->word_size, identifier
    );

    whitespace(in);

    if((c = fgetc(in)) != ';') {
        ungetc(c, in);
        do {
            whitespace(in);
            ival(args, in, out);
            whitespace(in);
        } while((c = fgetc(in)) == ',');

        if(c != ';') {
            eprintf(args->arg0, "expect ‘;’ at end of declaration\n");
            exit(1);
        }
    }
    else if((args->word_size * num) != 0)
        fprintf(out, "  .zero %ld\n", args->word_size * num);
}

static void declarations(struct compiler_args *args, FILE *in, FILE *out)
{
    static char buffer[BUFSIZ];
    char c;
    
    while(identifier(in, buffer, BUFSIZ)) {
        fprintf(out, ".globl %s\n", buffer);

        whitespace(in);
        switch(c = fgetc(in)) {
        case '(':
            function(args, in, out, buffer);
            break;
        
        case '[':
            vector(args, in, out, buffer);
            break;
        
        case EOF:
            eprintf(args->arg0, "unexpected end of file after declaration\n");
            exit(1);
        
        default:
            ungetc(c, in);
            global(args, in, out, buffer);
        }
    }

    if(fgetc(in) != EOF) {
        eprintf(args->arg0, "expect identifier at top level\n");
        exit(1);
    }
}
