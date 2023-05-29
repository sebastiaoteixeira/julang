#ifndef EXPRESSIONPARSER_H_DEFINED
#define EXPRESSIONPARSER_H_DEFINED
#include "parser.h"
#include "token.h"

typedef struct Operator {
    char* op;
    int precedence;
} Operator;

typedef struct ExpressionToken {
    Token token;
    int literal;
    int unary;
    int precedence;
} ExpressionToken;


void initExpressionParser(TLM* tokenListManagerRef);
int isOperator(char *c);
int getOperatorPrecedence(char *op);
node getExpressionAST();

#endif
