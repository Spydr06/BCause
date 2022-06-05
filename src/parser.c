#include "ast.h"
#include <parser.h>
#include <lexer.h>
#include <stdlib.h>
#include <list.h>

static inline bool is(Lexer_T* l, TokenKind_T kind) {
    return l->token.kind == kind;
}

static inline void advance(Lexer_T* l) {
    lexer_get_token(l);
}

static void consume(Lexer_T* l, TokenKind_T kind)
{
    if(!is(l, kind)) {
        eprintf("Expect token of kind `%s`, got `%s`.\n", 
            token_kind_to_str(kind),
            token_kind_to_str(l->token.kind)
        );
        exit(EXIT_FAILURE);
    }
    advance(l);
}

static Node_T* block(Lexer_T* l) 
{
    Node_T* node = calloc(1, sizeof(Node_T));
    node->kind = NODE_BLOCK;

    consume(l, TOKEN_LBRACE);
    consume(l, TOKEN_RBRACE);

    return node;
}

void parse(Lexer_T* l, Node_T* ast)
{
    init_list(&ast->root.objs);

    lexer_get_token(l);
    while(l->token.kind != TOKEN_EOF) {
        char* name = l->token.value;
        consume(l, TOKEN_ID);
        
        Node_T* node = calloc(1, sizeof(Node_T));

        if(is(l, TOKEN_LPAREN)) {
            advance(l);
            node->kind = NODE_FUNC;
            node->func.name = name;
            init_list(&node->func.args);

            while(!is(l, TOKEN_RPAREN) && !is(l, TOKEN_EOF)) {
                Node_T* arg = calloc(1, sizeof(Node_T));
                arg->kind = NODE_VAR;
                arg->var.name = l->token.value;
                list_push(&node->func.args, arg);
                consume(l, TOKEN_ID);
                if(!is(l, TOKEN_RPAREN))
                    consume(l, TOKEN_COMMA);
            }
            consume(l, TOKEN_RPAREN);

            node->func.body = block(l);
        }
        else {
            node->kind = NODE_VAR;
            node->var.name = name;    
            consume(l, TOKEN_SEMICOLON);
        }

        list_push(&ast->root.objs, node);
    }
}