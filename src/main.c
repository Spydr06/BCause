#include "ast.h"
#include "codegen.h"
#include <io.h>
#include <parser.h>
#include <stdlib.h>
#include <string.h>

static const char HELP_TEXT[] = "";

static int compile(char* filename) {
    Lexer_T lexer;
    init_lexer(&lexer, filename);

    Node_T ast;
    memset(&ast, 0, sizeof(Node_T));
    parse(&lexer, &ast);
    
    printf("%ld objs\n", ast.root.objs.size);

    free_lexer(&lexer);

    Generator_T generator;
    memset(&generator, 0, sizeof(Generator_T));
    generate(&generator, &ast, "a.out");
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    if(argc == 1) {
        eprintf("Too few arguments given.\n");
        return EXIT_FAILURE;
    }

    char* filename = NULL;

    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "--help") == 0) {
            println(HELP_TEXT);
            return EXIT_SUCCESS;
        }
        else
            filename = argv[i];
    }

    if(!filename) {
        eprintf("No input file given.\n");
        return EXIT_FAILURE;
    }

    return compile(filename);
}

