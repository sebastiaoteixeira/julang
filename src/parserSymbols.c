#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "token.h"
#include "parser.h"
#include "parserSymbols.h"
#include "module.h"
#include "symbolsNaming.h"

#define PATH_MAX 1024

char *currentModuleHash;
char *rootPath;

void setCurrentModuleHash(char *hash) {
    currentModuleHash = hash;
}

char *getCurrentModuleHash() {
    return currentModuleHash;
}

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

Symbol *findSymbol(SymbolStack *stack, char *name, char *moduleHash) {
    if (moduleHash == NULL) moduleHash = getCurrentModuleHash();
    for (int i = stack->size - 1; i >= 0; i--) {
        if (strcmp(stack->table[i].name, name) == 0 && strcmp(stack->table[i].moduleHash, moduleHash) == 0) {
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

void pushPrimitive(SymbolStack *stack, char *name, char *moduleHash, short type) {
    Symbol symbol = {
        .name = name,
        .data = NULL,
        .level = stack->level,
        .type = type
    };
    pushSymbol(stack, symbol);
}

void _pushFunction(SymbolStack *stack, char *name, char *moduleHash, short ret, Symbol *params, unsigned long paramsLength, unsigned long compulsoryArgsLength, int variadic) {
    Symbol symbol = {
        .name = name,
        .moduleHash = malloc(strlen(getCurrentModuleHash()) + 1),
        .data = (FunctionData *) malloc(sizeof(FunctionData)),
        .level = stack->level,
        .type = FUNCTION
    };
    strcpy(symbol.moduleHash, getCurrentModuleHash());
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

    _pushFunction(stack, function->children[1].data.text, getCurrentModuleHash(), function->children[0].data.type, params, paramsLength, compulsoryLength, 0);
}

void pushStaticArray(SymbolStack *stack, char *name, char *moduleHash, short type, unsigned long* dims, int nDims) {
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

void pushDynamicArray(SymbolStack *stack, char *name, char *moduleHash, short type) {
    Symbol symbol = {
        .name = name,
        .data = (DynamicArrayData *) malloc(sizeof(DynamicArrayData)),
        .level = stack->level,
        .type = DARRAY
    };
    ((DynamicArrayData *) symbol.data)->type = type;
    pushSymbol(stack, symbol);
}

void pushObject(SymbolStack *stack, char *name, char *moduleHash, Symbol *fields, unsigned long fieldsLength) {
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

short verifyFunction(SymbolStack *stack, char *name, char *moduleHash, node *args, node *reordenedArgs, unsigned long argsLength) {
    for (int i = stack->size - 1; i >= 0; i--) {
        // Search for a function with the same name
        if (stack->table[i].type == FUNCTION && strcmp(stack->table[i].name, name) == 0 && strcmp(stack->table[i].moduleHash, moduleHash) == 0) {
            // Create a list of remaining parameters
            FunctionData *function = (FunctionData *) stack->table[i].data;
            Symbol *params = (Symbol *) malloc(sizeof(Symbol) * function->paramsLength);
            memcpy(params, function->params, sizeof(Symbol) * function->paramsLength);
            Symbol *remainingParams = params;
            int remainingParamsLength = function->paramsLength;
            int ordened = 1;
            int allParamsFound = 1;
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
                        // If the parameter is not found, this is not the function we are looking for
                        allParamsFound = 0;
                        goto nextFunction;
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
            
            nextFunction:

            free(params);
            
            if (compulsoryArgsMissing || !allParamsFound) continue;

            return function->ret;
        }
    }
    printf("Error: Function '%s' is not defined\n", name);
    exit(1);
}

void _pushImport(SymbolStack *stack, char *name, char *moduleHash, char *importedModuleHash) {
    Symbol symbol = {
        .name = name,
        .moduleHash = (char *) malloc(strlen(moduleHash) + 1),
        .data = (ImportData *) malloc(sizeof(ImportData)),
        .level = stack->level,
        .type = IMPORT
    };
    strcpy(symbol.moduleHash, moduleHash);
    ((ImportData *) symbol.data)->moduleHash = importedModuleHash;
    pushSymbol(stack, symbol);
}

void* getModulePath(node *lib, char *dest) {
    // Node is of type DOT or VAR
    // Its child 0 is an expression with DOT Tree
    // The left part of the DOT Tree is the module path
    // The right part of the DOT Tree is the module name

    if (lib->data.type == DOT) {
        getModulePath(lib->children, dest);
        strcat(dest, "/");
        strcat(dest, lib->children[1].data.text);
    } else if (lib->data.type == VAR) {
        strcat(dest, lib->data.text);
    } else {
        printf("Error: Invalid module path\n");
        exit(1);
    }
}

void pushImport(SymbolStack *stack, node *importedModuleNode) {
    // Extract current module path
    char modulePath[PATH_MAX];
    strcpy(modulePath, getCurrentModule()->name);
    // Remove the module file name
    char *lastSlash = strrchr(modulePath, '/');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = '\0';
    } else {
        *modulePath = '.';
        *(modulePath+1) = '/';
        *(modulePath+2) = '\0';
    }

    getModulePath(importedModuleNode->children->children, modulePath);
    strcat(modulePath, ".ju");

    // Import the module
    moduleData module = importModule(stack, modulePath);
    char *moduleHash = module.hash;
    char *symbolName = module.name;
    lastSlash = strrchr(symbolName, '/') + 1;
    if (lastSlash != NULL) {
        symbolName = lastSlash;
    }


    // Push the import symbol
    _pushImport(stack, symbolName, getCurrentModuleHash(), moduleHash);
}

char *getImportSymbol(SymbolStack *stack, char *symbol, char *moduleHash) {
    // Search for the import symbol with this ^ moduleHash
    // If found, return the moduleHash of the imported module
    // If not found, return NULL

    assert(symbol != NULL);

    if (moduleHash == NULL) moduleHash = getCurrentModuleHash();
    
    for (int i = 0; i < stack->size; i++) {
        if (stack->table[i].type == IMPORT
            && strcmp(stack->table[i].moduleHash, moduleHash) == 0
            && strcmp(stack->table[i].name, symbol) == 0) {
                return ((ImportData *) stack->table[i].data)->moduleHash;
        }
    }
    return NULL;
}

/*
    @param function: a dot or a var node that identifies the function
*/
char *getFunctionName(node *function) {
    if (function->data.type == VAR) {
        return function->data.text;
    } else if (function->data.type == DOT) {
        return getFunctionName(function->children + 1);
    } else {
        printf("Error: Expected function name at line %d\n", function->data.line);
        exit(1);
    }
}

char *extractModuleHash(SymbolStack *symbolStack, node ast) {
    char *LHS;
    if (ast.data.type == DOT) {
        LHS = extractModuleHash(symbolStack, ast.children[0]);
    } else if (ast.data.type == VAR) {
        if (LHS = getImportSymbol(symbolStack, ast.data.text, NULL)) {
            return LHS;
        } else {
            return getCurrentModuleHash();
        }
    } else {
        printf("Error: Invalid module path\n");
        exit(1);
    }
    
    char *importSymbol;
    if (importSymbol = getImportSymbol(symbolStack, ast.children[1].data.text, LHS)) {
        return importSymbol;
    } else {
        return LHS;
    }
}


short getSymbolType(SymbolStack *stack, node *symbolNode) {
    if (symbolNode->data.type == VAR) {
        Symbol *symbol = findSymbol(stack, symbolNode->data.text, NULL);
        if (symbol == NULL) {
            printf("Error: Symbol '%s' is not defined\n", symbolNode->data.text);
            exit(1);
        }
        return symbol->type;
    } else if (symbolNode->data.type == DOT) {
        char *moduleHash = extractModuleHash(stack, *symbolNode);
        Symbol *symbol = findSymbol(stack, symbolNode->children[1].data.text, moduleHash);
        if (symbol == NULL) {
            printf("Error: Symbol '%s' is not defined\n", symbolNode->children[1].data.text);
            exit(1);
        }
        return symbol->type;
    } else {
        printf("Error: Expected symbol at line %d\n", symbolNode->data.line);
        exit(1);
    }
}

#define TYPES_COUNT 6

// The value in each cell is the type of the result of the operation
// CHAR, INT, LONG, FLOAT, DOUBLE, BOOL
const unsigned char arithmeticsTypesResolutionTable[TYPES_COUNT][TYPES_COUNT] = 
{
    {CHAR, INT, LONG, FLOAT, DOUBLE, CHAR},
    {INT, INT, LONG, FLOAT, DOUBLE, INT},
    {LONG, LONG, LONG, FLOAT, DOUBLE, LONG},
    {FLOAT, FLOAT, FLOAT, FLOAT, DOUBLE, FLOAT},
    {DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE},
    {CHAR, INT, LONG, FLOAT, DOUBLE, BOOL}
};

short resolveExpressionType(SymbolStack *stack, node *expression) {
    if (expression->data.type == VAR || expression->data.type == DOT)
        return getSymbolType(stack, expression);
        
    if (expression->data.type == INUM)
        return INT;
    
    if (expression->data.type == FNUM)
        return FLOAT;
    
    if (expression->data.type == TRUE || expression->data.type == FALSE)
        return BOOL;

    if (expression->data.type >= PLUS && expression->data.type <= MOD) {
        short types[2];
        types[0] = resolveExpressionType(stack, expression->children);
        types[1] = resolveExpressionType(stack, expression->children + 1);
        return arithmeticsTypesResolutionTable[types[0] - CHAR][types[1] - CHAR];
    }
        
    if (expression->data.type >= NOT && expression->data.type <= LTE)
        return BOOL;
        
    if (expression->data.type >= ASSIGN && expression->data.type <= BSR)
        return resolveExpressionType(stack, expression->children);
    
    if (expression->data.type == CALL) {
        char *functionName = getFunctionName(expression->children);
        return verifyFunction(stack, functionName, NULL, expression->children + 1, NULL, expression->children[1].length);
    }
}

int verifyType(SymbolStack* stack, node *expression1, node *expression2) {
    return resolveExpressionType(stack, expression1) == resolveExpressionType(stack, expression2);
}
