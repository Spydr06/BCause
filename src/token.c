#include <lexer.h>

static const char* TOKEN_KIND_STRS[TOKEN_LAST] = {
    [TOKEN_EOF] = "end of file",
    [TOKEN_ID] = "identifier",
    [TOKEN_NUMBER] = "number",
    [TOKEN_CHAR] = "char literal",
    [TOKEN_EXTRN] = "extrn",
    [TOKEN_AUTO] = "auto",
    [TOKEN_IF] = "if",
    [TOKEN_WHILE] = "while",
    [TOKEN_SWITCH] = "switch",
    [TOKEN_CASE] = "case",
    [TOKEN_GOTO] = "goto",
    [TOKEN_LPAREN] = "(",
    [TOKEN_RPAREN] = ")",
    [TOKEN_LBRACKET] = "[",
    [TOKEN_RBRACKET] = "]",
    [TOKEN_LBRACE] = "{",
    [TOKEN_RBRACE] = "}",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_COMMA] = ",",
    [TOKEN_EQUALS] = "=",
    [TOKEN_SLASH] = "/",
    [TOKEN_ESLASH] = "=/",
    [TOKEN_PLUS] = "+",
    [TOKEN_EPLUS] = "=+",
    [TOKEN_MINUS] = "-",
    [TOKEN_EMINUS] = "=-",
    [TOKEN_STAR] = "*",
    [TOKEN_ESTAR] = "=*",
    [TOKEN_PERCENT] = "%",
    [TOKEN_EPERCENT] = "=%"
};

const char* token_kind_to_str(TokenKind_T kind) {
    return TOKEN_KIND_STRS[kind];
}

const char* token_to_str(Token_T* token) {
    switch(token->kind) {
        case TOKEN_ID:
        case TOKEN_CHAR:
        case TOKEN_NUMBER:
            return token->value;
        
        default:
            return token_kind_to_str(token->kind);
    }
}