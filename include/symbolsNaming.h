#ifndef SYMBOLSNAMING_H_INCLUDED
#define SYMBOLSNAMING_H_INCLUDED
#include "token.h"

char* generateSymbolHash(char* input, char* encoded);
char* generateModuleHash(Token* input);
char* generateParametersHash(short* input, unsigned int length);

#endif // SYMBOLSNAMING_H_INCLUDED
