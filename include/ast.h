#pragma once
#include <list.h>

typedef enum {
    NODE_ROOT,
    NODE_VAR,
    NODE_FUNC,

    NODE_BLOCK,
} NodeKind_T;

typedef struct AST_NODE_STRUCT Node_T;

struct AST_NODE_STRUCT {
    NodeKind_T kind;

    union {
        struct {
            List_T objs;
        } root;
        struct {
            char* name;
            Node_T* value;
        } var;
        struct {
            char* name;
            List_T args;
            Node_T* body;
        } func;
    };
};