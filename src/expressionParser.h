#ifndef EXPRESSIONPARSER_H_DEFINED
#define EXPRESSIONPARSER_H_DEFINED
#include "parser.h"

struct {
    char* op;
    int precedence;
} *Operators;

char *PRIORITY_LEVELS[] = {"||\\",
                          "&&\\",
                          "|\\",
                          "^\\",
                          "&\\",
                          "==\\!=\\",
                          "<\\>\\<=\\>=\\",
                          "<<\\>>\\",
                          "+\\-\\",
                          "*\\/\\%\\"};

int initExpressionParser();
int isOperator(char *c);
int getOperatorPrecedence(char *op);
node getExpressionAST();

#endif