#include "compiler.h"
#include "list.h"
#include <stddef.h>

#define _XOPEN_SOURCE 700
#include <stdio.h>
#undef _XOPEN_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

#define ASSERT_CHAR(args, in, expect, message) do { \
    if(fgetc(in) != expect) {                       \
        eprintf(args->arg0, message);               \
        exit(1);                                    \
    }} while(0)

enum asm_register {
    RAX,
    RBX,
    RCX,
    RDX,
    RDI,
    RSI,
    RBP,
    RSP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15
};

static const char* registers[R15 + 1] = {
    "%rax",
    "%rbx",
    "%rcx",
    "%rdx",
    "%rdi",
    "%rsi",
    "%rbp",
    "%rsp",
    "%r8",
    "%r9",
    "%r10",
    "%r11",
    "%r12",
    "%r13",
    "%r14",
    "%r15"
};

#define MAX_FN_CALL_ARGS 6

static const char* arg_registers[MAX_FN_CALL_ARGS] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

static inline const char* strregister(enum asm_register reg) {
    return registers[reg];
}

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

static bool expression(struct compiler_args *args, FILE *in, FILE *out);
static void declarations(struct compiler_args *args, FILE *in, FILE *buffer);
static int subprocess(char *arg0, const char *p_name, char *const *p_arg);

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
    char* asm_file = A_S;
    char* obj_file = A_O;
    size_t buf_len, len, i;
    FILE *buffer = open_memstream(&buf, &buf_len);
    FILE *out, *in; 
    int exit_code;

    // open every provided `.b` file and generate assembly for it
    for(i = 0; i < (size_t) args->num_input_files; i++) {
        len = strlen(args->input_files[i]);
        if(len >= 2 && args->input_files[i][len - 1] == 'b' && args->input_files[i][len - 2] == '.') {
            if(!(in = fopen(args->input_files[i], "r"))) {
                eprintf(args->arg0, "%s: %s\ncompilation terminated.\n", args->input_files[i], strerror(errno));
                return 1;
            }
            declarations(args, in, buffer);
            fclose(in);
        }
    }

    for(i = 0; i < args->locals.size; i++)
        free(args->locals.data[i]);
    list_free(&args->locals);

    // write the buffer to an assembly file
    fclose(buffer);
    if(!(out = fopen(asm_file, "w"))) {
        eprintf(args->arg0, "cannot open file " QUOTE_FMT("%s") " %s.", A_S, strerror(errno));
        return 1;
    }
    fwrite(buf, buf_len, 1, out);
    fclose(out);
    free(buf);

    if(args->do_assembling) {
        if((exit_code = subprocess(args->arg0, "as", (char *const[]){
            "as", 
            asm_file,
            "-o", obj_file,
            0
        }))) {
            eprintf(args->arg0, "error running assembler (exit code %d)\n", exit_code);
            return 1;
        }

        remove(asm_file);
    }
    
    if(args->do_linking) {
        if((exit_code = subprocess(args->arg0, "ld", (char *const[]){
            "ld",
            "-static", "-nostdlib",
            obj_file,
            "-L.", "-L/lib64", "-L/usr/local/lib64",
            "-lb",
            "-o", args->output_file,
            0
        }))) {
            eprintf(args->arg0, "error running linker (exit code %d)\n", exit_code);
            return 1;
        }

        remove(obj_file);
    }

    return 0;
}

static int subprocess(char *arg0, const char *p_name, char *const *p_arg)
{
    pid_t pid = fork();

    if(pid < 0)
    {
        eprintf(arg0, "error forking parent process " QUOTE_FMT("%s") "\n", arg0);
        exit(1);
    }

    if(pid == 0 && execvp(p_name, p_arg) == -1)
    {
        eprintf(arg0, "error executing " QUOTE_FMT("%s") ": %s\n", p_name, strerror(errno));
        exit(1);
    }

    int pid_status;
    if(waitpid(pid, &pid_status, 0) == -1)
    {
        eprintf(arg0, "error getting status of child process %d\n", pid);
        exit(1);
    }

    return WEXITSTATUS(pid_status);
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
    char c = 0;
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
                eprintf(args->arg0, "undefined escape character " QUOTE_FMT("*%c"), c);
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
        fprintf(out, "  .quad %s\n", buffer);
    }
    else if(c == '\'') {
        if((value = character(args, in)) == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .quad %lu\n", value);
    }
    else {
        ungetc(c, in);
        if((value = number(in)) == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect ival\n");
            exit(1);
        }
        fprintf(out, "  .quad %lu\n", value);
    }
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
            eprintf(args->arg0, "expect " QUOTE_FMT(";") " at end of declaration\n");
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
            eprintf(args->arg0, "unexpected end of file, expect vector size after " QUOTE_FMT("[") "\n");
            exit(1);
        }
        whitespace(in);

        if(fgetc(in) != ']') {
            eprintf(args->arg0, "expect " QUOTE_FMT("]") " after vector size\n");
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
            eprintf(args->arg0, "expect " QUOTE_FMT(";") " at end of declaration\n");
            exit(1);
        }
    }
    else if((args->word_size * num) != 0)
        fprintf(out, "  .zero %ld\n", args->word_size * num);
}

static void cmp_expr(struct compiler_args *args, FILE *in, FILE *out, enum cmp_operator op)
{
    fprintf(out, "  push %%rax\n");
    expression(args, in, out);
    fprintf(out,
        "  pop %%rdi\n"
        "  cmp %%rax, %%rdi\n"
        "  %s %%al\n"
        "  movzb %%al, %%rax\n",
        cmp_instruction[op]
    );
}

static bool bin_op(struct compiler_args *args, FILE *in, FILE *out, char c) {
    switch(c) {
    case '+': /* addition operator */
    case '-': /* subtraction operator */
    case '*': /* multiplication operator */
        fprintf(out, "  push %%rax\n");
        expression(args, in, out);
        fprintf(out, "  pop %%rdi\n  %s %%rdi, %%rax\n", c == '+' ? "add" : c == '-' ? "sub" : "imul");
        break;
    
    case '/': /* division operator */
    case '%': /* modulo operator */
        fprintf(out, "  push %%rax\n");
        expression(args, in, out);
        fprintf(out, "  pop %%rdi\n  cqo\n  idiv %%rdi\n");
        if(c == '%')
            fprintf(out, "  mov %%rdx, %%rdi\n");
        break;
    
    case '<':
        switch(c = fgetc(in)) {
        case '<': /* shift-left operator */
            fprintf(out, "  push %%rax\n");
            expression(args, in, out);
            fprintf(out, "  mov %%rax, %%rcx\n  pop %%rax\n  shl %%cl, %%rax\n");
            break;
        case '=': /* less-than-or-equal operator */
            cmp_expr(args, in, out, CMP_LE);
            break;
        default: /* less-than operator */
            ungetc(c, in);
            cmp_expr(args, in, out, CMP_LT);
        }
        break;

    case '>':
        switch(c = fgetc(in)) {
        case '>': /* shift-right-operator */
            fprintf(out, "  push %%rax\n");
            expression(args, in, out);
            fprintf(out, "  mov %%rax, %%rcx\n  pop %%rax\n  sar %%cl, %%rax\n");
            break;
        case '=': /* greater-than-or-equal operator */
            cmp_expr(args, in, out, CMP_GE);
            break;
        default: /* greater-than operator */
            ungetc(c, in);
            cmp_expr(args, in, out, CMP_GT);
        }
        break;

    case '!': /* inequality operator */
        if((c = fgetc(in)) != '=') {
            eprintf(args->arg0, "unknown operator " QUOTE_FMT("!%c") "\n", c);
            exit(1);
        }
        cmp_expr(args, in, out, CMP_NE);
        break;
    
    case '=': /* equality operator */
        if((c = fgetc(in)) != '=') {
            eprintf(args->arg0, "unknown operator " QUOTE_FMT("=%c") "\n", c);
            exit(1);
        }
        cmp_expr(args, in, out, CMP_EQ);
        break;

    case '&': /* bitwise and operator */
        fprintf(out, "  push %%rax\n");
        expression(args, in, out);
        fprintf(out, "  pop %%rdi\n  and %%rdi, %%rax\n");
        break;
    
    case '|': /* bitwise or operator */
        fprintf(out, "  push %%rax\n");
        expression(args, in, out);
        fprintf(out, "  pop %%rdi\n  or %%rdi, %%rax\n");
        break;
    default:
        ungetc(c, in);
        return false;
    }

    return true;
}

static bool operator(struct compiler_args *args, FILE *in, FILE *out, bool left_is_lvalue)
{
    static long conditional = 0;
    char c, c1, c2;
    bool is_lvalue = false;
    int num_args = 0;
    
    whitespace(in);

    switch(c = fgetc(in)) {
    case '?': /* conditional operator */
        fprintf(out, "  cmp $0, %%rax\n  je .L.cond.else.%ld\n", conditional);
        expression(args, in, out);
        whitespace(in);
        if((c = fgetc(in)) != ':') {
            eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(":") " between conditional branches\n", c);
            exit(1);
        }
        fprintf(out, "  jmp .L.cond.end.%ld\n.L.cond.else.%ld:\n", conditional, conditional);
        expression(args, in, out);
        fprintf(out, ".L.cond.end.%ld:\n", conditional++);
        break;

    case '=': /* assignment operator */
        c1 = fgetc(in);
        c2 = fgetc(in);
        ungetc(c2, in);
        ungetc(c1, in);

        if(c1 == '=' && c2 != '=') /* check for equality operator `==` */
            goto bin_op;

        if(!left_is_lvalue) {
            eprintf(args->arg0, "left operand of assignment has to be an lvalue");
            exit(1);
        }

        fprintf(out, "  push %%rax\n");
        fprintf(out, "  mov (%%rax), %%rax\n");

        if(!bin_op(args, in, out, fgetc(in))) {
            whitespace(in);
            expression(args, in, out);
        }

        fprintf(out, "  pop %%rdi\n  mov %%rax, (%%rdi)\n");
        return false;

    case '[': /* index operator */
        fprintf(out, "  push %%rax\n");
        expression(args, in, out);
        fprintf(out, "  pop %%rdi\n  shl $3, %%rax\n  add %%rdi, %%rax\n");
    
        if((c = fgetc(in)) != ']') {
            eprintf(args->arg0, "unexpected token " QUOTE_FMT("%c") ", expect closing " QUOTE_FMT("]") " after index expression\n", c);
            exit(1);
        }
        is_lvalue = operator(args, in, out, true);
        break;
    
    case '(': /* function call */
        fprintf(out, "  push %%rax\n");

        while((c = fgetc(in)) != ')') {
            ungetc(c, in);
            expression(args, in, out);

            if(++num_args > MAX_FN_CALL_ARGS) {
                eprintf(args->arg0, "only %d call arguments are currently supported\n", MAX_FN_CALL_ARGS);
                exit(1);
            }
            fprintf(out, "  mov %%rax, %s\n", arg_registers[num_args - 1]);

            if((c = fgetc(in)) == ')')
                break;
            else if(c == ',')
                continue;
            
            eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect closing " QUOTE_FMT(")") " after call expression\n", c);
            exit(1);
        }

        fprintf(out, "  pop %%r10\n  call *%%r10\n");
        break;

    default:
    bin_op:
        if(left_is_lvalue)
            fprintf(out, "  mov (%%rax), %%rax\n");

        bin_op(args, in, out, c);
    }
    
    return is_lvalue;
}

static bool expression(struct compiler_args *args, FILE *in, FILE *out)
{
    static char buffer[BUFSIZ];
    char c;
    size_t i;
    long value;
    bool is_lvalue = false;
    bool dereference = false;

    whitespace(in);

    switch(c = fgetc(in)) {
    case '\'': /* character literal */
        if((value = character(args, in)))
            fprintf(out, "  mov $%lu, %%rax\n", value);
        else
            fprintf(out, "  xor %%rax, %%rax\n");
        break;
    
    case '(': /* parentheses */
        expression(args, in, out);
        ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after " QUOTE_FMT("(<expr>"));
        break;

    case '!': /* not operator */
        expression(args, in, out);
        fprintf(out, "  cmp $0, %%rax\n  sete %%al\n  movzx %%al, %%rax\n");
        break;
    
    case '-':
        if((c = fgetc(in)) == '-') { /* prefix decrement operator */
            if(!expression(args, in, out)) {
                eprintf(args->arg0, "expected lvalue after " QUOTE_FMT("--") "\n");
                exit(1);
            }
            fprintf(out, "  mov (%%rax), %%rdi\n  sub $1, %%rdi\n  mov %%rdi, (%%rax)");
            is_lvalue = true;
        }
        else { /* negation operator */
            ungetc(c, in);
            expression(args, in, out);
            fprintf(out, "  neg %%rax\n");
        }
        break;
    
    case '+': /* prefix increment operator */
        if((c = fgetc(in)) != '+') {
            eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT("+") "\n", c);
            exit(1);
        }
        if(!expression(args, in, out)) {
            eprintf(args->arg0, "expected lvalue after " QUOTE_FMT("++") "\n");
            exit(1);
        }
        fprintf(out, "  mov (%%rax), %%rdi\n  add $1, %%rdi\n  mov %%rdi, (%%rax)");

        is_lvalue = true;
        break;

    case '*': /* indirection operator */
        expression(args, in, out);
        dereference = true;
        is_lvalue = true;
        break;
    
    case '&': /* address operator */
        if(!expression(args, in, out)) {
            eprintf(args->arg0, "expected lvalue after " QUOTE_FMT("&") "\n");
            exit(1);
        }
        break;

    case EOF:
        eprintf(args->arg0, "unexpected end of file, expect expression");
        exit(1);
        
    default:
        if(isdigit(c)) { /* integer literal */
            ungetc(c, in);
            if((value = number(in)))
                fprintf(out, "  mov $%lu, %%rax\n", value);
            else
                fprintf(out, "  xor %%rax, %%rax\n");
        }
        else if(isalpha(c)) { /* identifier */
            is_lvalue = true;

            ungetc(c, in);
            identifier(in, buffer, BUFSIZ);

            for(i = 0; i < args->locals.size; i++) {
                if(strcmp(buffer, args->locals.data[i]) == 0) {
                    fprintf(out, "lea -%lu(%%rbp), %%rax\n", i * X86_64_WORD_SIZE);
                    goto operator;
                }
            }

            fprintf(out, "lea %s(%%rip), %%rax\n", buffer);
        }
        else {
            eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect expression\n", c);
            exit(1);
        }
    }

operator:
    return operator(args, in, out, is_lvalue);
}

static void statement(struct compiler_args *args, FILE *in, FILE *out, char* fn_ident, long switch_id, struct list *cases)
{
    char c;
    static char buffer[BUFSIZ];
    static long stmt_id = 0; /* unique id for each statement for generating labels */
    long id, value = 0;
    long i;
    struct list switch_case_list;
    char *name;

    whitespace(in);
    switch (c = fgetc(in)) {
    case '{':
        whitespace(in);
        while((c = fgetc(in)) != '}') {
            ungetc(c, in);
            statement(args, in, out, fn_ident, switch_id, cases);
            whitespace(in);
        }
        break;
    
    case ';':
        break; /* null statement */

    default:
        if(isalpha(c)) {
            ungetc(c, in);
            identifier(in, buffer, BUFSIZ);
            whitespace(in);
            
            if(strcmp(buffer, "goto") == 0) { /* goto statement */
                if(!identifier(in, buffer, BUFSIZ)) {
                    eprintf(args->arg0, "expect label name after " QUOTE_FMT("goto") "\n");
                    exit(1);
                }
                fprintf(out, "  jmp .L.label.%s\n", buffer);
                whitespace(in);
                ASSERT_CHAR(args, in, ';', "expect " QUOTE_FMT(";") " after " QUOTE_FMT("goto") " statement\n");
                return;
            }
            else if(strcmp(buffer, "return") == 0) { /* return statement */
                if((c = fgetc(in)) != ';') {
                    if(c != '(') {
                        eprintf(args->arg0, "expect " QUOTE_FMT("(") " or " QUOTE_FMT(";") " after " QUOTE_FMT("return") "\n");
                        exit(1);
                    }
                    expression(args, in, out);
                    whitespace(in);
                    ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after " QUOTE_FMT("return") " statement\n");
                    whitespace(in);
                    ASSERT_CHAR(args, in, ';', "expect " QUOTE_FMT(";") " after " QUOTE_FMT("return") " statement\n");
                }
                else
                    fprintf(out, "  xor %%rax, %%rax\n");
                fprintf(out, "  jmp .L.return.%s\n", fn_ident);
                return;
            }
            else if(strcmp(buffer, "if") == 0) { /* conditional statement */
                id = stmt_id++;

                ASSERT_CHAR(args, in, '(', "expect " QUOTE_FMT("(") " after " QUOTE_FMT("if") "\n");
                expression(args, in, out);
                fprintf(out, "  cmp $0, %%rax\n  je .L.else.%lu\n", id);
                whitespace(in);
                ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after condition\n");

                statement(args, in, out, fn_ident, -1, NULL);
                fprintf(out, "  jmp .L.end.%lu\n.L.else.%lu:\n", id, id);

                whitespace(in);
                memset(buffer, 0, 6 * sizeof(char));
                if((buffer[0] = fgetc(in)) == 'e' &&
                   (buffer[1] = fgetc(in)) == 'l' &&
                   (buffer[2] = fgetc(in)) == 's' &&
                   (buffer[3] = fgetc(in)) == 'e' && 
                   !isalnum((buffer[4] = fgetc(in)))) {
                    statement(args, in, out, fn_ident, -1, NULL);
                }
                else {
                    for(i = 4; i >= 0; i--) {
                        if(buffer[i])
                            ungetc(buffer[i], in);
                    }
                }

                fprintf(out, ".L.end.%lu:\n", id);
                return;
            }
            else if(strcmp(buffer, "while") == 0) { /* while statement */
                id = stmt_id++;

                ASSERT_CHAR(args, in, '(', "expect " QUOTE_FMT("(") " after " QUOTE_FMT("while") "\n");
                expression(args, in, out);
                fprintf(out, 
                    ".L.start.%lu:\n"
                    "  cmp $0, %%rax\n"
                    "  je .L.end.%lu\n",
                    id, id
                );
                whitespace(in);
                ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after condition\n");

                statement(args, in, out, fn_ident, -1, NULL);
                fprintf(out, "  jmp .L.start.%lu\n.L.end.%lu:\n", id, id);
                return;
            }
            else if(strcmp(buffer, "switch") == 0) { /* switch statement */
                id = stmt_id++;

                expression(args, in, out);
                fprintf(out, "  jmp .L.cmp.%ld\n.L.stmts.%ld:\n", id, id);

                memset(&switch_case_list, 0, sizeof(struct list));
                statement(args, in, out, fn_ident, id, &switch_case_list);
                fprintf(out, 
                    "  jmp .L.end.%ld\n"
                    ".L.cmp.%ld:\n", 
                    id, id
                );

                for(i = 0; i < (long) switch_case_list.size; i++)
                    fprintf(out, "  cmp $%ld, %%rax\n  je .L.case.%lu.%lu\n", (unsigned long) switch_case_list.data[i], id, (unsigned long) switch_case_list.data[i]);

                fprintf(out, ".L.end.%ld:\n", id);

                list_free(&switch_case_list); 
                return;
            }
            else if(strcmp(buffer, "case") == 0) { /* case statement */
                id = stmt_id++;

                if(switch_id < 0) {
                    eprintf(args->arg0, "unexpected " QUOTE_FMT("case") " outside of " QUOTE_FMT("switch") " statements\n");
                    exit(1);
                }

                switch (c = fgetc(in)) {
                case '\'':
                    value = character(args, in);
                    break;
                default:
                    if(isdigit(c)) {
                        ungetc(c, in);
                        value = number(in);
                        break;
                    }
                    
                    eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect constant after " QUOTE_FMT("case") "\n", c);
                    exit(1);
                }
                
                if(value == EOF) {
                    eprintf(args->arg0, "unexpected end of file, expect constant after " QUOTE_FMT("case") "\n");
                    exit(1);
                }
                whitespace(in);
                ASSERT_CHAR(args, in, ':', "expect " QUOTE_FMT(":") " after " QUOTE_FMT("case") "\n");
                list_push(cases, (void*) value);

                fprintf(out, ".L.case.%ld.%ld:\n", switch_id, value);
                statement(args, in, out, fn_ident, switch_id, cases);
                return;
            }
            else if(strcmp(buffer, "extrn") == 0) { /* external declaration */
                do {
                    if(!identifier(in, buffer, BUFSIZ)) {
                        eprintf(args->arg0, "expect identifier after " QUOTE_FMT("extrn") "\n");
                        exit(1);
                    }
                    whitespace(in);
                } while((c = fgetc(in)) == ',');

                if(c != ';') {
                    eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(";") " or " QUOTE_FMT(",") "\n", c);
                    exit(1);
                }
                return;
            }
            else if(strcmp(buffer, "auto") == 0) {
                do {
                    if(!identifier(in, buffer, BUFSIZ)) {
                        eprintf(args->arg0, "expect identifier after " QUOTE_FMT("auto") "\n");
                        exit(1);
                    }
                    whitespace(in);

                    if((c = fgetc(in)) == '\'') {
                        value = character(args, in);
                        whitespace(in);
                        c = fgetc(in);
                    }
                    else if(isdigit(c)) {
                        ungetc(c, in);
                        value = number(in);
                        whitespace(in);
                        c = fgetc(in);
                    }

                    name = calloc(strlen(buffer) + 1, sizeof(char));
                    strcpy(name, buffer);
                    list_push(&args->locals, name);

                    fprintf(out, "  sub $%lu, %%rsp\n  movl $%lu, -%lu(%%rbp)\n", X86_64_WORD_SIZE, value, (args->locals.size - 1) * X86_64_WORD_SIZE);
                } while((c) == ',');

                if(c != ';') {
                    eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(";") " or " QUOTE_FMT(",") "\n", c);
                    exit(1);
                }
                return;
            }
            else {
                switch(c = fgetc(in)) {
                case ':': /* label */
                    fprintf(out, ".L.label.%s:\n", buffer);
                    statement(args, in, out, fn_ident, switch_id, cases);
                    return;
                default:
                    ungetc(c, in);
                    for(i = strlen(buffer) - 1; i >= 0; i--)
                        ungetc(buffer[i], in);

                    expression(args, in, out);
                    whitespace(in);
                    if((c = fgetc(in)) != ';') {
                        eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") ", expect " QUOTE_FMT(";") " after expression statement\n", c);
                        exit(1);
                    }
                }
            }
        }
        else if(c == EOF) {
            eprintf(args->arg0, "unexpected end of file, expect statement\n");
            exit(1);
        }
        else {
            ungetc(c, in);
            expression(args, in, out);
            whitespace(in);
            if((c = fgetc(in)) != ';') {
                eprintf(args->arg0, "unexpected character " QUOTE_FMT("%c") " expect " QUOTE_FMT(";") " after expression statement\n", c);
                exit(1);
            }
        }
    }
}

static void function(struct compiler_args *args, FILE *in, FILE *out, char *identifier)
{
    size_t i;

    fprintf(out, ".text\n.type %s, @function\n%s:\n", identifier, identifier);

    ASSERT_CHAR(args, in, ')', "expect " QUOTE_FMT(")") " after function declaration\n");

    fprintf(out,
        "  push %%rbp\n"
        "  mov %%rsp, %%rbp\n"
    );

    for(i = 0; i < args->locals.size; i++)
        free(args->locals.data[i]);
    list_clear(&args->locals);

    statement(args, in, out, identifier, -1, NULL);
    
    fprintf(out, 
        ".L.return.%s:\n"
        "  mov %%rbp, %%rsp\n"
        "  pop %%rbp\n"
        "  ret\n",
        identifier
    );
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
