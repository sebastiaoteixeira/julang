#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "parser.h"
#include "parserSymbols.h"

SymbolStack* createSymbolStack() {
    SymbolStack* stack = (SymbolStack *) malloc(sizeof(SymbolStack));
    *stack = (SymbolStack) {
        .table = (Symbol *) malloc(sizeof(Symbol) * 16),
        .level = 0,
        .size = 0,
        .capacity = 16
    };
    
    return stack;
}

void destroySymbolStack(SymbolStack *stack) {
    for (int i = 0; i < stack->size; i++) {
        if (stack->table[i].data != NULL)
            if (stack->table[i].type == FUNCTION && ( (FunctionData *) stack->table[i].data)->params != NULL)
                free(((FunctionData *) stack->table[i].data)->params);
            else if (stack->table[i].type == ARRAY && ( (StaticArrayData *) stack->table[i].data)->size != NULL)
                free(((StaticArrayData *) stack->table[i].data)->size);
            else if (stack->table[i].type == OBJECT && ( (ObjectData *) stack->table[i].data)->fields != NULL)
                free(((ObjectData *) stack->table[i].data)->fields);
            free(stack->table[i].data);
    }
    free(stack->table);
    free(stack);
}

void pushSymbol(SymbolStack *stack, Symbol symbol) {
    if (stack->size == stack->capacity) {
        stack->capacity *= 2;
        stack->table = (Symbol *) realloc(stack->table, sizeof(Symbol) * stack->capacity);
    }
    stack->table[stack->size] = symbol;
    stack->size++;
}

Symbol popSymbol(SymbolStack *stack) {
    if (stack->size == 0) {
        printf("Error: Symbol stack is empty\n");
        exit(1);
    }
    stack->size--;
    return stack->table[stack->size];
}

Symbol *findSymbol(SymbolStack *stack, char *name) {
    for (int i = stack->size - 1; i >= 0; i--) {
        if (strcmp(stack->table[i].name, name) == 0) {
            return &(stack->table[i]);
        }
    }
    return NULL;
}

void increaseLevel(SymbolStack *stack) {
    stack->level++;
}

void decreaseLevel(SymbolStack *stack) {
    stack->level--;
    while (stack->size > 0 && stack->table[stack->size - 1].level > stack->level) {
        popSymbol(stack);
    }
}

void pushPrimitive(SymbolStack *stack, char *name, short type) {
    Symbol symbol = {
        .name = name,
        .data = NULL,
        .level = stack->level,
        .type = type
    };
    pushSymbol(stack, symbol);
}

void _pushFunction(SymbolStack *stack, char *name, short ret, Symbol *params, unsigned long paramsLength, unsigned long compulsoryArgsLength, int variadic) {
    Symbol symbol = {
        .name = name,
        .data = (FunctionData *) malloc(sizeof(FunctionData)),
        .level = stack->level,
        .type = FUNCTION
    };
    ((FunctionData *) symbol.data)->ret = ret;
    ((FunctionData *) symbol.data)->params = params;
    ((FunctionData *) symbol.data)->paramsLength = paramsLength;
    ((FunctionData *) symbol.data)->compulsoryArgsLength = compulsoryArgsLength;
    ((FunctionData *) symbol.data)->variadic = variadic;
    pushSymbol(stack, symbol);
}

void pushFunction(SymbolStack *stack, node *function) {
    // Count the number of parameters
    unsigned long paramsLength = 0;
    int compulsory = 1;
    int compulsoryLength = 0;
    for (int i = 2; i < function->length; i++) {
        if (function->children[i].data.type == DECLARATION) {
            paramsLength++;
            if (compulsory) compulsoryLength++;
        }
        if (compulsory && function->children[i].data.type == EXPRESSION) {
            compulsory = 0;
            compulsoryLength--;
        }
    }

    // Create a list of parameters
    Symbol *params = (Symbol *) malloc(sizeof(Symbol) * paramsLength);

    // Add the parameters to the list
    int j = 0;
    for (int i = 2; i < function->length; i++) {
        if (function->children[i].data.type == DECLARATION) {
            params[j] = (Symbol) {
                .name = function->children[i].children[1].data.text,
                .data = NULL,
                .level = stack->level,
                .type = function->children[i].children[0].data.type
            };
            j++;
        }
    }

    _pushFunction(stack, function->children[1].data.text, function->children[0].data.type, params, paramsLength, compulsoryLength, 0);
}

void pushStaticArray(SymbolStack *stack, char *name, short type, unsigned long* dims, int nDims) {
    Symbol symbol = {
        .name = name,
        .data = (StaticArrayData *) malloc(sizeof(StaticArrayData)),
        .level = stack->level,
        .type = ARRAY
    };
    ((StaticArrayData *) symbol.data)->type = type;
    ((StaticArrayData *) symbol.data)->size = dims;
    ((StaticArrayData *) symbol.data)->nDims = nDims;
    pushSymbol(stack, symbol);
}

void pushDynamicArray(SymbolStack *stack, char *name, short type) {
    Symbol symbol = {
        .name = name,
        .data = (DynamicArrayData *) malloc(sizeof(DynamicArrayData)),
        .level = stack->level,
        .type = DARRAY
    };
    ((DynamicArrayData *) symbol.data)->type = type;
    pushSymbol(stack, symbol);
}

void pushObject(SymbolStack *stack, char *name, Symbol *fields, unsigned long fieldsLength) {
    Symbol symbol = {
        .name = name,
        .data = (ObjectData *) malloc(sizeof(ObjectData)),
        .level = stack->level,
        .type = OBJECT
    };
    ((ObjectData *) symbol.data)->fields = fields;
    ((ObjectData *) symbol.data)->fieldsLength = fieldsLength;
    pushSymbol(stack, symbol);
}

void verifyFunction(SymbolStack *stack, char *name, node *args, node *reordenedArgs, unsigned long argsLength) {
    for (int i = stack->size - 1; i >= 0; i--) {
        // Search for a function with the same name
        if (stack->table[i].type == FUNCTION && strcmp(stack->table[i].name, name) == 0) {
            // Create a list of remaining parameters
            FunctionData *function = (FunctionData *) stack->table[i].data;
            Symbol *params = (Symbol *) malloc(sizeof(Symbol) * function->paramsLength);
            memcpy(params, function->params, sizeof(Symbol) * function->paramsLength);
            Symbol *remainingParams = params;
            int remainingParamsLength = function->paramsLength;
            int ordened = 1;
            // Add the arguments to reordenedArgs
            for (int j = 0; j < argsLength; j++) {
                // Verify if is an unordered argument
                if (ordened && args[j].length == 2) ordened = 0;

                if (ordened) {
                    // While the arguments are ordered, add them to reordenedArgs ...
                    reordenedArgs[j] = args[j];
                    // ... and remove them from remainingParams
                    remainingParams++;
                    remainingParamsLength--;
                } else {
                    // Ordened arguments are not allowed after unordered arguments
                    if (args[j].length == 1) {
                        printf("Error: Unordered arguments must be placed after ordered arguments\n");
                        exit(1);
                    }
                    // Search for the parameter with the same name
                    for (int k = 0; k < remainingParamsLength; k++) {
                        if (strcmp(remainingParams[k].name, args[j].children[0].data.text) == 0) {
                            // Add the argument to reordenedArgs ...
                            reordenedArgs[k] = args[j];
                            // ... and remove it from remainingParams
                            remainingParams[k] = remainingParams[remainingParamsLength - 1];
                            remainingParamsLength--;
                            break;
                        }
                        // If the parameter is not found, it is an error
                        if (k == remainingParamsLength - 1) {
                            printf("Error: Function '%s' does not have a parameter named '%s'\n", name, args[j].children[0].data.text);
                            exit(1);
                        }
                    }
                }
            }

            // Verify if the first remaining parameter is not a compulsory argument
            // If it is, so this is not the function we are looking for
            int compulsoryArgsMissing = 0;
            if (remainingParamsLength > 0) {
                for (int i = 0; i < function->compulsoryArgsLength; i++) {
                    if (strcmp(remainingParams[0].name, function->params[i].name) == 0) {
                        compulsoryArgsMissing = 1;
                    }
                }
            }
            
            free(params);
            
            if (compulsoryArgsMissing) {
                continue;
            }

            return;
        }
    }
    printf("Error: Function '%s' is not defined\n", name);
    exit(1);
    return;
}