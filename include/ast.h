#pragma once

typedef enum {
    NODE_FN,
    NODE_VAR,

    NODE_BLOCK,
    NODE_IF,
    NODE_WHILE,
    
    NODE_ASSING,
    NODE_ADD,
    NODE_SUB,
    NODE_MULT,
    NODE_DIV,
    NODE_MOD,
    NODE_CALL
} NodeKind_T;

typedef struct AST_NODE_STRUCT Node_T;

struct AST_NODE_STRUCT {
    NodeKind_T kind;

    union {
        struct {
            Node_T* body;
            Node_T** args;
            Node_T** vars;
        };
        struct {
            Node_T* left;
            Node_T* right;
        };
    };
};