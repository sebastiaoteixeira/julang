#ifndef MODULE_H
#define MODULE_H
#include <llvm-c/Core.h>
#include "parserSymbols.h"

typedef struct
{
    char *name;
    char *hash;
    LLVMModuleRef module;
} moduleData;

// ModuleCompilationStack
// This stack is used to keep track of the modules that are being compiled
struct MCS {
    int stackSize;
    int stackCapacity;
    moduleData *modules;
};
extern struct MCS moduleCompilationStack;

void pushModule(char *name, char *hash);
void popModule();
moduleData *getCurrentModule();
void initModuleCompilationStack();

struct ml {
    moduleData *modules;
    unsigned int nModules;
};

extern struct ml modulesList;

void initModulesList();
LLVMModuleRef getModuleRef(char* name);

moduleData compileModule(SymbolStack *stack, char* filePath, int isMain);
moduleData importModule(SymbolStack *stack, char* filePath);
char* getModuleHash(char *filename);


#endif // MODULE_H