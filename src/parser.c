#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "parser.h"
#include "expressionParser.h"
#include "symbolsNaming.h"
#include "parserSymbols.h"
#include "logger.h"

void parseStatement(node* parent);
void parseBlock(node* parent);
void parseStatement(node* parent);
void parseBlock(node* parent);
TLM tokenListManager;

SymbolStack* symbolStack;

node* addNChildren(node* parent, int n)
{
    if (verbose >= 5) printf("Adding %d children to %x ...\n", n, parent->children);
    parent->length += n;
    if (parent->length > 1) {
        parent->children = (node *) realloc(parent->children, sizeof(node) * parent->length);
    }
    return parent->children + parent->length - n;
}


node* addChild(node* parent)
{
    return addNChildren(parent, 1);
}


void parseExpression(node* parent) {
    node* expression = addChild(parent);
    expression->data.type = EXPRESSION;
    expression->length = 1;
    expression->children = (node*) malloc(sizeof(node));

    expression->children[0] = getExpressionAST(0);

    return;
}


void parseDeclaration(node* parent) {
    node* declaration = addChild(parent);
    declaration->data.type = DECLARATION;
    declaration->length = 0;
    declaration->children = (node*) malloc(sizeof(node));

    // Verify declaration
    if (isAType(tokenListManager.tokens[tokenListManager.index])) {
        node* type = addChild(declaration);
        type->data = tokenListManager.tokens[tokenListManager.index];
        type->length = 0;
        type->children = (node*) malloc(sizeof(node));
        tokenListManager.index++;


        if (tokenListManager.tokens[tokenListManager.index].type == VAR) {
            node* symbol = addChild(declaration);
            symbol->data = tokenListManager.tokens[tokenListManager.index];
            symbol->length = 0;
            symbol->children = (node*) malloc(sizeof(node));
            tokenListManager.index++;
            if (tokenListManager.tokens[tokenListManager.index].type == ASSIGN)
                tokenListManager.index--;
        } else {
            printf("Error: Expected identifier at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
            exit(1);
        }
    }

    return;
}

void parseFunction(node* parent) {
    node* function = addChild(parent);
    function->data.type = FUNCTION;
    function->length = 0;
    function->children = (node*) malloc(sizeof(node));

    // Verify function
    if (isAType(tokenListManager.tokens[tokenListManager.index])) {
        node* symbol = addChild(function);
        symbol->data = tokenListManager.tokens[tokenListManager.index];
        symbol->length = 0;
        symbol->children = (node*) malloc(sizeof(node));
        tokenListManager.index++;
        if (tokenListManager.tokens[tokenListManager.index].type == VAR) {
            node* symbol = addChild(function);
            symbol->data = tokenListManager.tokens[tokenListManager.index];
            symbol->length = 0;
            symbol->children = (node*) malloc(sizeof(node));
            tokenListManager.index++;
            if (tokenListManager.tokens[tokenListManager.index].type == LBRACK) {
                while (tokenListManager.tokens[tokenListManager.index].type != RBRACK) {
                    tokenListManager.index++;
                    parseDeclaration(function);
                    if (tokenListManager.tokens[tokenListManager.index].type == VAR) {
                        parseExpression(function);
                    }
                    if (tokenListManager.tokens[tokenListManager.index].type == COMMA) {
                        continue;
                    } else if (tokenListManager.tokens[tokenListManager.index].type == RBRACK) {
                        break;
                    } else {
                        printf("Error: expected ',' or ')' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
                        exit(1);
                    }
                }
                pushFunction(symbolStack, function);
                tokenListManager.index++;
                parseStatement(function);
            } else {
                printf("Error: expected '(' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
                exit(1);
            }
        } else {
            printf("Error: expected identifier at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
            exit(1);
        }
    }

    return;
}

void parseImport(node* parent) {
    node* import = addChild(parent);
    import->data.type = IMPORT;
    import->length = 0;
    import->children = (node*) malloc(sizeof(node));

    // Verify import
    if (tokenListManager.tokens[tokenListManager.index].type == IMPORT) {
        tokenListManager.index++;
        parseExpression(import);

        pushImport(symbolStack, import);
    }

    return;
}

void parseIfStatement(node* parent) {
    node* ifElseStatement = addChild(parent);
    ifElseStatement->data.type = IFSTATEMENT;
    ifElseStatement->length = 0;
    ifElseStatement->children = (node*) malloc(sizeof(node));

    // Verify if statement
    if (tokenListManager.tokens[tokenListManager.index].type == IF) {
        tokenListManager.index++;
        parseExpression(ifElseStatement);
        parseStatement(ifElseStatement);
    }

    // Verify else statement
    if (tokenListManager.tokens[tokenListManager.index].type == ELSE) {
        tokenListManager.index++;
        parseStatement(ifElseStatement);
    }

    return;
    return;
}

void parseWhileLoop(node* parent) {
    node* whileLoop = addChild(parent);
    whileLoop->data.type = WHILELOOPSTATEMENT;
    whileLoop->length = 0;
    whileLoop->children = (node*) malloc(sizeof(node));

    // Verify while loop
    if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
        tokenListManager.index++;
        parseExpression(whileLoop);
        parseStatement(whileLoop);
    }

    return;
    return;
}

void parseDoWhileLoop(node* parent) {
    node* doWhileLoop = addChild(parent);
    doWhileLoop->data.type = DOWHILELOOPSTATEMENT;
    doWhileLoop->length = 0;
    doWhileLoop->children = (node*) malloc(sizeof(node));

    // Verify do while loop
    if (tokenListManager.tokens[tokenListManager.index].type == DO) {
        tokenListManager.index++;
        parseStatement(doWhileLoop);
        if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
            tokenListManager.index++;
            parseExpression(doWhileLoop);
        } else {
            printf("Error: expected 'while' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
            exit(1);
        }
    }
}

void parseForLoop(node* parent) {
    node* forLoopStatement = addChild(parent);
    forLoopStatement->data.type = FORLOOPSTATEMENT;
    forLoopStatement->length = 0;
    forLoopStatement->children = (node*) malloc(sizeof(node));

    // Verify for loop
    if (tokenListManager.tokens[tokenListManager.index].type == FOR) {
        tokenListManager.index++;
        if (tokenListManager.tokens[tokenListManager.index].type == LBRACK) {
            tokenListManager.index++;
            parseStatement(forLoopStatement);
            parseExpression(forLoopStatement);
            if (tokenListManager.tokens[tokenListManager.index].type == SEMICOLON) {
                tokenListManager.index++;
            } else {
                printf("Error: expected ';' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
                exit(1);
            }
            parseStatement(forLoopStatement);
            parseExpression(forLoopStatement);
            if (tokenListManager.tokens[tokenListManager.index].type == SEMICOLON) {
                tokenListManager.index++;
            } else {
                printf("Error: expected ';' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
                exit(1);
            }
            parseStatement(forLoopStatement);
            if (tokenListManager.tokens[tokenListManager.index].type == RBRACK) {
                tokenListManager.index++;
                parseStatement(forLoopStatement);
            } else {
                printf("Error: expected ')' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
                exit(1);
            }
        } else {
            printf("Error: expected '(' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
            exit(1);
        }
    }

}

void parseBreakStatement(node* parent) {
    node* breakStatement = addChild(parent);
    breakStatement->data.type = BREAK;
    breakStatement->length = 0;

    // Verify break statement
    if (tokenListManager.tokens[tokenListManager.index].type == BREAK) {
        tokenListManager.index++;
    } else {
        printf("Error: expected 'break' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }
}

void parseContinueStatement(node* parent) {
    node* continueStatement = addChild(parent);
    continueStatement->data.type = CONTINUE;
    continueStatement->length = 0;

    // Verify continue statement
    if (tokenListManager.tokens[tokenListManager.index].type == CONTINUE) {
        tokenListManager.index++;
    } else {
        printf("Error: expected 'continue' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }

    return;
}

void parseReturnStatement(node* parent) {
    node* returnStatement = addChild(parent);
    returnStatement->data.type = RETURN;
    returnStatement->length = 0;
    returnStatement->children = (node*) malloc(sizeof(node));

    // Verify return statement
    if (tokenListManager.tokens[tokenListManager.index].type == RETURN) {
        tokenListManager.index++;
        parseExpression(returnStatement);
    }
}

void parseStatement(node* parent)
{
    // Verify block
    if (tokenListManager.tokens[tokenListManager.index].type == LBRACE) {
        parseBlock(parent);
    }

    // Verify declaration
    else if (isAType(tokenListManager.tokens[tokenListManager.index])) {
        if (tokenListManager.tokens[tokenListManager.index + 2].type == LBRACK) {
            printf("Function\n");
            parseFunction(parent);
        } else {
            printf("Declaration\n");
            parseDeclaration(parent);
        }
    }

    // Verify while loop
    else if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
        printf("While\n");
        parseWhileLoop(parent);
    }

    // Verify do while loop
    else if (tokenListManager.tokens[tokenListManager.index].type == DO) {
        printf("Do while\n");
        parseDoWhileLoop(parent);
    }

    // Verify for loop
    else if (tokenListManager.tokens[tokenListManager.index].type == FOR) {
        printf("For\n");
        parseForLoop(parent);
    }

    // Verify if statement
    else if (tokenListManager.tokens[tokenListManager.index].type == IF) {
        printf("If\n");
        parseIfStatement(parent);
    }

    // Verify return statement
    else if (tokenListManager.tokens[tokenListManager.index].type == RETURN) {
        printf("Return\n");
        parseReturnStatement(parent);
    }

    // Verify import statement
    else if (tokenListManager.tokens[tokenListManager.index].type == IMPORT) {
        printf("Import\n");
        parseImport(parent);
    }

    // Verify break statement
    else if (tokenListManager.tokens[tokenListManager.index].type == BREAK) {
        printf("Break\n");
        parseBreakStatement(parent);
    }

    // Verify continue statement
    else if (tokenListManager.tokens[tokenListManager.index].type == CONTINUE) {
        printf("Continue\n");
        parseContinueStatement(parent);
    }

    // Verify expression
    else if (tokenListManager.tokens[tokenListManager.index].type == VAR
            || isAnOperator(tokenListManager.tokens[tokenListManager.index]) 
            || isALiteral(tokenListManager.tokens[tokenListManager.index])
            || tokenListManager.tokens[tokenListManager.index].type == LBRACK) {
        printf("Expression\n");
        parseExpression(parent);
    }

    // Verify if is the End Of File
    else if (tokenListManager.tokens[tokenListManager.index].type == EOF) {
        printf("EOF\n");
        node* eofNode = addChild(parent);
        eofNode->data.type = EOF;
        eofNode->length = 0;
    }

    // Verify if is the end of a block
    else if (tokenListManager.tokens[tokenListManager.index].type == RBRACE) {
        printf("End of block\n");
        node* eofNode = addChild(parent);
        eofNode->data.type = EOF;
        eofNode->length = 0;
    }

    else {
        printf("Error: unknown token at line %d: %s\n", tokenListManager.tokens[tokenListManager.index].line, tokenListManager.tokens[tokenListManager.index].text);
        exit(1);
    }

    // Verify semicolon
    if (tokenListManager.tokens[tokenListManager.index].type == SEMICOLON) {
        tokenListManager.index++;
    }

    return;
}

void parseStatementList(node* parent)
{
    while (1) {
        parseStatement(parent);
        if (parent->children[parent->length - 1].data.type == EOF) break;
    }
}

void parseBlock(node* parent)
{
    node* block = addChild(parent);
    block->data.type = BLOCK;
    block->length = 0;
    block->children = (node*) malloc(sizeof(node));


    if (tokenListManager.tokens[tokenListManager.index].type != LBRACE) {
        printf("Error: Expected '{' at line %d\n",
            tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }
    tokenListManager.index++;

    parseStatementList(block);

    if (tokenListManager.tokens[tokenListManager.index].type != RBRACE) {
        printf("Error: Expected '}' at line %d\n",
            tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }
    tokenListManager.index++;

    return;
}

void parseProgram(node* rootRef)
{
    parseStatementList(rootRef);

    if (tokenListManager.tokens[tokenListManager.index].type == RBRACE) {
        printf("Error: Unexpected '}' at line %d\n",
            tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }

    return;
}

node runParser(SymbolStack *_symbolStack, Token *tokenList)
{
    symbolStack = _symbolStack;
    initExpressionParser((TLM *) &tokenListManager, symbolStack);

    tokenListManager.tokens = tokenList;
    tokenListManager.index = 0;

    node ASTRoot;
    ASTRoot.data.type = ROOT;
    printf("%d\n", sizeof(node));
    ASTRoot.length = 0;
    ASTRoot.children = (node*) malloc(sizeof(node));

    parseProgram(&ASTRoot);

    return ASTRoot;
}

/*
void iterateOverAST(node ast, void (*callback)(node, int), int depth)
{
    callback(ast, depth);
    for (int i = 0; i < ast.length; i++) {
        iterateOverAST(ast.children[i], callback, depth + 1);
    }
    return;
}
*/
