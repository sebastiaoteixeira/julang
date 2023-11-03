#include <stdio.h>
#include <stdlib.h>
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

    case RETURN:
        str = "RETURN";
        break;
    
    case IMPORT:
        str = "IMPORT";
        break;
        
    case CALL:
        str = "CALL";
        break;

    case PARAMETER:
        str = "PARAMETER";
        break;
    
    case ARGUMENT:
        str = "ARGUMENT";
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

    case EOF:
        str = "EOF";
        break;

    default:
        str = token.text;
        break;
    }
    printf("%s\n", str);
}

// A sequence of bits where 1 indicates a branch that already ended
struct {
    int length;
    long terminations;
} terminationsRecord;

void addToTerminationsRecord(int value) {
    if (value != 0 && value != 1) {
        printf("addToTerminationsRecord: value must be 0 or 1\n");
        exit(1);
    }
    if (terminationsRecord.length >= 64) {
        printf("AST deeper than 64 cannot be printed\n");
        exit(1);
    }
    terminationsRecord.length++;

    // shift the bits to the left by one
    // and add the new value to the end
    terminationsRecord.terminations <<= 1;
    terminationsRecord.terminations |= value;
}

void removeFromTerminationsRecord() {
    terminationsRecord.length--;

    // shift the bits to the right by one
    terminationsRecord.terminations >>= 1;
}

// Print abstract syntax tree
void printASTLevel(node *ast, int level) {
    if (ast == NULL) {
        return;
    }

    for (int i = 0; i < ast->length; i++) {
        for (int i = terminationsRecord.length - 1; i >= 0; i--) {
            if (terminationsRecord.terminations >> i & 1) {
                printf("        ");
            }
            else {
                printf("|       ");
            }
        }
        if (i == ast->length - 1) {
            addToTerminationsRecord(1);
            printf("\\-----> ");
        }
        else {
            addToTerminationsRecord(0);
            printf("|-----> ");
        }

        printToken(ast->children[i].data);
        for (int i = terminationsRecord.length - 1; i >= 0; i--) {
            if (terminationsRecord.terminations >> i & 1) {
                printf("        ");
            }
            else {
                printf("|       ");
            }
        } if (ast->children[i].length) printf("|");
        printf("\n");
        printASTLevel(&(ast->children[i]), level + 1);
    }

    removeFromTerminationsRecord();
}

void printAST(node *ast) {
    terminationsRecord.terminations = 0;
    terminationsRecord.length = 0;
    printASTLevel(ast, 0);
}
