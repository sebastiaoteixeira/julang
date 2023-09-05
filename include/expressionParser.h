#ifndef EXPRESSIONPARSER_H_DEFINED
#define EXPRESSIONPARSER_H_DEFINED
#include "parser.h"
#include "parserSymbols.h"
#include "token.h"

typedef struct {
    int op;
    int precedence;
} Operator;

typedef struct ExpressionToken {
    Token token;
    int literal;
    int unary;
    int precedence;
} ExpressionToken;


static const int OPERATORS_COUNT = 19;
static const short PRIORITY_LEVELS[11][4] = {{ASSIGN, 0, 0, 0},
                                     {OR, 0, 0, 0},
                                     {AND, 0, 0, 0},
                                     {BOR, 0, 0, 0},
                                     {BXOR, 0, 0, 0},
                                     {BAND, 0, 0, 0},
                                     {EQ, NEQ, 0, 0},
                                     {GT, LT, GTE, LTE},
                                     {BSL, BSR, 0, 0},
                                     {PLUS, MINUS, 0, 0},
                                     {MULT, DIV, MOD, 0}};


static Operator* operators;
static TLM* tokenListManagerRef;


void initExpressionParser(TLM* tokenListManagerRef, SymbolStack* symbolStack);
int isOperator(Token c);
int getOperatorPrecedence(Token op);
node getExpressionAST();

#endif
