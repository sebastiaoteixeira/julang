#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "module.h"
#include "logger.h"
#include "codegen.h"
#include "symbolsNaming.h"
#include "parserSymbols.h"
#include "lexer.h"
#include "parser.h"

FILE *readFile(char *fileName)
{
    FILE *fp;
    fp = fopen(fileName, "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }

    return fp;
}


char *getModuleHash(char *filename) {
    FILE *fp = readFile(filename);
    Token *tokenList = runLexer(fp);
    return generateModuleHash(tokenList);
}

moduleData compileModule(SymbolStack *symbolStack, char* filePath, int isMain) {
    FILE *inputCode = readFile(filePath);

    Token *tokenList = runLexer(inputCode);

    char *oldModuleHash = getCurrentModuleHash();
    setCurrentModuleHash(generateModuleHash(tokenList));
    pushModule(filePath, getCurrentModuleHash());

    if (verbose >= 4)
        printTokenList(tokenList);
    
    int filenameSize = strlen(filePath) - 3;
    char* filename = malloc(filenameSize + 1);
    memcpy(filename, filePath, filenameSize);
    filename[filenameSize] = '\0';

    // Verify if module is already imported
    int isImported = 0;
    int moduleIndex = modulesList.nModules;
    for (int i = 0; i < modulesList.nModules; i++) {
        if (strcmp(modulesList.modules[i].name, filePath) == 0) {
            isImported = 1;
            moduleIndex = i;
            break;
        }
    }

    if (!isImported) {
        modulesList.modules[moduleIndex].name = filename;
        modulesList.modules[moduleIndex].hash = generateModuleHash(tokenList);
    
        modulesList.nModules++;
        modulesList.modules = realloc(modulesList.modules, sizeof(moduleData) * (modulesList.nModules + 1));

        node ast = runParser(symbolStack, tokenList);
        printf("Print Abstract Syntax Tree:\n");
        printAST(&ast);

        printf("\n\nCalling generator.... \n");
        modulesList.modules[moduleIndex].module = generateCode(filePath, symbolStack, ast, isMain);

        size_t moduleIdSize;
        char *moduleId = LLVMGetSourceFileName(modulesList.modules[moduleIndex].module, &moduleIdSize);
    }

    char* llcCompileCommand = malloc(2 * filenameSize + 30);
    sprintf(llcCompileCommand, "llc-15 %s.bc -O2 -filetype=obj", filename);
    system(llcCompileCommand);
    free(llcCompileCommand);
    

    popModule();
    setCurrentModuleHash(oldModuleHash);

    return modulesList.modules[moduleIndex];
}

moduleData importModule(SymbolStack *stack, char* filepath) {
    return compileModule(stack, filepath, 0);
}

struct ml modulesList;

void initModulesList() {
    modulesList.modules = malloc(sizeof(moduleData));
    modulesList.nModules = 0;
}

LLVMModuleRef getModuleRef(char* hash) {
    for (int i = 0; i < modulesList.nModules; i++) {
        if (strcmp(modulesList.modules[i].hash, hash) == 0) {
            return modulesList.modules[i].module;
        }
    }
    return NULL;
}


// Module Compilation Stack
struct MCS moduleCompilationStack;

void pushModule(char *name, char *hash) {
    moduleCompilationStack.modules[moduleCompilationStack.stackSize].name = name;
    moduleCompilationStack.modules[moduleCompilationStack.stackSize].hash = hash;
    moduleCompilationStack.stackSize++;
    if (moduleCompilationStack.stackSize == moduleCompilationStack.stackCapacity) {
        moduleCompilationStack.stackCapacity <<= 1;
        moduleCompilationStack.modules = realloc(moduleCompilationStack.modules, sizeof(moduleData) * moduleCompilationStack.stackCapacity);
    }
}

void popModule() {
    moduleCompilationStack.stackSize--;
}

moduleData *getCurrentModule() {
    return &moduleCompilationStack.modules[moduleCompilationStack.stackSize - 1];
}

void initModuleCompilationStack() {
    moduleCompilationStack.stackSize = 0;
    moduleCompilationStack.stackCapacity = 1;
    moduleCompilationStack.modules = malloc(sizeof(moduleData));
}

void destroyModuleCompilationStack() {
    free(moduleCompilationStack.modules);
}