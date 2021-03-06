#include <io.h>
#include <lexer.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#define TOKEN1(l, _kind) do { \
    l->token.kind = _kind;           \
} while(0)

#define TOKEN2(l, _kind) do { \
    lexer_next(l);                 \
    l->token.kind = _kind;              \
} while(0)

static const char* KEYWORDS[TOKEN_LAST] = {
    [TOKEN_AUTO] = "auto",
    [TOKEN_EXTRN] = "extrn",
    [TOKEN_IF] = "if",
    [TOKEN_WHILE] = "while",
};

void init_lexer(Lexer_T* l, const char* filename)
{
    memset(l, 0, sizeof(Lexer_T));
    l->filename = filename;

    if(access(filename, F_OK)) {
        eprintf("Cannot access file `%s`, no such file or directory.\n", filename);
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen(filename, "r");
    if(!file) {
        eprintf("Error opening file `%s`\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    l->size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(!(l->buffer = calloc(l->size + 1, sizeof(char)))) {
        eprintf("Error allocating `%lu` bytes.\n", (l->size + 1) * sizeof(char));
        exit(EXIT_FAILURE);
    }

    if((fread(l->buffer, sizeof(char), l->size, file)) == -1) {
        eprintf("Error reading file `%s`.\n", filename);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void free_lexer(Lexer_T* l) 
{
    free(l->buffer);
}

static char lexer_next(Lexer_T* l) 
{
    return l->pos < l->size ? (l->c = l->buffer[l->pos++]) : (l->c = '\0');
}

static char lexer_peek(Lexer_T* l, size_t i) 
{
    return l->pos + --i < l->size ? l->buffer[l->pos + i] : '\0';
}

static void lexer_skip_whitespace(Lexer_T* l)
{
    while(lexer_next(l) == '\t' || l->c == ' ' || l->c == '\r' || l->c == '\n' || (l->c == '/' && lexer_peek(l, 1) == '*')) {
        if(l->c == '/' && lexer_peek(l, 1) == '*') {
            lexer_next(l);
            while(lexer_next(l) != '*' && lexer_peek(l, 1) != '/' && l->c != '\0');
            lexer_next(l);
            lexer_next(l);
        }
    }
}

static void lexer_get_id(Lexer_T* l) 
{
    while(isalnum(lexer_next(l)));
    l->pos -= 1;
}

static void lexer_get_num(Lexer_T* l)
{
    while(isdigit(lexer_next(l)));
    l->pos -= 1;
}

static void lexer_get_char(Lexer_T* l)
{
    while(lexer_next(l) != '\'') {
        if(l->c == '\0' || l->c == '\n') {
            eprintf("Unclosed char literal (%lu).\n", l->pos);
            exit(EXIT_FAILURE);
        }
    }
}

Token_T lexer_get_token(Lexer_T* l)
{
    lexer_skip_whitespace(l);

    l->token = (Token_T){.value = ""};
    size_t start = l->pos - 1;

    if(isalnum(l->c)) {
        bool is_num = false;
        if(isalpha(l->c))
            lexer_get_id(l);
        else {
            is_num = true;
            lexer_get_num(l);
        }
        size_t len = l->pos - start + (l->pos >= l->size - 1);

        l->token.kind = is_num ? TOKEN_NUMBER : TOKEN_ID;
        l->token.value = calloc(len + 1, sizeof(char));
        strncpy(l->token.value, &l->buffer[start], len);

        for(size_t i = 0; i < TOKEN_LAST; i++) {
            if(KEYWORDS[i] && strcmp(l->token.value, KEYWORDS[i]) == 0)
            {
                l->token.kind = i;
                break;
            }
        }
    }
    else
        switch(l->c) {
        case '(':
            TOKEN1(l, TOKEN_LPAREN);
            break;
        case ')':
            TOKEN1(l, TOKEN_RPAREN);
            break;
        case '[':
            TOKEN1(l, TOKEN_LBRACKET);
            break;
        case ']':
            TOKEN1(l, TOKEN_RBRACKET);
            break;
        case '{':
            TOKEN1(l, TOKEN_LBRACE);
            break;
        case '}':
            TOKEN1(l, TOKEN_RBRACE);
            break;
        case ';':
            TOKEN1(l, TOKEN_SEMICOLON);
            break;
        case ',':
            TOKEN1(l, TOKEN_COMMA);
            break;
        case '+':
            TOKEN1(l, TOKEN_PLUS);
            break;
        case '-':
            TOKEN1(l, TOKEN_MINUS);
            break;
        case '*':
            TOKEN1(l, TOKEN_STAR);
            break;
        case '/':
            TOKEN1(l, TOKEN_SLASH);
            break;
        case '%':
            TOKEN1(l, TOKEN_PERCENT);
            break;
        case '=': {
            char next = lexer_peek(l, 1);
            switch(next) {
            case '+':
                TOKEN2(l, TOKEN_EPLUS);
                break;
            case '-':
                TOKEN2(l, TOKEN_EMINUS);
                break;
            case '*':
                TOKEN2(l, TOKEN_ESTAR);
                break;
            case '/':
                TOKEN2(l, TOKEN_ESLASH);
                break;
            case '%':
                TOKEN2(l, TOKEN_EPERCENT);
                break;
            default:
                TOKEN1(l, TOKEN_EQUALS);        
            }
        } break;

        case '\'': {
            lexer_get_char(l);
            size_t len = l->pos - start - 1;
            l->token.kind = TOKEN_CHAR;   
            l->token.value = calloc(len, sizeof(char));
            snprintf(l->token.value, len, "%s", &l->buffer[start + 1]); 
        } break;
        
        case '\0':
            TOKEN1(l, TOKEN_EOF);
            break;

        default:
            eprintf("Unknown token at `%lu`\n", start);
            exit(EXIT_FAILURE);
        }

    return l->token;
}