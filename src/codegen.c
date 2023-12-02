#include <llvm-c/Core.h>
#include <llvm-c/BitWriter.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "symbolsNaming.h"
#include "token.h"
#include "module.h"
#include "parserSymbols.h"
#include "parser.h"


#define MAXIMUM_MODULE_NAME_LENGTH 64
int compDebug = 0;
LLVMValueRef printf_f;

SymbolStack* currentGlobalStack;

// Internal functions
int numberOfDigits(int number) {
    int digits = 1;
    while (number >= 10) {
        number /= 10;
        digits++;
    }
    return digits;
}

LLVMBasicBlockRef newNamedBlock(LLVMBuilderRef builder, char *name, int id) {
    int nameLength = strlen(name);
    char *newName = (char *) malloc(sizeof(char) * (nameLength + 1 + numberOfDigits(id)));
    sprintf(newName, "%s%d", name, id);
    return LLVMAppendBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)), newName);
}


// Symbol Table
// Saves all local symbols in a linked list
struct LST {
    char *name;
    LLVMValueRef value;
    LLVMTypeRef type;
    int level;
    struct LST *prev;
    struct LST *next;
} symbolTable;

int currentLevel;

void initSymbolTable() {
    symbolTable.name = NULL;
    symbolTable.value = NULL;
    symbolTable.type = NULL;
    symbolTable.level = 0;
    symbolTable.prev = NULL;
    symbolTable.next = NULL;
    currentLevel = 0;
}

struct LST* getLastSymbol() {
    struct LST* currentSymbol = &symbolTable;
    while (currentSymbol->next != NULL) {
        currentSymbol = currentSymbol->next;
    }
    return currentSymbol;
}

void newSymbol(char *name, LLVMValueRef value, LLVMTypeRef type) {
    struct LST* currentSymbol = getLastSymbol();
    currentSymbol->name = name;
    currentSymbol->value = value;
    currentSymbol->type = type;
    currentSymbol->level = currentLevel;
    currentSymbol->next = (struct LST *) malloc(sizeof(struct LST));
    *currentSymbol->next = (struct LST) {
        .name = NULL,
        .value = NULL,
        .type = NULL,
        .level = 0,
        .prev = currentSymbol,
        .next = NULL
    };
}

struct LST* getSymbol(char *name) {
    struct LST* currentSymbol = getLastSymbol()->prev;
    while (strcmp(currentSymbol->name, name) != 0) {
        if (currentSymbol->prev == NULL) {
            printf("Error: Symbol %s not found\n", name);
            exit(1);
        }
        currentSymbol = currentSymbol->prev;
    }
    return currentSymbol;
}

LLVMValueRef getValueFromSymbolTable(char *name) {
    struct LST* symbol = getSymbol(name);
    return symbol->value;
}

void updateSymbol(char *name, LLVMValueRef newValue) {
    getSymbol(name)->value = newValue;
}

void newLevel() {
    currentLevel++;
}

void deleteLevel() {
    if (currentLevel == 0) {
        printf("Error: Cannot delete level 0\n");
        exit(1);
    }

    struct LST* currentSymbol = getLastSymbol()->prev;
    currentLevel--;

    while (currentSymbol->level > currentLevel) {
        free(currentSymbol->next);
        currentSymbol->next = NULL;
        currentSymbol = currentSymbol->prev;
    }
}
// End Symbol Table


char* getModuleName(char *path) {
    char* name = strdup(path);
    // Replace / by .
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] == '/') {
            name[i] = '.';
        }
    }
    return name;
}



// Type checking functions
int isIntegerNumberType(LLVMTypeRef type) {
    LLVMTypeKind typeKind = LLVMGetTypeKind(type);

    return typeKind == LLVMIntegerTypeKind;
}

int isFloatingPointNumberType(LLVMTypeRef type) {
    LLVMTypeKind typeKind = LLVMGetTypeKind(type);

    return typeKind == LLVMHalfTypeKind
        || typeKind == LLVMFloatTypeKind
        || typeKind == LLVMDoubleTypeKind
        || typeKind == LLVMX86_FP80TypeKind
        || typeKind == LLVMFP128TypeKind;
}

int isNumberType(LLVMTypeRef type) {
    return isIntegerNumberType(type) || isFloatingPointNumberType(type);
}

int isBooleanType(LLVMTypeRef type) {
    return type == LLVMInt1Type();
}


LLVMModuleRef createModule(char* name) {
    return LLVMModuleCreateWithName(name);
}

LLVMValueRef createNewFunction(LLVMModuleRef mod, char *name, LLVMTypeRef *param_types, int param_count, LLVMTypeRef ret_type) {
    LLVMTypeRef function_type = LLVMFunctionType(ret_type, param_types, param_count, 0);
    LLVMValueRef function = LLVMAddFunction(mod, name, function_type);
    printf("Function Created - %s(%d params)\n", name, LLVMCountParams(function));
    return function;
}

LLVMTypeRef getLLVMType(Token token) {
    LLVMTypeRef type;
    switch (token.type) {
        case INT:
            type = LLVMInt32Type();
            break;
        case LONG:
            type = LLVMInt64Type();
            break;
        case FLOAT:
            type = LLVMFloatType();
            break;
        case DOUBLE:
            type = LLVMDoubleType();
            break;
        case BOOL:
            type = LLVMInt1Type();
            break;
        case CHAR:
            type = LLVMInt16Type();
            break;
        default:
            fprintf(stderr, "error: invalid token\n");
            exit(1);
    }
    return type;
}


short getTokenType(LLVMTypeRef LLVMType) {
    short type;
    switch (LLVMGetTypeKind(LLVMType)) {
        case LLVMIntegerTypeKind:
            switch (LLVMGetIntTypeWidth(LLVMType)) {
                case 1:
                    type = BOOL;
                    break;
                case 8:
                    type = CHAR;
                    break;
                case 32:
                    type = INT;
                    break;
                case 64:
                    type = LONG;
                    break;
                default:
                    fprintf(stderr, "error: invalid type\n");
                    exit(1);
            }
            break;
        case LLVMFloatTypeKind:
            type = FLOAT;
            break;
        case LLVMDoubleTypeKind:
            type = DOUBLE;
            break;
        default:
            fprintf(stderr, "error: invalid type\n");
            exit(1);
    }
    return type;
}


// Add all global value of iMod to oMod
void importSymbols(LLVMModuleRef oMod, LLVMModuleRef iMod) {
    LLVMValueRef value = LLVMGetFirstGlobal(iMod);
    while(value != NULL) {
        LLVMTypeRef type = LLVMTypeOf(value);
        LLVMAddGlobal(oMod, type, LLVMGetValueName(value));
        value = LLVMGetNextGlobal(value);
    }
    LLVMValueRef function = LLVMGetFirstFunction(iMod);
    while(function != NULL) {
        if (strlen(LLVMGetValueName(function)) > 20) {
            LLVMTypeRef type = LLVMGlobalGetValueType(function);

            // LLVMGetValueName is working and returning the correct name
            char* name = LLVMGetValueName(function);

            // TODO: The LLVMAddFunction below is corrupting the oMod
            // LLVMWriteBitcodeToFile will not work
            LLVMAddFunction(oMod, LLVMGetValueName(function), type);

            // LLVMGetReturnType is also not working (seg fault)
            LLVMGetReturnType(type);
            printf("Function Imported - %s(", name);
            LLVMTypeRef *functionParams = malloc(sizeof(LLVMTypeRef) * (LLVMCountParams(function) + 1));

            // LLVMGetParamTypes is also not working (seg fault)
            LLVMGetParamTypes(type, functionParams);
            for (int i = 0; i < LLVMCountParams(function); i++) {
                char *typeString = LLVMPrintTypeToString(functionParams[i]);
                printf("%s, ", typeString);
                free(typeString);
            }
            printf(")\n");
            free(functionParams);
        }
        function = LLVMGetNextFunction(function);
    }
}

void generateDeclaration(LLVMModuleRef mod, node ast, int global, LLVMBuilderRef builder) {
    short typeToken = ast.children[0].data.type;
    LLVMTypeRef type = getLLVMType(ast.children[0].data);
    
    LLVMValueRef value;
    if (global) {
        value = LLVMAddGlobal(mod, type, ast.children[1].data.text);
        LLVMSetInitializer(value, LLVMConstNull(type));
    } else {
        value = LLVMBuildAlloca(builder, type, ast.children[1].data.text);
    }
    newSymbol(ast.children[1].data.text, value, type);
}


// Get global symbols (global variables, etc.)
void generateSymbols(LLVMModuleRef mod, node ast) {
    // Iterate over the root node's children
    for (int i = 0; i < ast.length; i++) {
        if (ast.children[i].data.type == DECLARATION) {
            generateDeclaration(mod, ast.children[i], 1, NULL);
        }
    }
}


// Generate code for literals
LLVMValueRef generateLiteral(LLVMModuleRef mod, node ast, LLVMBuilderRef builder) {
    // Iterate over the root node's children
    if (ast.data.type == INUM) {
        LLVMTypeRef type = LLVMInt32Type();
        return LLVMConstInt(type, atoi(ast.data.text), 0);
    } else if (ast.data.type == FNUM) {
        LLVMTypeRef type = LLVMDoubleType();
        return LLVMConstReal(type, atof(ast.data.text));
    } else if (ast.data.type == TRUE) {
        LLVMTypeRef type = LLVMInt1Type();
        return LLVMConstInt(type, 1, 0);
    } else if (ast.data.type == FALSE) {
        LLVMTypeRef type = LLVMInt1Type();
        return LLVMConstInt(type, 0, 0);
    } else if (ast.data.type == NLL) {
        LLVMTypeRef type = LLVMPointerType(LLVMInt8Type(), 0);
        return LLVMConstPointerNull(type);
    } else {
    }
}

// Convert to boolean
LLVMValueRef convertToBoolean(LLVMModuleRef mod, LLVMValueRef value, LLVMBuilderRef builder) {
    // Verify if type is i1
    // If not convert to i1
    // The value should be 0 if value is 0 and 1 if value is not 0
    // First, verify if is a float and get the compare function
    LLVMTypeRef type = LLVMTypeOf(value);
    if (isBooleanType(type)) {
        return value;
    } else if (isIntegerNumberType(type)) {
        return LLVMBuildICmp(builder, LLVMIntNE, value, LLVMConstInt(type, 0, 0), "");
    } else if (isFloatingPointNumberType(type)) {
        return LLVMBuildFCmp(builder, LLVMRealUNE, value, LLVMConstReal(type, 0), "");
    } else {
        fprintf(stderr, "error: invalid type\n");
        exit(1);
    }
}

// Generate code for expression
LLVMValueRef generateExpression(LLVMModuleRef mod, node ast, LLVMBuilderRef builder) {
    // Verify for assignement or unary operation
    if (ast.data.type == ASSIGN) {
        LLVMValueRef newValue = generateExpression(mod, ast.children[1], builder);
        LLVMValueRef pointerToValue = getValueFromSymbolTable(ast.children[0].data.text);
        LLVMBuildStore(builder, newValue, pointerToValue);
        return newValue;
    } if (ast.data.type == NOT) {
        return LLVMBuildNot(builder, convertToBoolean(mod, generateExpression(mod, ast.children[0], builder), builder), "nottmp");
    } else if (ast.data.type == BNOT) {
        return LLVMBuildNot(builder, generateExpression(mod, ast.children[0], builder), "bnottmp");
    } else if (ast.data.type >> 4 == 0x03) {
        // For literal
        return generateLiteral(mod, ast, builder);
    } else if (ast.data.type == ARRAY) {
        LLVMValueRef firstElement = generateExpression(mod, ast.children[0], builder);
        LLVMTypeRef elementType = LLVMTypeOf(firstElement);
        LLVMValueRef *values = (LLVMValueRef *) malloc(sizeof(LLVMValueRef) * ast.length);
        for (int j = 0; j < ast.length; j++) {
            values[j] = generateExpression(mod, ast.children[j], builder);
        }
        return LLVMConstArray(elementType, values, ast.length);
    } else if (ast.data.type == VAR) {
        // For variable
        struct LST* symbol = getSymbol(ast.data.text);
        return LLVMBuildLoad2(builder, symbol->type, symbol->value, "loadtmp");
    } else if (ast.data.type == CALL) {
        // For a function call
        short* arguments_types = (short*) malloc(sizeof(short) * (ast.length - 1));
        LLVMValueRef* arguments = (LLVMValueRef*) malloc(sizeof(LLVMValueRef) * (ast.length - 1));
        for (int i = 0; i < ast.length - 1; i++) {
            for (unsigned int j = 0; j < ast.children[i+1].length; j++) {
                if (ast.children[i+1].children[j].data.type == EXPRESSION) {
                    arguments[i] = generateExpression(mod, ast.children[i+1].children[j].children[0], builder);
                    arguments_types[i] = getTokenType(LLVMTypeOf(arguments[i]));
                }
            }
        }
        char* paramsHash = generateParametersHash(arguments_types, ast.length - 1);
        free(arguments_types);
        node* functionSymbolName = ast.children;
        char* moduleHash = extractModuleHash(currentGlobalStack, *functionSymbolName);
        char* functionName = getFunctionName(functionSymbolName);
        char* fullName = (char *) malloc(sizeof(char) * (strlen(moduleHash) + strlen(functionName) + strlen(paramsHash) + 1));
        sprintf(fullName, "%s%s%s", moduleHash, functionName, paramsHash);
        LLVMValueRef function = LLVMGetNamedFunction(mod, fullName);
        free(fullName);

        unsigned int params_count = LLVMCountParams(function);
        return LLVMBuildCall2(builder, LLVMGlobalGetValueType(function), function, arguments, params_count, "calltmp");
        free(arguments);
    }

    // For binary operation
    LLVMValueRef LHS = generateExpression(mod, ast.children[0], builder);
    LLVMValueRef RHS = generateExpression(mod, ast.children[1], builder);

    if (ast.data.type == PLUS) {
        return LLVMBuildAdd(builder, LHS, RHS, "addtmp");
    } else if (ast.data.type == MINUS) {
        return LLVMBuildSub(builder, LHS, RHS, "subtmp");
    } else if (ast.data.type == MULT) {
        return LLVMBuildMul(builder, LHS, RHS, "multmp");
    } else if (ast.data.type == DIV) {
        return LLVMBuildSDiv(builder, LHS, RHS, "divtmp");
    } else if (ast.data.type == MOD) {
        return LLVMBuildSRem(builder, LHS, RHS, "modtmp");
    } else if (ast.data.type == DIV) {
        return LLVMBuildSDiv(builder, LHS, RHS, "divtmp");
    } else if (ast.data.type == MOD) {
        return LLVMBuildSRem(builder, LHS, RHS, "modtmp");
    } else if (ast.data.type >= EQ && ast.data.type <= LTE) {
        LLVMIntPredicate pred;
        if (ast.data.type == EQ) {
            pred = LLVMIntEQ;
        } else if (ast.data.type == NEQ) {
            pred = LLVMIntNE;
        } else if (ast.data.type == GT) {
            pred = LLVMIntSGT;
        } else if (ast.data.type == GTE) {
            pred = LLVMIntSGE;
        } else if (ast.data.type == LT) {
            pred = LLVMIntSLT;
        } else if (ast.data.type == LTE) {
            pred = LLVMIntSLE;
        }
        return LLVMBuildICmp(builder, pred, LHS, RHS, "cmptmp");
    } else if (ast.data.type == BAND) {
        return LLVMBuildAnd(builder, LHS, RHS, "andtmp");
    } else if (ast.data.type == BOR) {
        return LLVMBuildOr(builder, LHS, RHS, "ortmp");
    } else if (ast.data.type == BXOR) {
        return LLVMBuildXor(builder, LHS, RHS, "xortmp");
    } else if (ast.data.type == BSL) {
        return LLVMBuildShl(builder, LHS, RHS, "shltmp");
    } else if (ast.data.type == BSR) {
        return LLVMBuildLShr(builder, LHS, RHS, "shrtemp");
    } else if (ast.data.type >= AND && ast.data.type <= XOR) {
        // Logical operations
        LHS = convertToBoolean(mod, LHS, builder);
        RHS = convertToBoolean(mod, RHS, builder);
        if (ast.data.type == AND) {
            return LLVMBuildAnd(builder, LHS, RHS, "andtmp");
        } else if (ast.data.type == OR) {
            return LLVMBuildOr(builder, LHS, RHS, "ortmp");
        } else if (ast.data.type == XOR) {
            return LLVMBuildXor(builder, LHS, RHS, "xortmp");
        }
    } else if (ast.data.type == DOT) {
        
    } else {
        fprintf(stderr, "error: invalid token\n");
        exit(1);
    }

    return 0;
}

// Generate code for function
LLVMValueRef generateFunction(LLVMModuleRef mod, char *moduleName, char *name, LLVMTypeRef *llvm_param_types, unsigned int paramsLength, LLVMTypeRef ret_type, node ast, LLVMBuilderRef builder) {    
    LLVMValueRef funct = createNewFunction(mod, name, llvm_param_types, paramsLength, getLLVMType(ast.children[0].data));

    LLVMBasicBlockRef block = LLVMAppendBasicBlock(funct, "entry");
    newSymbol(name, funct, LLVMTypeOf(funct));
    newLevel();
    LLVMBuilderRef functionBuilder = LLVMCreateBuilder();
    LLVMPositionBuilderAtEnd(functionBuilder, block);
    unsigned int params_count = LLVMCountParams(funct);
    LLVMValueRef* paramsValues = (LLVMValueRef*) malloc(sizeof(LLVMValueRef) * params_count);
    LLVMGetParams(funct, paramsValues);
    for(unsigned int i = 0; i < params_count; i++) {
        LLVMValueRef paramPtr = LLVMBuildAlloca(functionBuilder, LLVMTypeOf(paramsValues[i]), ast.children[i + 2].children[1].data.text);
        LLVMBuildStore(functionBuilder, paramsValues[i], paramPtr);
        newSymbol(ast.children[i + 2].children[1].data.text, paramPtr, LLVMTypeOf(paramsValues[i]));
    }
    free(paramsValues);
    generateStatement(mod, moduleName, ast.children[ast.length - 1], functionBuilder);
    deleteLevel();
    return funct;
}

int isBitmaskZero(unsigned int* bitmask, int size) {
    for (int i = 0; i < size; i++) {
        if (bitmask[i] != 0) {
            return 0;
        }
    }
    return 1;
}

// Generate code for statement
void generateStatement(LLVMModuleRef mod, char* moduleName, node ast, LLVMBuilderRef builder) {
    // Iterate over the root node's children and search for statements
    if (ast.data.type == IFSTATEMENT) {
        static int ifCount = 0;
        // Add a new basic block for the if statement
        char* blockNames[4] = {"iftest", "if", "endif", "else"};
        LLVMBasicBlockRef ifBlocks[4];
        for (int i = 0; i < ast.length + 1; i++) {
            ifBlocks[i] = newNamedBlock(builder, blockNames[i], ifCount);
        }
        // Generate the condition
        LLVMBuildBr(builder, ifBlocks[0]);
        LLVMPositionBuilderAtEnd(builder, ifBlocks[0]);
        LLVMValueRef condition = convertToBoolean(mod, generateExpression(mod, ast.children[0].children[0], builder), builder);

        switch (ast.length)
        {
        case 2:
            LLVMBuildCondBr(builder, condition, ifBlocks[1], ifBlocks[2]);
            break;

        case 3:
            LLVMBuildCondBr(builder, condition, ifBlocks[1], ifBlocks[3]);
            break;
        }
        // Generate true block
        LLVMPositionBuilderAtEnd(builder, ifBlocks[1]);
        generateStatement(mod, moduleName, ast.children[1], builder);
        LLVMBuildBr(builder, ifBlocks[2]);
        // Generate false block
        if (ast.length == 3) {
            LLVMPositionBuilderAtEnd(builder, ifBlocks[3]);
            generateStatement(mod, moduleName, ast.children[2], builder);
            LLVMBuildBr(builder, ifBlocks[2]);
        }
        // Back to merge block
        LLVMPositionBuilderAtEnd(builder, ifBlocks[2]);


    } else if (ast.data.type == WHILELOOPSTATEMENT) {
        static int whileCount = 0;
        // Add a new basic block for the while loop
        char* blockNames[3] = {"whiletest", "while", "endwhile"};
        LLVMBasicBlockRef whileBlocks[3];
        for (int i = 0; i < ast.length; i++) {
            whileBlocks[i] = newNamedBlock(builder, blockNames[i], whileCount);
        }
        // Generate the condition
        LLVMPositionBuilderAtEnd(builder, whileBlocks[0]);
        LLVMValueRef condition = convertToBoolean(mod, generateExpression(mod, ast.children[0], builder), builder);
        LLVMBuildCondBr(builder, condition, whileBlocks[1], whileBlocks[2]);
        // Generate while block
        LLVMPositionBuilderAtEnd(builder, whileBlocks[1]);
        generateStatement(mod, moduleName, ast.children[1], builder);
        LLVMBuildBr(builder, whileBlocks[0]);
        // Back to merge block
        LLVMPositionBuilderAtEnd(builder, whileBlocks[2]);


    } else if (ast.data.type == DOWHILELOOPSTATEMENT) {
        static int doWhileCount = 0;
        int nDigits = numberOfDigits(doWhileCount);
        // Add a new basic block for the do while loop
        char* blockNames[3] = {"dowhile", "dowhiletest", "enddowhile"};
        LLVMBasicBlockRef doWhileBlocks[3];
        for (int i = 0; i < ast.length; i++) {
            doWhileBlocks[i] = newNamedBlock(builder, blockNames[i], doWhileCount);
        }
        // Generate do while block
        LLVMPositionBuilderAtEnd(builder, doWhileBlocks[0]);
        generateStatement(mod, moduleName, ast.children[0], builder);
        LLVMBuildBr(builder, doWhileBlocks[1]);
        // Generate the condition
        LLVMPositionBuilderAtEnd(builder, doWhileBlocks[1]);
        LLVMValueRef condition = convertToBoolean(mod, generateExpression(mod, ast.children[1], builder), builder);
        LLVMBuildCondBr(builder, condition, doWhileBlocks[0], doWhileBlocks[2]);
        // Back to merge block
        LLVMPositionBuilderAtEnd(builder, doWhileBlocks[2]);

    } else if (ast.data.type == FORLOOPSTATEMENT) {
        // TODO
    } else if (ast.data.type == FUNCTION) {
        int paramsLength = ast.length - 3;
        for (int i = 2; i < ast.length - 1; i++) {
            if (ast.children[i].data.type == EXPRESSION) {
                paramsLength--;
            }
        }
        LLVMTypeRef* llvm_param_types = malloc(sizeof(LLVMTypeRef) * paramsLength);
        short* param_types = malloc(sizeof(short) * paramsLength);
        node* defaultExpressions = malloc(sizeof(node) * 2 * (ast.length - 3 - paramsLength));
        int compulsoryParams = 0;
        int paramsIndex = 0;
        int expressionIndex = 0;
        for (int i = 2; i < ast.length - 1; i++) {
            // Count compulsory parameters ...
            if (ast.children[i].data.type == EXPRESSION) {
                compulsoryParams--;
                // ... save default expressions ...
                defaultExpressions[expressionIndex * 2] = ast.children[i - 1];
                defaultExpressions[expressionIndex * 2 + 1] = ast.children[i].children[0];
                expressionIndex++;
            } else {
                compulsoryParams++;
                // ... and add types to the array
                llvm_param_types[paramsIndex] = getLLVMType(ast.children[i].children[0].data);
                param_types[paramsIndex] = ast.children[i].children[0].data.type;
                paramsIndex++;
            }
        }

        char* moduleHash = getModuleHash(moduleName);
        char* paramsHash = generateParametersHash(param_types, paramsLength);
        char* rootFunctionName = (char *) malloc(sizeof(char) * (strlen(ast.children[1].data.text) + strlen(paramsHash) + strlen(moduleHash) + 1));
        strcpy(rootFunctionName, moduleHash);
        strcat(rootFunctionName, ast.children[1].data.text);
        strcat(rootFunctionName, paramsHash);
        LLVMValueRef rootFunction = generateFunction(mod, moduleName, rootFunctionName, llvm_param_types, paramsLength, getLLVMType(ast.children[0].data), ast, builder);


        // Bitmask for non-compulsory parameters
        unsigned int* paramsBitmask = malloc(sizeof(unsigned int) * (paramsLength / sizeof(unsigned int) + 1));
        int remainingParams = paramsLength - compulsoryParams;
        for (int i = 0; i < paramsLength / sizeof(unsigned int) + 1; i++) {
            if (remainingParams < sizeof(unsigned int) * 8) {
                paramsBitmask[i] = (1 << remainingParams) - 1;
            } else {
                paramsBitmask[i] = 0xFFFFFFFF;
                remainingParams -= sizeof(unsigned int) * 8;
            }
        }

        // Generate a subfunction for each combination of non-compulsory parameters
        while (!isBitmaskZero(paramsBitmask, paramsLength / sizeof(unsigned int) + 1)) {
            // Decrease bitmask
            for (int i = 0; i < paramsLength / sizeof(unsigned int) + 1; i++) {
                if (paramsBitmask[i] != 0) {
                    paramsBitmask[i]--;
                    break;
                } else {
                    paramsBitmask[i] = 0xFFFFFFFF;
                }
            }

            // Generate parameters sequences
            short* subFunctionParamTypes = malloc(sizeof(short) * (paramsLength));
            // Copy compulsory parameters to sequences
            memcpy(subFunctionParamTypes, param_types, sizeof(short) * compulsoryParams);
            int nonCompulsoryParams = 0;
            for (int i = 0; i < paramsLength - compulsoryParams; i++) {
                if (paramsBitmask[sizeof(unsigned int) * i / 32] & (1 << (i % 32))) {
                    subFunctionParamTypes[compulsoryParams + i] = param_types[compulsoryParams + i];
                    nonCompulsoryParams++;
                }
            }

            // Generate a function name
            char* paramsHash = generateParametersHash(subFunctionParamTypes, compulsoryParams + nonCompulsoryParams);
            free(subFunctionParamTypes);
            char* subFunctionName = (char *) malloc(sizeof(char) * (strlen(rootFunctionName) + 1 + numberOfDigits(compulsoryParams + nonCompulsoryParams)));
            strcpy(subFunctionName, moduleHash);
            strcat(subFunctionName, ast.children[1].data.text);
            strcat(subFunctionName, paramsHash);
            LLVMValueRef subFunction = createNewFunction(mod, subFunctionName, llvm_param_types, compulsoryParams + nonCompulsoryParams, getLLVMType(ast.children[0].data));
            LLVMBasicBlockRef block = LLVMAppendBasicBlock(subFunction, "entry");
            newSymbol(subFunctionName, subFunction, LLVMTypeOf(subFunction));
            LLVMBuilderRef functionBuilder = LLVMCreateBuilder();
            LLVMPositionBuilderAtEnd(functionBuilder, block);
            newLevel();

            // Subfunction builder
            LLVMValueRef* args = (LLVMValueRef*) malloc(sizeof(LLVMValueRef) * paramsLength);
            int paramIndex = 0;
            for (int i = 0; i < paramsLength ; i++) {
                // NOTE: In this case it's the args length because paramsLength refers to
                // the number of parameters in the root function that we are calling
                if (i < compulsoryParams) {
                    args[i] = LLVMGetParam(subFunction, paramIndex);
                    paramIndex++;
                } else {
                    int nonCompulsoryIndex = i - compulsoryParams;
                    if (paramsBitmask[sizeof(unsigned int) * nonCompulsoryIndex / 32] & (1 << (nonCompulsoryIndex % 32))) {
                        args[i] = LLVMGetParam(subFunction, paramIndex);
                        paramIndex++;
                    } else {
                        generateDeclaration(mod, defaultExpressions[nonCompulsoryIndex * 2], 0, functionBuilder);
                        args[i] = generateExpression(mod, defaultExpressions[nonCompulsoryIndex * 2 + 1], functionBuilder);
                    }
                }
            }
            LLVMValueRef ret = LLVMBuildCall2(functionBuilder, LLVMGlobalGetValueType(rootFunction), rootFunction, args, paramsLength, "calltmp");
            LLVMBuildRet(functionBuilder, ret);
        }

        free(param_types);
        free(llvm_param_types);
        free(paramsBitmask);

    } else if (ast.data.type == RETURN) {
        LLVMBuildRet(builder, generateExpression(mod, ast.children[0].children[0], builder));
    } else if (ast.data.type == IMPORT) {
        LLVMModuleRef importedMod = getModuleRef(extractModuleHash(currentGlobalStack, ast.children->children[0]));
        size_t moduleIdSize;
        char *moduleId = LLVMGetSourceFileName(importedMod, &moduleIdSize);
        if (importedMod == NULL) {
            fprintf(stderr, "error: module not found\n");
            exit(1);
        }
        importSymbols(mod, importedMod);
    } else if (ast.data.type == BREAK) {
        // TODO
    } else if (ast.data.type == CONTINUE) {
        // TODO
    } else if (ast.data.type == BLOCK) {
        newLevel();
        for (int i = 0; i < ast.length; i++) {
            generateStatement(mod, moduleName, ast.children[i], builder);
        }
        deleteLevel();
    } else if (ast.data.type == EXPRESSION) {
        printf("Generating expression\n");
        generateExpression(mod, ast.children[0], builder);
    }
}

// Generate code for main function
LLVMValueRef generateMain(LLVMModuleRef mod, char* name, node ast, LLVMBuilderRef builder) {
    // Create a new function
    printf("Creating main() function\n");
    LLVMTypeRef function_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    char* mainName = (char *) malloc(sizeof(char) * (strlen(getModuleHash(name)) + 6));
    sprintf(mainName, "%smain", getModuleHash(name));
    LLVMValueRef main = LLVMAddFunction(mod, mainName, function_type);
    printf("main() created\n");
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    // printDouble function

    if (compDebug) {
        // Declare printf external function
        LLVMTypeRef printf_args[] = {LLVMPointerType(LLVMInt8Type(), 0)};
        printf_f = LLVMAddFunction(mod, "printf", LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0), printf_args, 1, 1));
    }

    // Iterate over the root node's children and search for instruction
    for (int i = 0; i < ast.length; i++) {
        if (ast.children[i].data.type != DECLARATION) {
            generateStatement(mod, name, ast.children[i], builder);
        }
    }

    LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));

    return main;
}


LLVMModuleRef generateCode(char *name, SymbolStack *_currentGlobalStack, node ast, int isMain) {
    LLVMModuleRef mod = createModule(name);
    LLVMBuilderRef builder = LLVMCreateBuilder();

    currentGlobalStack = _currentGlobalStack;

    initSymbolTable();

    // Verify if first node is the root node
    if (ast.data.type != ROOT) {
        fprintf(stderr, "error: root node not found\n");
        return;
    }

    generateSymbols(mod, ast);

    // Create main function
    LLVMValueRef mainFunction = generateMain(mod, name, ast, builder);
    if (isMain) {
        // Create a new function
        LLVMTypeRef function_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
        LLVMValueRef main = LLVMAddFunction(mod, "main", function_type);
        LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main, "entry");
        LLVMPositionBuilderAtEnd(builder, entry);
        // Call main function
        LLVMValueRef ret = LLVMBuildCall2(builder, LLVMGlobalGetValueType(mainFunction), mainFunction, NULL, 0, "calltmp");
        LLVMBuildRet(builder, ret);
    }

    // Generate bytecode
    char* destinationFile = (char *) malloc(strlen(name) + 1);
    strcpy(destinationFile, name);
    strcpy(strrchr(destinationFile, 46), ".bc");
    size_t moduleIdSize;
    char *moduleId = LLVMGetSourceFileName(mod, &moduleIdSize);
    if (LLVMWriteBitcodeToFile(mod, destinationFile) != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }
    free(destinationFile);

    return mod;
}
