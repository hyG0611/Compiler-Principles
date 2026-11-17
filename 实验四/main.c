#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

extern int yyparse(void);
extern past ast_root;
extern FILE *yyin;

int main(int argc, char **argv)
{
    static char result[IR_CAPACITY];
    FILE *input = stdin;

    int module_output = argc > 1 && strcmp(argv[1], "--llvm") == 0;
    int source_index = module_output ? 2 : 1;

    if (argc > source_index + 1) {
        fprintf(stderr, "Usage: %s [--llvm] [source.c]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == source_index + 1) {
        input = fopen(argv[source_index], "r");
        if (input == NULL) {
            perror(argv[source_index]);
            return EXIT_FAILURE;
        }
    }

    /* Match the sample's test.c convention when no stdin was supplied. */
    if (argc == source_index) {
        int first = fgetc(stdin);
        if (first == EOF) {
            input = fopen("test.c", "r");
            if (input == NULL) {
                perror("test.c");
                return EXIT_FAILURE;
            }
        } else {
            ungetc(first, stdin);
        }
    }

    yyin = input;
    int status = yyparse();
    if (input != stdin) fclose(input);
    if (status != 0) return EXIT_FAILURE;

    if (module_output) genProgram(ast_root, result);
    else genSampleProgram(ast_root, result);
    return fputs(result, stdout) == EOF ? EXIT_FAILURE : EXIT_SUCCESS;
}

