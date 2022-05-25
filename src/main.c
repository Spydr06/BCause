#include <io.h>
#include <lexer.h>
#include <stdlib.h>
#include <string.h>

static const char HELP_TEXT[] = "";

static int compile(char* filename) {
    Lexer_T lexer;
    init_lexer(&lexer, filename);

    Token_T tok;
    while((tok = lexer_get_token(&lexer)).kind != TOKEN_EOF) {
        printf("%d %s\n", tok.kind, tok.value);
    }

    free_lexer(&lexer);
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

