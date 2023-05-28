#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "token.h"

struct {
    Token *tokens;
    int index;
} tokenListManager;

// Declare AST node
typedef struct node {
    Token data;
    struct node *children;
    int length;
} node;

void addChild(node parent, node child);


node runParser(Token *tokens);

#endif
