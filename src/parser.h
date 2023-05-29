#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "token.h"

typedef struct TLMs {
    Token *tokens;
    int index;
} TLM;

// Declare AST node
typedef struct node {
    Token data;
    struct node *children;
    int length;
} node;

void addChild(node parent, node child);


node runParser(Token *tokens);

#endif
