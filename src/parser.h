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

void addChild(node parent, node child);


node runParser(Token *tokens);

#endif
