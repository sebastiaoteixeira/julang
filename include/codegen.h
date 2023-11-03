#ifndef CODEGEN_H
#define CODEGEN_H
#include <llvm-c/Core.h>
#include "parser.h"


LLVMModuleRef generateCode(char *modName, SymbolStack *_currentGlobalStack, node ast, int isMain);

void generateStatement(LLVMModuleRef mod, char* moduleName, node ast, LLVMBuilderRef builder);

#endif // CODEGEN_H