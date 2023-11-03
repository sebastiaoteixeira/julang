#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lexer.h"
#include "logger.h"
#include "module.h"
#include "codegen.h"
#include "symbolsNaming.h"
#include "parserSymbols.h"
#include "token.h"

int main(int argc, char **argv)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    rootPath = cwd;
    initModulesList();
    initModuleCompilationStack();

    SymbolStack *symbolStack = createSymbolStack();

    if (argc < 2) return 1;
    compileModule(symbolStack, argv[1], 1);

    destroySymbolStack(symbolStack);

    unsigned int totalSize = 0;
    for (int i = 0; i < modulesList.nModules; i++) {
        totalSize += strlen(modulesList.modules[i].name) + 3;
    }
    char* linkerCommand = (char *) malloc(totalSize + 7);
    // TODO: call ld instead of clang
    memcpy(linkerCommand, "clang ", 7);
    for (int i = 0; i < modulesList.nModules; i++) {
        sprintf(linkerCommand + strlen(linkerCommand), "%s.o ", modulesList.modules[i].name);
    }
    system(linkerCommand);
    free(linkerCommand);

    printf("Terminated\n");
    return 0;
}
