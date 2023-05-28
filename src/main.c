#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

FILE *readFile(char *fileName)
{
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return fp;
    }

    return fp;
}

int main(int argc, char **argv)
{
    if (argc < 2) return 1;

    FILE *inputCode = readFile(argv[1]);

    Token *tokenList = runLexer(inputCode);

    printTokenList(tokenList);
    return 0;
}