#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "codegen.h"
#include "symbolsNaming.h"
#include "token.h"

static const verbose = 4;

FILE *readFile(char *fileName)
{
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    return fp;
}

int main(int argc, char **argv)
{
    if (argc < 2) return 1;

    FILE *inputCode = readFile(argv[1]);

    Token *tokenList = runLexer(inputCode);
    char* moduleHash = generateModuleHash(tokenList);
    printf("%s\n", moduleHash);

    if (verbose >= 4)
        printTokenList(tokenList);

    node ast = runParser(tokenList);
    printf("Print Abstract Syntax Tree:\n");
    printAST(&ast);

    printf("\n\nCalling generator.... \n");
    generateCode(argv[1], ast);

    printf("Terminated\n");
    return 0;
}
