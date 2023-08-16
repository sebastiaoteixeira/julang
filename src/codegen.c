#include <llvm-c-15/llvm-c/Core.h>
#include <llvm-c-15/llvm-c/BitWriter.h>
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <stdlib.h>
#include "codegen.h"
#include "token.h"
#include "parser.h"


#define MAXIMUM_MODULE_NAME_LENGTH 64


// Symbol Table
// Saves all local symbols in a linked list
struct LST {
    char *name;
    LLVMValueRef value;
    int level;
    struct LST *prev;
    struct LST *next;
} symbolTable;

void startSymbolTable() {
    symbolTable.name = NULL;
    symbolTable.value = NULL;
    symbolTable.level = 0;
    symbolTable.prev = NULL;
    symbolTable.next = (struct LST *) malloc(sizeof(struct LST));
    static int currentLevel = 0;
}

struct LST* getLastSymbol() {
    struct LST* currentSymbol = &symbolTable;
    while (currentSymbol->next != NULL) {
        currentSymbol = currentSymbol->next;
    }
    return currentSymbol;
}

void newSymbol(char *name, LLVMValueRef value) {
    static int currentLevel;

    struct LST currentSymbol = *getLastSymbol();
    currentSymbol.name = name;
    currentSymbol.value = value;
    currentSymbol.level = currentLevel;
    currentSymbol.next = (struct LST *) malloc(sizeof(struct LST));
    currentSymbol.next->prev = &currentSymbol;
}

LLVMValueRef getValueFromSymbolTable(char *name) {
    struct LST currentSymbol = *getLastSymbol();
    while (strcmp(currentSymbol.name, name) != 0) {
        currentSymbol = *currentSymbol.prev;
    }
    return currentSymbol.value;
}

void newLevel() {
    static int currentLevel;

    currentLevel++;
}

void deleteLevel() {
    static int currentLevel;

    if (currentLevel == 0) {
        printf("Error: Cannot delete level 0\n");
        exit(1);
    }

    struct LST currentSymbol = *getLastSymbol();
    currentLevel--;

    while (currentSymbol.level > currentLevel) {
        currentSymbol = *currentSymbol.prev;
        free(currentSymbol.next);
    }
}
// End Symbol Table


char* getModuleFileName(LLVMModuleRef mod) {
    char *name;
    size_t len = MAXIMUM_MODULE_NAME_LENGTH;
    name = LLVMGetModuleIdentifier(mod, &len);

    // Replace . by /
    for (int i = 0; i < strlen(name); i++) {
        if (name[i] == '.') {
            name[i] = '/';
        }
    }

    return name;
}

// Type checking functions
int isIntegerNumberType(LLVMTypeRef type) {
    return type == LLVMInt8Type()
        || type == LLVMInt16Type()
        || type == LLVMInt32Type()
        || type == LLVMInt64Type()
        || type == LLVMInt128Type();
}

int isFloatingPointNumberType(LLVMTypeRef type) {
    return type == LLVMHalfType()
        || type == LLVMFloatType()
        || type == LLVMDoubleType()
        || type == LLVMFP128Type()
        || type == LLVMX86FP80Type();
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
    LLVMValueRef function = LLVMAddGlobal(mod, function_type, name);
    printf("Function Created - %s()", name);
    return LLVMGetNamedGlobal(mod, name);
}

// Get global symbols (global variables, etc.)
void parseGlobalSymbols(LLVMModuleRef mod, node ast) {
    // Iterate over the root node's children
    for (int i = 0; i < ast.length; i++) {
        if (ast.children[i].data.type == DECLARATION) {
            short typeToken = ast.children[i].children[0].data.type;

            LLVMTypeRef type;
            switch (typeToken) {
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
            LLVMAddGlobal(mod, type, ast.children[i].data.text);
        }

    }
}



// Generate code for literals
LLVMValueRef generateLiteral(LLVMModuleRef mod, node ast, LLVMBuilderRef builder) {
    // Iterate over the root node's children
    for (int i = 0; i < ast.length; i++) {
        if (ast.children[i].data.type == INUM) {
            LLVMTypeRef type = LLVMInt32Type();
            return LLVMConstInt(type, atoi(ast.children[i].data.text), 0);
        } else if (ast.children[i].data.type == FNUM) {
            LLVMTypeRef type = LLVMDoubleType();
            return LLVMConstReal(type, atof(ast.children[i].data.text));
        } else if (ast.children[i].data.type == TRUE) {
            LLVMTypeRef type = LLVMInt1Type();
            return LLVMConstInt(type, 1, 0);
        } else if (ast.children[i].data.type == FALSE) {
            LLVMTypeRef type = LLVMInt1Type();
            return LLVMConstInt(type, 0, 0);
        } else if (ast.children[i].data.type == NLL) {
            LLVMTypeRef type = LLVMPointerType(LLVMInt8Type(), 0);
            return LLVMConstPointerNull(type);
        } else {

        }
    }
}

// Convert to boolean
LLVMValueRef convertToBoolean(LLVMModuleRef mod, LLVMValueRef value, LLVMBuilderRef builder) {
    // Verify if type is i1
    // If not convert to i1
    // The value should be 0 if value is 0 and 1 if value is not 0
    // First, verify is is a float and get the compare function
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
    // Iterate over the root node's children
    for (int i = 0; i < ast.length; i++) {
        LLVMValueRef RHS, LHS;
        // Verify for assignement or unary operation
        if (ast.children[i].data.type == ASSIGN) {
            newSymbol(ast.children[i].children[0].data.text, generateExpression(mod, ast.children[i].children[1], builder));
            continue;
        } if (ast.children[i].data.type == NOT) {
            return LLVMBuildNot(builder, convertToBoolean(mod, generateExpression(mod, ast.children[i].children[0], builder), builder), "nottmp");
        } else if (ast.children[i].data.type == BNOT) {
            return LLVMBuildNot(builder, generateExpression(mod, ast.children[i].children[0], builder), "bnottmp");
        } else if (ast.children[i].data.type >> 4 == 0x03) {
            // For literal
            return generateLiteral(mod, ast.children[i], builder);
        } else if (ast.children[i].data.type == ARRAY) {
            LLVMValueRef firstElement = generateExpression(mod, ast.children[i].children[0], builder);
            LLVMTypeRef elementType = LLVMTypeOf(firstElement);
            LLVMValueRef *values = (LLVMValueRef *) malloc(sizeof(LLVMValueRef) * ast.children[i].length);
            for (int j = 0; j < ast.children[i].length; j++) {
                values[j] = generateExpression(mod, ast.children[i].children[j], builder);
            }
            return LLVMConstArray(elementType, values, ast.children[i].length);
        } else if (ast.children[i].data.type == 0x60) {
            // For variable
            return getValueFromSymbolTable(ast.children[i].data.text);
        } else {
            // For binary operation
            LHS = generateExpression(mod, ast.children[i].children[0], builder);
            RHS = generateExpression(mod, ast.children[i].children[1], builder);
        }

        if (ast.children[i].data.type == PLUS) {
            return LLVMBuildAdd(builder, LHS, RHS, "addtmp");
        } else if (ast.children[i].data.type == MINUS) {
            return LLVMBuildSub(builder, LHS, RHS, "subtmp");
        } else if (ast.children[i].data.type == MULT) {
            return LLVMBuildMul(builder, LHS, RHS, "multmp");
        } else if (ast.children[i].data.type == DIV) {
            return LLVMBuildSDiv(builder, LHS, RHS, "divtmp");
        } else if (ast.children[i].data.type == MOD) {
            return LLVMBuildSRem(builder, LHS, RHS, "modtmp");
        } else if (ast.children[i].data.type == DIV) {
            return LLVMBuildSDiv(builder, LHS, RHS, "divtmp");
        } else if (ast.children[i].data.type == MOD) {
            return LLVMBuildSRem(builder, LHS, RHS, "modtmp");
        } else if (ast.children[i].data.type >= EQ && ast.children[i].data.type <= LTE) {
            LLVMIntPredicate pred;
            if (ast.children[i].data.type == EQ) {
                pred = LLVMIntEQ;
            } else if (ast.children[i].data.type == NEQ) {
                pred = LLVMIntNE;
            } else if (ast.children[i].data.type == GT) {
                pred = LLVMIntSGT;
            } else if (ast.children[i].data.type == GTE) {
                pred = LLVMIntSGE;
            } else if (ast.children[i].data.type == LT) {
                pred = LLVMIntSLT;
            } else if (ast.children[i].data.type == LTE) {
                pred = LLVMIntSLE;
            }
            return LLVMBuildICmp(builder, pred, LHS, RHS, "cmptmp");
        } else if (ast.children[i].data.type == BAND) {
            return LLVMBuildAnd(builder, LHS, RHS, "andtmp");
        } else if (ast.children[i].data.type == BOR) {
            return LLVMBuildOr(builder, LHS, RHS, "ortmp");
        } else if (ast.children[i].data.type == BXOR) {
            return LLVMBuildXor(builder, LHS, RHS, "xortmp");
        } else if (ast.children[i].data.type == BSL) {
            return LLVMBuildShl(builder, LHS, RHS, "shltmp");
        } else if (ast.children[i].data.type == BSR) {
            return LLVMBuildLShr(builder, LHS, RHS, "shrtemp");
        } else {
            // Logical operations
            LHS = convertToBoolean(mod, LHS, builder);
            RHS = convertToBoolean(mod, RHS, builder);

            if (ast.children[i].data.type == AND) {

            } else if (ast.children[i].data.type == OR) {
                // TODO
            } else if (ast.children[i].data.type == XOR) {
                // TODO
            }
        }
    }
    return 0;
}

// Generate code for statement
void generateStatement(LLVMModuleRef mod, node ast, LLVMBuilderRef builder) {
    // Iterate over the root node's children and search for statements
    for (int i = 0; i < ast.length; i++) {
        if (ast.children[i].data.type == IFSTATEMENT) {
            // Add a new basic block for the if statement
            LLVMBasicBlockRef ifBlock = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)), "if");
            LLVMBasicBlockRef elseBlock = LLVMAppendBasicBlock(LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder)), "else");

        } else if (ast.children[i].data.type == WHILELOOPSTATEMENT) {
            // TODO
        } else if (ast.children[i].data.type == DOWHILELOOPSTATEMENT) {
            // TODO
        } else if (ast.children[i].data.type == FORLOOPSTATEMENT) {
            // TODO
        } else if (ast.children[i].data.type == RETURN) {
            // TODO
        } else if (ast.children[i].data.type == BREAK) {
            // TODO
        } else if (ast.children[i].data.type == CONTINUE) {
            // TODO
        } else if (ast.children[i].data.type == EXPRESSION) {
            printf("Generating expression");
            generateExpression(mod, ast.children[i], builder);
        }
    }
}

// Generate code for main function
void generateMain(LLVMModuleRef mod, node ast, LLVMBuilderRef builder) {
    // Create a new function
    printf("Creating main() function\n");
    LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef sum = LLVMAddFunction(mod, "main", ret_type);
    printf("main() created\n");
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(sum, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);
    printf("Added basic block to sum\n");
    // Iterate over the root node's children and search for instruction
    for (int i = 0; i < ast.length; i++) {
        if (ast.children[i].data.type != DECLARATION) {
            generateStatement(mod, ast.children[i], builder);
        }
    }
}

void generateCode(char *name, node ast) {
    LLVMModuleRef mod = createModule(name);
    LLVMBuilderRef builder = LLVMCreateBuilder();

    startSymbolTable();


    // Verify if first node is the root node
    if (ast.data.type != ROOT) {
        fprintf(stderr, "error: root node not found\n");
        return;
    }

    parseGlobalSymbols(mod, ast);



    // Create main function
    generateMain(mod, ast, builder);

    if (LLVMWriteBitcodeToFile(mod, getModuleFileName(mod)) != 0) {
        fprintf(stderr, "error writing bitcode to file, skipping\n");
    }
}
