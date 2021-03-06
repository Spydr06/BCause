#pragma once

#include <io.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_CHAR,

    TOKEN_EXTRN,
    TOKEN_AUTO,
    TOKEN_IF,
    TOKEN_WHILE,
    TOKEN_SWITCH,
    TOKEN_GOTO,
    TOKEN_CASE,

    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACKET,  // [
    TOKEN_RBRACKET,  // ]
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_SEMICOLON, // ;
    TOKEN_COMMA,     // ,
    TOKEN_EQUALS,    // =
    TOKEN_SLASH,     // /
    TOKEN_ESLASH,    // =/
    TOKEN_PLUS,      // +
    TOKEN_EPLUS,     // =+
    TOKEN_MINUS,     // -
    TOKEN_EMINUS,    // =-
    TOKEN_STAR,      // *
    TOKEN_ESTAR,     // =*
    TOKEN_PERCENT,   // %
    TOKEN_EPERCENT,  // =%

    TOKEN_LAST
} TokenKind_T;

typedef struct {
    TokenKind_T kind;
    char* value;
} Token_T;

const char* token_kind_to_str(TokenKind_T kind);
const char* token_to_str(Token_T* token);

typedef struct {
    const char* filename;
    char* buffer;
    size_t pos;
    size_t size;
    char c;
    Token_T token;
} Lexer_T;

void init_lexer(Lexer_T* l, const char* filename);
void free_lexer(Lexer_T* l);
Token_T lexer_get_token(Lexer_T* l);