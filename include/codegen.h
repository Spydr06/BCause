#pragma once

#include "ast.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    FILE* out;
    size_t address;

    size_t file_size_addr;
    size_t code_start_pos;
} Generator_T;

void generate(Generator_T* g, Node_T* ast, const char* destination);