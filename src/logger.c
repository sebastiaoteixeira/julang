#include <stdio.h>
#include "logger.h"
#include "parser.h"

void printToken(Token token) {
    char *str;
    switch (token.type)
    {
    case EXPRESSION:
        str = "EXPRESSION";
        break;
    case DECLARATION:
        str = "DECLARATION";
        break;

    case FUNCTION:
        str = "FUNCTION";
        break;

    case ARGUMENT:
        str = "ARGUMENT";
        break;

    case PARAMETER:
        str = "PARAMETER";
        break;

    case BLOCK:
        str = "BLOCK";
        break;

    case IFSTATEMENT:
        str = "IFSTATEMENT";
        break;

    case WHILELOOPSTATEMENT:
        str = "WHILELOOPSTATEMENT";
        break;

    case DOWHILELOOPSTATEMENT:
        str = "DOWHILELOOPSTATEMENT";
        break;

    case FORLOOPSTATEMENT:
        str = "FORLOOPSTATEMENT";
        break;

    default:
        str = token.text;
        break;
    }
    printf("%s\n", str);
}

// Print abstract syntax tree
void printASTLevel(node *ast, int level) {
    if (ast == NULL) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("|   ");
    }

    for (int i = 0; i < ast->length; i++) {
        if (i == ast->length - 1) printf("|---");
        else printf("|---");
        printToken(ast->children[i].data);
        printASTLevel(&(ast->children[i]), level + 1);
    }
}

void printAST(node *ast) {
    printASTLevel(ast, 0);
}
