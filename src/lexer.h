#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "token.h"

int isLetter(char c);
int isDigit(char c);

unsigned int linecount = 1;

Token* runLexer(FILE* iCode);

Token nextToken(FILE *iCode);

#endif // LEXER_H_INCLUDED
