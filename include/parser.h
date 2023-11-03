#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "token.h"

typedef struct TLMs {
    Token *tokens;
    int index;
} TLM;

// Declare AST node
typedef struct Node {
    Token data;
    struct Node *children;
    int length;
} node;

node* addChild(node* parent);

void parseExpression(node* parent);

#include "parserSymbols.h"

node runParser(SymbolStack *symbolStack, Token *tokens);

#endif
