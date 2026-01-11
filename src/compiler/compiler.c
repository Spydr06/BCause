#include "compiler.h"
#include "list.h"

#define _XOPEN_SOURCE 700
#include <stdio.h>
#undef _XOPEN_SOURCE

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define ASSERT_CHAR(args, in, expect, ...) do {     \
    char _c;                                        \
    if ((_c = fgetc(in)) != expect) {               \
        eprintf(args->arg0, __VA_ARGS__);           \
        exit(1);                                    \
    }} while (0)

#define MAX_FN_CALL_ARGS 6

static const char* arg_registers[MAX_FN_CALL_ARGS] = {
    "%rdi",
    "%rsi",
    "%rdx",
    "%rcx",
    "%r8",
    "%r9"
};

enum cmp_operator {
    CMP_LT = 0, /* less-than operator */
    CMP_LE, /* less-than-equal operator */
    CMP_GT, /* greater-than operator */
    CMP_GE, /* greater-than-equal operator */
    CMP_EQ, /* equality operator */
    CMP_NE  /* non-equality operator */
};

const char* cmp_instruction[CMP_NE + 1] = {
    "setl",
    "setle",
    "setg",
    "setge",
    "sete",
    "setne",
};

enum binary_operator {
    /* + */     BIN_ADD = 0,
    /* - */     BIN_SUB,
    /* * */     BIN_MUL,
    /* / */     BIN_DIV,
    /* % */     BIN_MOD,
    /* << */    BIN_SHL,
    /* >> */    BIN_SAR,
    /* & */     BIN_AND,
    /* | */     BIN_OR,
};

const char* binary_code[BIN_OR + 1] = {
    /* + */     "  pop %rdi\n"
                "  add %rdi, %rax\n",

    /* - */     "  mov %rax, %rdi\n"
                "  pop %rax\n"
                "  sub %rdi, %rax\n",

    /* * */     "  pop %rdi\n"
                "  imul %rdi, %rax\n",

    /* / */     "  mov %rax, %rdi\n"
                "  pop %rax\n"
                "  cqo\n"
                "  idiv %rdi\n",

    /* % */     "  mov %rax, %rdi\n"
                "  pop %rax\n"
                "  cqo\n"
                "  idiv %rdi\n"
                "  mov %rdx, %rax\n",

    /* << */    "  mov %rax, %rcx\n"
                "  pop %rax\n"
                "  shl %cl, %rax\n",

    /* >> */    "  mov %rax, %rcx\n"
                "  pop %rax\n"
                "  sar %cl, %rax\n",

    /* & */     "  pop %rdi\n"
                "  and %rdi, %rax\n",

    /* | */     "  pop %rdi\n"
                "  or %rdi, %rax\n",
};

struct stack_var {
    char* name;
    unsigned long offset;
};

static struct {
    const char *file_name;
    unsigned long line;
} compiler_pos;

//
// Transform compiler_pos into a string for error logging.
//
static char* get_pos(void)
{
    char *str = malloc(500 * sizeof(char)); // should be enough, right? right??
    sprintf(str, "%s:%ld", compiler_pos.file_name, compiler_pos.line);
    return str; // the compiler should exit after calling this function, but if it leaks, oopsies.
}

//
// Allocate structure for a stack variable.
//
static struct stack_var* init_stack_var(const char* name, unsigned long offset)
{
    struct stack_var* ptr = (struct stack_var*) malloc(sizeof(struct stack_var));
    ptr->name = strdup(name);
    ptr->offset = offset;
    return ptr;
}

//
// Deallocate a stack variable structure.
//
static void free_stack_var(struct stack_var* ptr)
{
    free(ptr->name);
    free(ptr);
}

static void expression(struct compiler_args *args, FILE *in, FILE *out, int level);
static void declarations(struct compiler_args *args, FILE *in, FILE *buffer);
static int subprocess(const char *arg0, const char *p_name, char *const *p_arg);

//
// Print message with prefix "error:".
//
#ifdef __GNUC__
__attribute((format(printf, 2, 3)))
#endif
void eprintf(const char *arg0, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, COLOR_BOLD_WHITE "%s: " COLOR_BOLD_RED "error: " COLOR_RESET, arg0);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

//
// Concatenate two strings into a dynamically allocated buffer.
//
char *concat(const char *a, const char *b)
{
    unsigned len = strlen(a) + strlen(b) + 1;
    char *result = calloc(1, len);
    if (!result) {
        fprintf(stderr, "out of memory in concat()\n");
        exit(1);
    }
    strcpy(result, a);
    strcat(result, b);
    return result;
}

//
// Run compiler with given arguments.
//
int compile(struct compiler_args *args)
{
    // create a buffer for the assembly code
    char* buf;
    char* asm_file = args->do_assembling ? concat(args->output_file, ".s") : args->output_file;
    char* obj_file = args->do_linking ? concat(args->output_file, ".o") : args->output_file;
    size_t buf_len, len, i;
    FILE *buffer = open_memstream(&buf, &buf_len);
    FILE *out, *in;
    int exit_code;

    // open every provided `.b` file and generate assembly for it
    for (i = 0; i < (size_t) args->num_input_files; i++) {
        len = strlen(args->input_files[i]);
        struct stat sbuf;
        if (stat(args->input_files[i], &sbuf) != 0 || S_ISDIR(sbuf.st_mode)) {
            eprintf(args->arg0, "cannot open file " QUOTE_FMT("%s") ".\n", args->input_files[i]);
            return 1;
        }
        if (len >= 2 && args->input_files[i][len - 1] == 'b' && args->input_files[i][len - 2] == '.') {
            compiler_pos.file_name = args->input_files[i];
            compiler_pos.line = 1;
            if (!(in = fopen(args->input_files[i], "r"))) {
                eprintf(args->arg0, "%s: %s\ncompilation terminated.\n", args->input_files[i], strerror(errno));
                return 1;
            }
            declarations(args, in, buffer);
            fclose(in);
        }
    }

    // write the buffer to an assembly file
    fclose(buffer);
    if (!(out = fopen(asm_file, "w"))) {
        eprintf(args->arg0, "cannot open file " QUOTE_FMT("%s") " %s.\n", A_S, strerror(errno));
        return 1;
    }
    fwrite(buf, buf_len, 1, out);
    fclose(out);
    free(buf);

    if (args->do_assembling) {
        if ((exit_code = subprocess(args->arg0, "as", (char *const[]){
            "as",
            asm_file,
            "-o", obj_file,
            0
        }))) {
            eprintf(args->arg0, "error running assembler (exit code %d)\n", exit_code);
            return 1;
        }

        if (!args->save_temps)
            remove(asm_file);
    }

    if (args->do_linking) {
        if ((exit_code = subprocess(args->arg0, "ld", (char *const[]){
            "ld",
            "-static", "-nostdlib",
            obj_file,
            args->lib_dir, "-L/lib64", "-L/usr/local/lib",
            "-lb",
            "-o", args->output_file,
            "-z", "noexecstack",
            0
        }))) {
            eprintf(args->arg0, "error running linker (exit code %d)\n", exit_code);
            return 1;
        }

        if (!args->save_temps)
            remove(obj_file);
    }

    return 0;
}

//
// Execute a program as a sub-process.
// Wait for completion.
// Return error status.
//
static int subprocess(const char *arg0, const char *p_name, char *const *p_arg)
{
    fprintf(stdout, "%s", p_name);
    for (unsigned i = 1; p_arg[i]; i++) {
        fprintf(stdout, " %s", p_arg[i]);
    }
    fprintf(stdout, "\n");
    fflush(stdout);

    pid_t pid = fork();

    if (pid < 0)
    {
        eprintf(arg0, "error forking parent process " QUOTE_FMT("%s") "\n", arg0);
        exit(1);
    }

    if (pid == 0 && execvp(p_name, p_arg) == -1)
    {
        eprintf(arg0, "error executing " QUOTE_FMT("%s") ": %s\n", p_name, strerror(errno));
        exit(1);
    }

    int pid_status;
    if (waitpid(pid, &pid_status, 0) == -1)
    {
        eprintf(arg0, "error getting status of child process %d\n", pid);
        exit(1);
    }

    return WEXITSTATUS(pid_status);
}

//
// Parse a comment.
// It starts with /* and finishes with */.
//
static void comment(FILE *in)
{
    int c;

    while ((c = fgetc(in)) != EOF) {
        if (c == '\n') ++compiler_pos.line;
        if (c == '*') {
            if ((c = fgetc(in)) == '/')
                return;
            ungetc(c, in);
        }
    }

    eprintf(get_pos(), "unclosed comment, expect " QUOTE_FMT("*/") " to close the comment\n");
    exit(1);
}

//
// Skip whitespace characters and comments.
//
static void whitespace(struct compiler_args *args, FILE *in)
{
    (void) args;
    int c;

    while ((c = fgetc(in)) != EOF) {
        if (isspace(c)) {
            if (c == '\n') ++compiler_pos.line;
            continue;
        }

        if (c == '/') {
            if ((c = fgetc(in)) == '*') {
                comment(in);
                continue;
            }
            else {
                ungetc(c, in);
                ungetc('/', in);
                return;
            }
        }
        ungetc(c, in);
        return;
    }
}

//
// Parse an identifier.
// It may include alphanumeric characters or underscore.
//
static int identifier(struct compiler_args *args, FILE *in, char* buffer)
{
    int read = 0;
    int c;

    whitespace(args, in);

    while ((c = fgetc(in)) != EOF) {
        if (!isalpha(c) && !isalnum(c) && c != '_') {
            ungetc(c, in);
            buffer[read] = '\0';
            return read;
        }
        buffer[read++] = c;
    }
    buffer[read] = '\0';
    return read;
}

//
// Parse an integer literal, possibly empty.
// Leading zero means octal value.
//
static intptr_t number(struct compiler_args *args, FILE *in)
{
    intptr_t num = 0;
    int c, base;

    whitespace(args, in);
    c = fgetc(in);
    if (c == EOF) {
        return EOF;
    }
    if (c == '0') {
        base = 8;
    } else {
        base = 10;
    }
    while (isdigit(c)) {
        num = (num * base) + c -'0';
        c = fgetc(in);
        if (c == EOF) {
            return EOF;
        }
    }
    ungetc(c, in);
    return num;
}

//
// Parse a multi-character literal.
// Return value.
//
static intptr_t character(struct compiler_args *args, FILE *in)
{
    int c = 0;
    int i;
    intptr_t value = 0;

    for (i = 0; i < args->word_size; i++) {
        if ((c = fgetc(in)) == '\'') {
            return value;
        }

        if (c == '*') {
            switch (c = fgetc(in)) {
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
            case 'r':
                c = '\r';
                break;
            default:
                eprintf(get_pos(), "undefined escape character " QUOTE_FMT("*%c") "\n", c);
                exit(1);
            }
        }

        // Little endian.
        value |= ((uintptr_t) (uint8_t) c) << (i * 8);
    }

    if (fgetc(in) != '\'') {
        eprintf(get_pos(), "unclosed char literal\n");
        exit(1);
    }

    return value;
}

//
// Parse a string literal.
//
static void string(struct compiler_args *args, FILE *in)
{
    int c;
    size_t alloc = 32;
    size_t size = 0;
    char *string = (char*) calloc(alloc, sizeof(char));

    while ((c = fgetc(in)) != '"') {
        if (c == '*') {
            switch (c = fgetc(in)) {
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
                eprintf(get_pos(), "undefined escape character " QUOTE_FMT("*%c") "\n", c);
                exit(1);
            }
        }
        else if (c == EOF) {
            eprintf(get_pos(), "unterminated string literal\n");
            exit(1);
        }
        string[size] = c;
        size++;
        if (size >= alloc)
            string = (char*) realloc(string, (alloc *= 2) * sizeof(char));
    }
    string[size] = 0;
    list_push(&args->strings, string);
}

//
// Parse one initialization value.
// It can be:
//      integer literal
//      negative integer literal
//      'char'
//      "string"
//
static void ival(struct compiler_args *args, FILE *in, FILE *out)
{
    static char buffer[BUFSIZ];
    intptr_t value;
    int c = fgetc(in);

    if (isalpha(c)) {
        ungetc(c, in);
        if (identifier(args, in, buffer) == EOF) {
            eprintf(get_pos(), "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .quad %s\n", buffer);
    }
    else if (c == '\'') {
        if ((value = character(args, in)) == EOF) {
            eprintf(get_pos(), "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .quad %lu\n", value);
    }
    else if (c == '\"') {
        string(args, in);
        fprintf(out, "  .quad .string.%lu\n", args->strings.size - 1);
    }
    else if (c == '-') {
        if ((value = number(args, in)) == EOF) {
            eprintf(get_pos(), "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .quad -%lu\n", value);
    }
    else {
        ungetc(c, in);
        if ((value = number(args, in)) == EOF) {
            eprintf(get_pos(), "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .quad %lu\n", value);
    }
}

//
// Parse declaration of a global scalar variable.
// An optional initialization list can be present.
//
static void global(struct compiler_args *args, FILE *in, FILE *out, char *identifier)
{
    fprintf(out,
        ".data\n"
        ".type %s, @object\n"
        ".align %d\n"
        "%s:\n",
        identifier, args->word_size, identifier
    );

    int c;
    if ((c = fgetc(in)) != ';') {
        ungetc(c, in);
        do {
            whitespace(args, in);
            ival(args, in, out);
            whitespace(args, in);
        } while ((c = fgetc(in)) == ',');

        if (c != ';') {
            eprintf(get_pos(), "expect " QUOTE_FMT(";") " at end of declaration\n");
            exit(1);
        }
    }
    else
        fprintf(out, "  .zero %d\n", args->word_size);
}

//
// Parse declaration of a global array.
// An optional initialization list can be present.
//
static void vector(struct compiler_args *args, FILE *in, FILE *out, char *identifier)
{
    intptr_t nwords = 0;
    int c;

    whitespace(args, in);
    if ((c = fgetc(in)) != ']') {
        ungetc(c, in);
        nwords = number(args, in);
        if (nwords == EOF) {
            eprintf(get_pos(), "unexpected end of file, expect vector size after " QUOTE_FMT("[") "\n");
            exit(1);
        }
        whitespace(args, in);

        if (fgetc(in) != ']') {
            eprintf(get_pos(), "expect " QUOTE_FMT("]") " after vector size\n");
            exit(1);
        }
    }

    fprintf(out,
        ".data\n.type %s, @object\n"
        ".align %d\n"
        "%s:\n"
        "  .quad .+8\n",
        identifier, args->word_size, identifier
    );

    whitespace(args, in);

    if ((c = fgetc(in)) != ';') {
        ungetc(c, in);
        do {
            whitespace(args, in);
            ival(args, in, out);
            whitespace(args, in);
            nwords--;
        } while ((c = fgetc(in)) == ',');

        if (c != ';') {
            eprintf(get_pos(), "expect " QUOTE_FMT(";") " at end of declaration\n");
            exit(1);
        }
    }

    if (nwords > 0)
        fprintf(out, "  .zero %ld\n", args->word_size * nwords);
}

//
// Find given name among locals or externs of current function.
//
static intptr_t find_identifier(struct compiler_args *args, const char *buffer, bool *is_extrn)
{
    size_t i;
    struct stack_var* var;

    for (i = 0; i < args->locals.size; i++) {
        var = (struct stack_var*) args->locals.data[i];
        if (strcmp(buffer, var->name) == 0) {
            if (is_extrn)
                *is_extrn = false;
            return var->offset;
        }
    }

    for (i = 0; i < args->extrns.size; i++) {
        if (strcmp(buffer, args->extrns.data[i]) == 0) {
            if (is_extrn)
                *is_extrn = true;
            return i;
        }
    }

    return -1;
}

//
// Parse a postfix operation.
// Return true when result is lvalue (address of the value).
//
static bool postfix(struct compiler_args *args, FILE *in, FILE *out, bool is_lvalue)
{
    int c, num_args = 0;

    switch (c = fgetc(in)) {
    case '[':
        /* index operator */
        fprintf(out, "  push (%%rax)\n");
        expression(args, in, out, 15);
        fprintf(out, "  pop %%rdi\n  shl $3, %%rax\n  add %%rdi, %%rax\n");

        if ((c = fgetc(in)) != ']') {
            eprintf(get_pos(), "unexpected token " QUOTE_FMT("%c") ", expect closing " QUOTE_FMT("]") " after index expression\n", c);
            exit(1);
        }
        is_lvalue = true;
        break;

    case '(':
        /* function call */
        fprintf(out, "  push %%rax\n");

        while ((c = fgetc(in)) != ')') {
            ungetc(c, in);
            expression(args, in, out, 15);

            if (++num_args > MAX_FN_CALL_ARGS) {
                eprintf(get_pos(), "only %d call arguments are currently supported\n", MAX_FN_CALL_ARGS);
                exit(1);
            }
            fprintf(out, "  push %%rax\n");

            whitespace(args, in);
            if ((c = fgetc(in)) == ')')
                break;
            else if (c == ',')
                continue;

            eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect closing " QUOTE_FMT(")") " after call expression\n", c);
            exit(1);
        }

        while (num_args > 0)
            fprintf(out, "  pop %s\n", arg_registers[--num_args]);

        fprintf(out, "  pop %%r10\n  call *%%r10\n");
        is_lvalue = false;
        break;

    case '+':
        if ((c = fgetc(in)) != '+') {
            ungetc(c, in);
            ungetc('+', in);
            break;
        }

        /* postfix increment operator */
        fprintf(out,
            "  mov (%%rax), %%rcx\n"
            "  addq $1, (%%rax)\n"
            "  mov %%rcx, %%rax\n"
        );
        is_lvalue = false;
        break;

    case '-':
        if ((c = fgetc(in)) != '-') {
            ungetc(c, in);
            ungetc('-', in);
            break;
        }

        /* postfix decrement operator */
        fprintf(out,
            "  mov (%%rax), %%rcx\n"
            "  subq $1, (%%rax)\n"
            "  mov %%rcx, %%rax\n"
        );
        is_lvalue = false;
        break;

    default:
        ungetc(c, in);
        break;
    }
    return is_lvalue;
}

//
// Parse a term.
// It may have only unary operations (no binary ops).
// Return true when it's an lvalue (address of the value).
//
static bool term(struct compiler_args *args, FILE *in, FILE *out)
{
    static char buffer[BUFSIZ];
    int c;
    intptr_t value;
    bool is_lvalue = false, is_extrn = false;

    whitespace(args, in);

    switch (c = fgetc(in)) {
    case '\'': /* character literal */
        if ((value = character(args, in)))
            fprintf(out, "  mov $%lu, %%rax\n", value);
        else
            fprintf(out, "  xor %%rax, %%rax\n");
        break;

    case '\"': /* string literal */
        string(args, in);
        fprintf(out, "  lea .string.%lu(%%rip), %%rax\n", args->strings.size - 1);
        break;

    case '(': /* parentheses */
        expression(args, in, out, 15);
        ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after " QUOTE_FMT("(<expr>") ", got " QUOTE_FMT("%c") "\n", c);
        break;

    case '!': /* not operator */
        if (term(args, in, out)) {
            /* fetch rvalue */
            fprintf(out, "  mov (%%rax), %%rax\n");
        }
        fprintf(out, "  cmp $0, %%rax\n  sete %%al\n  movzx %%al, %%rax\n");
        break;

    case '-':
        if ((c = fgetc(in)) == '-') { /* prefix decrement operator */
            if (!term(args, in, out)) {
                eprintf(get_pos(), "expected lvalue after " QUOTE_FMT("--") "\n");
                exit(1);
            }
            fprintf(out, "  mov (%%rax), %%rdi\n  sub $1, %%rdi\n  mov %%rdi, (%%rax)\n");
            is_lvalue = true;
        }
        else { /* negation operator */
            ungetc(c, in);
            if (term(args, in, out)) {
                /* fetch rvalue */
                fprintf(out, "  mov (%%rax), %%rax\n");
            }
            fprintf(out, "  neg %%rax\n");
        }
        break;

    case '+': /* prefix increment operator */
        if ((c = fgetc(in)) != '+') {
            eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT("+") "\n", c);
            exit(1);
        }
        if (!term(args, in, out)) {
            eprintf(get_pos(), "expected lvalue after " QUOTE_FMT("++") "\n");
            exit(1);
        }
        fprintf(out, "  mov (%%rax), %%rdi\n  add $1, %%rdi\n  mov %%rdi, (%%rax)\n");
        is_lvalue = true;
        break;

    case '*': /* indirection operator */
        if (term(args, in, out)) {
            /* fetch rvalue */
            fprintf(out, "  mov (%%rax), %%rax\n");
        }
        is_lvalue = true;
        break;

    case '&': /* address operator */
        if (!term(args, in, out)) {
            eprintf(get_pos(), "expected lvalue after " QUOTE_FMT("&") "\n");
            exit(1);
        }
        break;

    case EOF:
        eprintf(get_pos(), "unexpected end of file, expect expression\n");
        exit(1);

    default:
        if (isdigit(c)) { /* integer literal */
            ungetc(c, in);
            if ((value = number(args, in)))
                fprintf(out, "  mov $%lu, %%rax\n", value);
            else
                fprintf(out, "  xor %%rax, %%rax\n");
        }
        else if (isalpha(c)) { /* identifier */
            is_lvalue = true;

            ungetc(c, in);
            identifier(args, in, buffer);

            if ((value = find_identifier(args, buffer, &is_extrn)) < 0) {

                // Unknown identifier.
                whitespace(args, in);
                c = fgetc(in);
                if (c == '(') {
                    // When next symbol is '(', add this name to the list of externals.
                    ungetc(c, in);
                    list_push(&args->extrns, strdup(buffer));
                    is_extrn = true;
                } else {
                    eprintf(get_pos(), "undefined identifier " QUOTE_FMT("%s") "\n", buffer);
                    exit(1);
                }
            }

            if (is_extrn)
                fprintf(out, "  lea %s(%%rip), %%rax\n", buffer);
            else
                fprintf(out, "  lea -%lu(%%rbp), %%rax\n", (value + 2) * args->word_size);

            is_lvalue = postfix(args, in, out, is_lvalue);
        }
        else {
            eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect expression\n", c);
            exit(1);
        }
    }

    return is_lvalue;
}

//
// Generate code for binary operation.
//
static void binary_expr(struct compiler_args *args, FILE *in, FILE *out, enum binary_operator op, int level)
{
    fprintf(out, "  push %%rax\n");
    expression(args, in, out, level);
    fputs(binary_code[op], out);
}

//
// Generate code for comparison operation.
//
static void cmp_expr(struct compiler_args *args, FILE *in, FILE *out, enum cmp_operator op, int level)
{
    fprintf(out, "  push %%rax\n");
    expression(args, in, out, level);
    fprintf(out,
        "  pop %%rdi\n"
        "  cmp %%rax, %%rdi\n"
        "  %s %%al\n"
        "  movzb %%al, %%rax\n",
        cmp_instruction[op]
    );
}

//
// Generate code for assignment operation:
//      =+
//      =-
//      =*
//      =/
//      =%
//      =<<
//      =<=
//      =<
//      =>>
//      =>=
//      =>
//      =!=
//      ===
//      =&
//      =|
//
static void assign_expr(struct compiler_args *args, FILE *in, FILE *out, char c, int level)
{
    switch (c) {
    case '+': /* addition operator */
        binary_expr(args, in, out, BIN_ADD, level);
        break;

    case '*': /* multiplication operator */
        binary_expr(args, in, out, BIN_MUL, level);
        break;

    case '-': /* subtraction operator */
        binary_expr(args, in, out, BIN_SUB, level);
        break;

    case '/': /* division operator */
        binary_expr(args, in, out, BIN_DIV, level);
        break;

    case '%': /* modulo operator */
        binary_expr(args, in, out, BIN_MOD, level);
        break;

    case '<':
        switch (c = fgetc(in)) {
        case '<': /* shift-left operator */
            binary_expr(args, in, out, BIN_SHL, level);
            break;
        case '=': /* less-than-or-equal operator */
            cmp_expr(args, in, out, CMP_LE, level);
            break;
        default: /* less-than operator */
            ungetc(c, in);
            cmp_expr(args, in, out, CMP_LT, level);
        }
        break;

    case '>':
        switch (c = fgetc(in)) {
        case '>': /* shift-right-operator */
            binary_expr(args, in, out, BIN_SAR, level);
            break;
        case '=': /* greater-than-or-equal operator */
            cmp_expr(args, in, out, CMP_GE, level);
            break;
        default: /* greater-than operator */
            ungetc(c, in);
            cmp_expr(args, in, out, CMP_GT, level);
        }
        break;

    case '!': /* inequality operator */
        if ((c = fgetc(in)) != '=') {
            eprintf(get_pos(), "unknown operator " QUOTE_FMT("!%c") "\n", c);
            exit(1);
        }
        cmp_expr(args, in, out, CMP_NE, level);
        break;

    case '=': /* equality operator */
        if ((c = fgetc(in)) != '=') {
            eprintf(get_pos(), "unknown operator " QUOTE_FMT("=%c") "\n", c);
            exit(1);
        }
        cmp_expr(args, in, out, CMP_EQ, level);
        break;

    case '&': /* bitwise and operator */
        binary_expr(args, in, out, BIN_AND, level);
        break;

    case '|': /* bitwise or operator */
        binary_expr(args, in, out, BIN_OR, level);
        break;

    default: /* plain assignment */
        ungetc(c, in);
        expression(args, in, out, level);
    }
}

//
// Parse expression.
// Allow operations up to the given precedence level.
//
static void expression(struct compiler_args *args, FILE *in, FILE *out, int level)
{
    bool left_is_lvalue = term(args, in, out);
    int c, c2;
    static size_t conditional = 0;

    for (;;) {
        whitespace(args, in);
        c = fgetc(in);

        if (level >= 13 && c == '?') {
            /* ternary operators have the lowest precedence, so they need to be resolved here */
            size_t this_conditional = conditional++;

            if (left_is_lvalue) {
                /* fetch rvalue */
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            fprintf(out, "  cmp $0, %%rax\n  je .L.cond.else.%ld\n", this_conditional);
            expression(args, in, out, 12);
            whitespace(args, in);
            if ((c2 = fgetc(in)) != ':') {
                eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(":") " between conditional branches\n", c2);
                exit(1);
            }
            fprintf(out, "  jmp .L.cond.end.%ld\n.L.cond.else.%ld:\n", this_conditional, this_conditional);
            expression(args, in, out, 13);
            fprintf(out, ".L.cond.end.%ld:\n", this_conditional);
            return;
        }

        //
        // Binary operations, left assosiative.
        //
        if (level >= 4 && c == '+') {
            /* addition operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_ADD, 3);
            continue;
        }
        if (level >= 4 && c == '-') {
            /* subtraction operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_SUB, 3);
            continue;
        }
        if (level >= 3 && c == '*') {
            /* multiplication operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_MUL, 2);
            continue;
        }
        if (level >= 3 && c == '/') {
            /* division operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_DIV, 2);
            continue;
        }
        if (level >= 3 && c == '%') {
            /* modulo operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_MOD, 2);
            continue;
        }
        if (c == '<') {
            c2 = fgetc(in);
            if (level >= 5 && c2 == '<') {
                /* shift-left operator */
                if (left_is_lvalue) {
                    fprintf(out, "  mov (%%rax), %%rax\n");
                    left_is_lvalue = false;
                }
                binary_expr(args, in, out, BIN_SHL, 4);
                continue;
            }
            if (level >= 6 && c2 == '=') {
                /* less-than-or-equal operator */
                if (left_is_lvalue) {
                    fprintf(out, "  mov (%%rax), %%rax\n");
                    left_is_lvalue = false;
                }
                cmp_expr(args, in, out, CMP_LE, 5);
                continue;
            }
            ungetc(c2, in);
            if (level >= 6) {
                /* less-than operator */
                if (left_is_lvalue) {
                    fprintf(out, "  mov (%%rax), %%rax\n");
                    left_is_lvalue = false;
                }
                cmp_expr(args, in, out, CMP_LT, 5);
                continue;
            }
        }
        if (c == '>') {
            c2 = fgetc(in);
            if (level >= 5 && c2 == '>') {
                /* shift-right-operator */
                if (left_is_lvalue) {
                    fprintf(out, "  mov (%%rax), %%rax\n");
                    left_is_lvalue = false;
                }
                binary_expr(args, in, out, BIN_SAR, 4);
                continue;
            }
            if (level >= 6 && c2 == '=') {
                /* greater-than-or-equal operator */
                if (left_is_lvalue) {
                    fprintf(out, "  mov (%%rax), %%rax\n");
                    left_is_lvalue = false;
                }
                cmp_expr(args, in, out, CMP_GE, 5);
                continue;
            }
            ungetc(c2, in);
            if (level >= 6) {
                /* greater-than operator */
                if (left_is_lvalue) {
                    fprintf(out, "  mov (%%rax), %%rax\n");
                    left_is_lvalue = false;
                }
                cmp_expr(args, in, out, CMP_GT, 5);
                continue;
            }
        }
        if (level >= 7 && c == '!') {
            /* inequality operator */
            if ((c2 = fgetc(in)) != '=') {
                eprintf(get_pos(), "unknown operator " QUOTE_FMT("!%c") "\n", c2);
                exit(1);
            }
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            cmp_expr(args, in, out, CMP_NE, 6);
            continue;
        }
        if (level >= 8 && c == '&') {
            /* bitwise and operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_AND, 7);
            continue;
        }
        if (level >= 10 && c == '|') {
            /* bitwise or operator */
            if (left_is_lvalue) {
                fprintf(out, "  mov (%%rax), %%rax\n");
                left_is_lvalue = false;
            }
            binary_expr(args, in, out, BIN_OR, 9);
            continue;
        }
        if (c == '=') {
            c2 = fgetc(in);
            if (level >= 7 && c2 == '=') {
                int c3 = fgetc(in);
                ungetc(c3, in);
                if (c3 != '=') {
                    /* equality operator */
                    if (left_is_lvalue) {
                        fprintf(out, "  mov (%%rax), %%rax\n");
                        left_is_lvalue = false;
                    }
                    cmp_expr(args, in, out, CMP_EQ, 6);
                    continue;
                }
            }
            if (level >= 14) {
                //
                // Assignment operator, right associative.
                //
                if (!left_is_lvalue) {
                    eprintf(get_pos(), "left operand of assignment has to be an lvalue\n");
                    exit(1);
                }
                fprintf(out, "  push %%rax\n  mov (%%rax), %%rax\n");
                assign_expr(args, in, out, c2, 14);
                fprintf(out, "  pop %%rdi\n  mov %%rax, (%%rdi)\n");
                left_is_lvalue = false;
                continue;
            }
            ungetc(c2, in);
        }

        // No more operations at this level.
        ungetc(c, in);
        if (left_is_lvalue) {
            /* fetch rvalue */
            fprintf(out, "  mov (%%rax), %%rax\n");
        }
        return;
    }
}

//
// Parse a statement.
//
static void statement(struct compiler_args *args, FILE *in, FILE *out,
                      char* fn_ident, intptr_t switch_id, struct list *cases)
{
    int c;
    static char buffer[BUFSIZ];
    size_t id;
    static size_t stmt_id = 0; /* unique id for each statement for generating labels */
    intptr_t i, value = 0;
    struct list switch_case_list;

    whitespace(args, in);
    switch (c = fgetc(in)) {
    case '{': {
        unsigned long stack_offset = args->stack_offset;

        whitespace(args, in);
        while ((c = fgetc(in)) != '}') {
            ungetc(c, in);
            statement(args, in, out, fn_ident, switch_id, cases);
            whitespace(args, in);
        }

        // reset stack so variables in loops don't overflow the stack
        if (stack_offset != args->stack_offset) {
            fprintf(out, "  add $%lu, %%rsp\n", (args->stack_offset - stack_offset) * args->word_size);
            args->stack_offset = stack_offset;
        }
        }
        break;

    case ';':
        break; /* null statement */

    default:
        if (isalpha(c)) {
            ungetc(c, in);
            identifier(args, in, buffer);
            whitespace(args, in);

            if (strcmp(buffer, "goto") == 0) { /* goto statement */
                if (!identifier(args, in, buffer)) {
                    eprintf(get_pos(), "expect label name after " QUOTE_FMT("goto") "\n");
                    exit(1);
                }
                fprintf(out, "  jmp .L.label.%s.%s\n", buffer, fn_ident);
                whitespace(args, in);
                ASSERT_CHAR(args, in, ';', "expect " QUOTE_FMT(";") " after " QUOTE_FMT("goto") " statement\n");
                return;
            }
            else if (strcmp(buffer, "return") == 0) { /* return statement */
                if ((c = fgetc(in)) != ';') {
                    if (c != '(') {
                        eprintf(get_pos(), "expect " QUOTE_FMT("(") " or " QUOTE_FMT(";") " after " QUOTE_FMT("return") "\n");
                        exit(1);
                    }
                    expression(args, in, out, 15);
                    whitespace(args, in);
                    ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after " QUOTE_FMT("return") " statement\n");
                    whitespace(args, in);
                    ASSERT_CHAR(args, in, ';', "expect " QUOTE_FMT(";") " after " QUOTE_FMT("return") " statement\n");
                }
                else
                    fprintf(out, "  xor %%rax, %%rax\n");
                fprintf(out, "  jmp .L.return.%s\n", fn_ident);
                return;
            }
            else if (strcmp(buffer, "if") == 0) { /* conditional statement */
                id = stmt_id++;

                ASSERT_CHAR(args, in, '(', "expect " QUOTE_FMT("(") " after " QUOTE_FMT("if") "\n");
                expression(args, in, out, 15);
                fprintf(out, "  cmp $0, %%rax\n  je .L.else.%lu\n", id);
                whitespace(args, in);
                ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after condition\n");

                statement(args, in, out, fn_ident, -1, NULL);
                fprintf(out, "  jmp .L.end.%lu\n.L.else.%lu:\n", id, id);

                whitespace(args, in);
                memset(buffer, 0, 6 * sizeof(char));
                if ((buffer[0] = fgetc(in)) == 'e' &&
                   (buffer[1] = fgetc(in)) == 'l' &&
                   (buffer[2] = fgetc(in)) == 's' &&
                   (buffer[3] = fgetc(in)) == 'e' &&
                   !isalnum((buffer[4] = fgetc(in)))) {
                    statement(args, in, out, fn_ident, -1, NULL);
                }
                else {
                    for (i = 4; i >= 0; i--) {
                        if (buffer[i])
                            ungetc(buffer[i], in);
                    }
                }

                fprintf(out, ".L.end.%lu:\n", id);
                return;
            }
            else if (strcmp(buffer, "while") == 0) { /* while statement */
                id = stmt_id++;

                ASSERT_CHAR(args, in, '(', "expect " QUOTE_FMT("(") " after " QUOTE_FMT("while") "\n");
                fprintf(out, ".L.start.%lu:\n", id);
                expression(args, in, out, 15);
                fprintf(out,
                    "  cmp $0, %%rax\n"
                    "  je .L.end.%lu\n",
                    id
                );
                whitespace(args, in);
                ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after condition\n");

                statement(args, in, out, fn_ident, -1, NULL);
                fprintf(out, "  jmp .L.start.%lu\n.L.end.%lu:\n", id, id);
                return;
            }
            else if (strcmp(buffer, "switch") == 0) { /* switch statement */
                id = stmt_id++;

                expression(args, in, out, 15);
                fprintf(out, "  jmp .L.cmp.%ld\n.L.stmts.%ld:\n", id, id);

                memset(&switch_case_list, 0, sizeof(struct list));
                statement(args, in, out, fn_ident, id, &switch_case_list);
                fprintf(out,
                    "  jmp .L.end.%ld\n"
                    ".L.cmp.%ld:\n",
                    id, id
                );

                for (i = 0; i < (intptr_t) switch_case_list.size; i++)
                    fprintf(out, "  cmp $%lu, %%rax\n  je .L.case.%lu.%lu\n", (uintptr_t) switch_case_list.data[i], id, (uintptr_t) switch_case_list.data[i]);

                fprintf(out, ".L.end.%ld:\n", id);

                list_free(&switch_case_list);
                return;
            }
            else if (strcmp(buffer, "case") == 0) { /* case statement */
                id = stmt_id++;

                if (switch_id < 0) {
                    eprintf(get_pos(), "unexpected " QUOTE_FMT("case") " outside of " QUOTE_FMT("switch") " statements\n");
                    exit(1);
                }

                switch (c = fgetc(in)) {
                case '\'':
                    value = character(args, in);
                    break;
                default:
                    if (isdigit(c)) {
                        ungetc(c, in);
                        value = number(args, in);
                        break;
                    }

                    eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect constant after " QUOTE_FMT("case") "\n", c);
                    exit(1);
                }

                if ((intptr_t) value == EOF) {
                    eprintf(get_pos(), "unexpected end of file, expect constant after " QUOTE_FMT("case") "\n");
                    exit(1);
                }
                whitespace(args, in);
                ASSERT_CHAR(args, in, ':', "expect " QUOTE_FMT(":") " after " QUOTE_FMT("case") "\n");
                list_push(cases, (void*) value);

                fprintf(out, ".L.case.%ld.%lu:\n", switch_id, value);
                statement(args, in, out, fn_ident, switch_id, cases);
                return;
            }
            else if (strcmp(buffer, "extrn") == 0) { /* external declaration */
                do {
                    if (!identifier(args, in, buffer)) {
                        eprintf(get_pos(), "expect identifier after " QUOTE_FMT("extrn") "\n");
                        exit(1);
                    }

                    if (find_identifier(args, buffer, NULL) >= 0) {
                        eprintf(get_pos(), "identifier " QUOTE_FMT("%s") " is already defined in this scope\n", buffer);
                        exit(1);
                    }

                    list_push(&args->extrns, strdup(buffer));

                    whitespace(args, in);
                } while ((c = fgetc(in)) == ',');

                if (c != ';') {
                    eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(";") " or " QUOTE_FMT(",") "\n", c);
                    exit(1);
                }
                return;
            }
            else if (strcmp(buffer, "auto") == 0) {
                do {
                    if (!identifier(args, in, buffer)) {
                        eprintf(get_pos(), "expect identifier after " QUOTE_FMT("auto") "\n");
                        exit(1);
                    }
                    if (find_identifier(args, buffer, NULL) >= 0) {
                        eprintf(get_pos(), "identifier " QUOTE_FMT("%s") " is already defined in this scope\n", buffer);
                        exit(1);
                    }
                    whitespace(args, in);

                    value = -1;
                    if ((c = fgetc(in)) == '\'') {
                        value = character(args, in);
                        whitespace(args, in);
                        c = fgetc(in);
                    }
                    else if (c == '[') {
                        value = number(args, in);
                        whitespace(args, in);
                        if ((c = fgetc(in)) != ']') {
                            eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT("]") "\n", c);
                            exit(1);
                        }
                        whitespace(args, in);
                        c = fgetc(in);
                    }
                    else if (isdigit(c)) {
                        ungetc(c, in);
                        value = number(args, in);
                        whitespace(args, in);
                        c = fgetc(in);
                    }

                    if (value < 0) {
                        // Scalar.
                        list_push(&args->locals, init_stack_var(buffer, args->stack_offset));
                        args->stack_offset += 1;
                        fprintf(out, "  sub $%u, %%rsp\n", args->word_size);
                    } else {
                        // Vector.
                        list_push(&args->locals, init_stack_var(buffer, args->stack_offset + value));
                        args->stack_offset += value + 1;
                        fprintf(out, "  sub $%lu, %%rsp\n", args->word_size * (value + 1));

                        // Initialize pointer.
                        fprintf(out, "  lea -%lu(%%rbp), %%rax\n", args->stack_offset * args->word_size);
                        fprintf(out, "  movq %%rax, -%lu(%%rbp)\n", (args->stack_offset + 1) * args->word_size);
                    }
                } while ((c) == ',');

                if (c != ';') {
                    eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(";") " or " QUOTE_FMT(",") "\n", c);
                    exit(1);
                }

                // align stack to 16 bytes
                if (args->stack_offset % 2) {
                    fprintf(out, "  sub $%u, %%rsp\n", args->word_size);
                    args->stack_offset++;
                }
                return;
            }
            else {
                switch (c = fgetc(in)) {
                case ':': /* label */
                    fprintf(out, ".L.label.%s.%s:\n", buffer, fn_ident);
                    statement(args, in, out, fn_ident, switch_id, cases);
                    return;
                default:
                    ungetc(c, in);
                    for (i = strlen(buffer) - 1; i >= 0; i--)
                        ungetc(buffer[i], in);

                    expression(args, in, out, 15);
                    whitespace(args, in);
                    if ((c = fgetc(in)) != ';') {
                        eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(";") " after expression statement\n", c);
                        exit(1);
                    }
                }
            }
        }
        else if (c == EOF) {
            eprintf(get_pos(), "unexpected end of file, expect statement\n");
            exit(1);
        }
        else {
            ungetc(c, in);
            expression(args, in, out, 15);
            whitespace(args, in);
            if ((c = fgetc(in)) != ';') {
                eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") " expect " QUOTE_FMT(";") " after expression statement\n", c);
                exit(1);
            }
        }
    }
}

//
// Parse a list of function arguments.
//
static void arguments(struct compiler_args *args, FILE *in, FILE *out)
{
    int c;
    int i = 0;
    static char buffer[BUFSIZ];

    while (1) {
        whitespace(args, in);
        if (!identifier(args, in, buffer)) {
            eprintf(get_pos(), "expect " QUOTE_FMT(")") " or identifier after function arguments\n");
            exit(1);
        }
        fprintf(out, "  sub $%u, %%rsp\n  mov %s, -%lu(%%rbp)\n", args->word_size, arg_registers[i++], (args->stack_offset + 2) * args->word_size);

        list_push(&args->locals, init_stack_var(strdup(buffer), args->stack_offset++));

        whitespace(args, in);
        switch (c = fgetc(in)) {
            case ')':
                return;
            case ',':
                continue;
            default:
                eprintf(get_pos(), "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(")") " or " QUOTE_FMT(",") "\n", c);
                exit(1);
        }
    }
}

//
// Parse a function definition.
//
static void function(struct compiler_args *args, FILE *in, FILE *out, char *fn_id)
{
    size_t i;
    int c;

    // Clear the list of locals.
    for (i = 0; i < args->locals.size; i++)
        free_stack_var((struct stack_var*) args->locals.data[i]);
    list_clear(&args->locals);
    args->stack_offset = 0;

    // Clear the list of externals.
    for (i = 0; i < args->extrns.size; i++)
        if (args->extrns.data[i] != fn_id)
            free(args->extrns.data[i]);
    list_clear(&args->extrns);

    // Add name of the function to externals.
    list_push(&args->extrns, fn_id);

    fprintf(out,
        ".text\n"
        ".type %s, @function\n"
        "%s:\n"
        "  push %%rbp\n"
        "  mov %%rsp, %%rbp\n"
        "  sub $%d, %%rsp\n",
        fn_id, fn_id, args->word_size
    );

    if ((c = fgetc(in)) != ')') {
        ungetc(c, in);
        arguments(args, in, out);
    }

    statement(args, in, out, fn_id, -1, NULL);

    fprintf(out,
        "  xor %%rax, %%rax\n"
        ".L.return.%s:\n"
        "  mov %%rbp, %%rsp\n"
        "  pop %%rbp\n"
        "  ret\n",
        fn_id
    );
}

//
// Create read-only section with strings.
//
static void strings(struct compiler_args *args, FILE *out)
{
    char *string;
    size_t i, j, size;

    fprintf(out, ".section .rodata\n");

    for (i = 0; i < args->strings.size; i++) {
        fprintf(out, ".string.%lu:\n", i);

        string = (char*) args->strings.data[i];
        size = strlen(string);
        for (j = 0; j < size; j++)
            fprintf(out, "  .byte %u\n", string[j]);
        fprintf(out, "  .byte 0\n");

        free(string);
    }

    list_free(&args->strings);
}

//
// Parse top level declarations:
//      name(...    -- function definition
//      name[...    -- vector declaration
//      name...     -- scalar declaration
//
static void declarations(struct compiler_args *args, FILE *in, FILE *out)
{
    static char buffer[BUFSIZ];
    int c;
    size_t i;

    while (identifier(args, in, buffer)) {
        fprintf(out, ".globl %s\n", buffer);

        switch (c = fgetc(in)) {
        case '(':
            function(args, in, out, buffer);
            break;

        case '[':
            vector(args, in, out, buffer);
            break;

        case EOF:
            eprintf(get_pos(), "unexpected end of file after declaration\n");
            exit(1);

        default:
            ungetc(c, in);
            global(args, in, out, buffer);
        }
    }

    if (fgetc(in) != EOF) {
        eprintf(get_pos(), "expect identifier at top level\n");
        exit(1);
    }

    strings(args, out);

    // Clear the list of locals.
    for (i = 0; i < args->locals.size; i++)
        free_stack_var((struct stack_var*) args->locals.data[i]);
    list_free(&args->locals);
    args->stack_offset = 0;

    // Clear the list of externals.
    for (i = 0; i < args->extrns.size; i++)
        if (args->extrns.data[i] != buffer)
            free(args->extrns.data[i]);
    list_free(&args->extrns);
}
