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

void iterateOverAST(node ast, void (*callback)(node), int depth);
void parseExpression(node* parent);
void iterateOverAST(node ast, void (*callback)(node), int depth);

node runParser(Token *tokens);

#endif
