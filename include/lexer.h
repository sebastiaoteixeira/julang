#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <stdio.h>
#include "token.h"

int isLetter(char c);
int isDigit(char c);

Token* runLexer(FILE* iCode);

Token nextToken(FILE *iCode);
void printTokenList(Token *tokenList);

#endif // LEXER_H_INCLUDED
