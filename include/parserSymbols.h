#ifndef PARSER_SYMBOLS_H
#define PARSER_SYMBOLS_H

#include "parser.h"

typedef struct {
    char *name;
    void *data;
    int level;
    short type;
} Symbol;

typedef struct {
    Symbol *table;
    int level;
    int size;
    int capacity;
} SymbolStack;

SymbolStack *createSymbolStack();
void destroySymbolStack(SymbolStack *stack);
void pushSymbol(SymbolStack *stack, Symbol symbol);
Symbol popSymbol(SymbolStack *stack);
Symbol *findSymbol(SymbolStack *stack, char *name);
void increaseLevel(SymbolStack *stack);
void decreaseLevel(SymbolStack *stack);


// Data structures
// Primitives

void pushPrimitive(SymbolStack *stack, char *name, short type);

// Functions
typedef struct {
    short ret;
    Symbol *params;
    unsigned long paramsLength;
    unsigned long compulsoryArgsLength;
    int variadic;
} FunctionData;

void _pushFunction(SymbolStack *stack, char *name, short ret, Symbol *params, unsigned long paramsLength, unsigned long compulsoryArgsLength, int variadic);
void pushFunction(SymbolStack *stack, node *function);
void verifyFunction(SymbolStack *stack, char *name, node *args, node *reordenedArgs, unsigned long argsLength);

// Static arrays
typedef struct {
    short type;
    unsigned long *size;
    unsigned long nDims;
} StaticArrayData;

void pushStaticArray(SymbolStack *stack, char *name, short type, unsigned long* dims, int nDims);

// Dynamic arrays
typedef struct {
    short type;
} DynamicArrayData;

void pushDynamicArray(SymbolStack *stack, char *name, short type);

// Objects
typedef struct {
    Symbol *fields;
    unsigned long fieldsLength;
} ObjectData;

void pushObject(SymbolStack *stack, char *name, Symbol *fields, unsigned long fieldsLength);

#endif // PARSER_SYMBOLS_H