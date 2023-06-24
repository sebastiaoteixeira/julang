#include <stdlib.h>
#include <stdio.h>
#include "token.h"
#include "parser.h"
#include "expressionParser.h"

node parseStatement();
node parseBlock();
TLM tokenListManager;

void addChild(node parent, node child)
{
    parent.length++;
    if (parent.length > 1)
        parent.children = (node *) realloc(parent.children, sizeof(node) * parent.length);
    parent.children[parent.length - 1] = child;
}

void addNChildren(node parent, node* children, int n)
{

    printf("Adding %d children from %x to %x ...\n", n, children, parent.children);
    parent.length += n;
    if (parent.length > 1) {
        printf("Increasing capacity of node %x for %d children = %d bytes\n", parent.children, parent.length, sizeof(node) * parent.length);
        parent.children = (node *) realloc(parent.children, sizeof(node) * parent.length);
    }
    for (int i = parent.length - n; i < parent.length; i++)
        parent.children[i] = children[i - parent.length];
}

node parseExpression() {
    node expression;
    expression.data.type = EXPRESSION;
    expression.length = 0;
    expression.children = (node*) malloc(sizeof(node));

    node expressionRootOperation = getExpressionAST();
    addChild(expression, expressionRootOperation);

    return expression;
}

node parseDeclaration() {
    node declaration;
    declaration.data.type = DECLARATION;
    declaration.length = 0;
    declaration.children = (node*) malloc(sizeof(node));

    // Verify declaration
    if (isAType(tokenListManager.tokens[tokenListManager.index])) {
        tokenListManager.index++;
        if (tokenListManager.tokens[tokenListManager.index].type == VAR) {
            tokenListManager.index++;
            if (tokenListManager.tokens[tokenListManager.index].type == ASSIGN) {
                tokenListManager.index--;
                addChild(declaration, parseExpression());
            }
        }
    }

    return declaration;
}

node parseIfStatement() {
    node ifElseStatement;
    ifElseStatement.data.type = IFSTATEMENT;
    ifElseStatement.length = 0;
    ifElseStatement.children = (node*) malloc(sizeof(node));

    // Verify if statement
    if (tokenListManager.tokens[tokenListManager.index].type == IF) {
        tokenListManager.index++;
        addChild(ifElseStatement, parseExpression());
        addChild(ifElseStatement, parseStatement());
    }

    // Verify else statement
    if (tokenListManager.tokens[tokenListManager.index].type == ELSE) {
        tokenListManager.index++;
        addChild(ifElseStatement, parseStatement());
    }

    return ifElseStatement;
}

node parseWhileLoop() {
    node whileLoop;
    whileLoop.data.type = WHILELOOPSTATEMENT;
    whileLoop.length = 0;
    whileLoop.children = (node*) malloc(sizeof(node));

    // Verify while loop
    if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
        tokenListManager.index++;
        addChild(whileLoop, parseExpression());
        addChild(whileLoop, parseStatement());
    }

    return whileLoop;
}

node parseDoWhileLoop() {
    node doWhileLoop;
    doWhileLoop.data.type = DOWHILELOOPSTATEMENT;
    doWhileLoop.length = 0;
    doWhileLoop.children = (node*) malloc(sizeof(node));

    // Verify do while loop
    if (tokenListManager.tokens[tokenListManager.index].type == DO) {
        tokenListManager.index++;
        addChild(doWhileLoop, parseStatement());
        if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
            tokenListManager.index++;
            addChild(doWhileLoop, parseExpression());
        } else {
            exit(1);
            printf("Error: expected 'while' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
        }
    }

    return doWhileLoop;
}

node parseForLoop() {
    node forLoopStatement;
    forLoopStatement.data.type = FORLOOPSTATEMENT;
    forLoopStatement.length = 0;
    forLoopStatement.children = (node*) malloc(sizeof(node));

    // Verify for loop
    if (tokenListManager.tokens[tokenListManager.index].type == FOR) {
        tokenListManager.index++;
        if (tokenListManager.tokens[tokenListManager.index].type == LBRACK) {
            tokenListManager.index++;
            addChild(forLoopStatement, parseStatement());
            addChild(forLoopStatement, parseExpression());
            addChild(forLoopStatement, parseStatement());
            if (tokenListManager.tokens[tokenListManager.index].type == RBRACK) {
                tokenListManager.index++;
                addChild(forLoopStatement, parseStatement());
            } else {
                exit(1);
                printf("Error: expected ')' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
            }
        } else {
            exit(1);
            printf("Error: expected '(' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
        }
    }

    return forLoopStatement;
}

node parseBreakStatement() {
    node breakStatement;
    breakStatement.data.type = BREAK;
    breakStatement.length = 0;

    // Verify break statement
    if (tokenListManager.tokens[tokenListManager.index].type == BREAK) {
        tokenListManager.index++;
    }

    return breakStatement;
}

node parseContinueStatement() {
    node continueStatement;
    continueStatement.data.type = CONTINUE;
    continueStatement.length = 0;

    // Verify continue statement
    if (tokenListManager.tokens[tokenListManager.index].type == CONTINUE) {
        tokenListManager.index++;
    }

    return continueStatement;
}

node parseStatement()
{
    node statement;
    statement.data.type = STATEMENT;
    statement.length = 0;
    statement.children = (node*) malloc(sizeof(node));
    printf("Statement at line %d: %s\n", tokenListManager.tokens[tokenListManager.index].line, tokenListManager.tokens[tokenListManager.index].text);

    // Verify block
    if (tokenListManager.tokens[tokenListManager.index].type == LBRACE) {
        printf("Block\n");
        addChild(statement, parseBlock());
        return statement;
    }

    // Verify declaration
    else if (isAType(tokenListManager.tokens[tokenListManager.index])) {
        printf("Declaration\n");
        addChild(statement, parseDeclaration());
    }

    // Verify while loop
    else if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
        printf("While\n");
        addChild(statement, parseWhileLoop());
    }

    // Verify do while loop
    else if (tokenListManager.tokens[tokenListManager.index].type == DO) {
        printf("Do while\n");
        addChild(statement, parseDoWhileLoop());
    }

    // Verify for loop
    else if (tokenListManager.tokens[tokenListManager.index].type == FOR) {
        printf("For\n");
        addChild(statement, parseForLoop());
    }

    // Verify if statement
    else if (tokenListManager.tokens[tokenListManager.index].type == IF) {
        printf("If\n");
        addChild(statement, parseIfStatement());
    }

    /* TODO: Verify return statement
    if (tokenListManager.tokens[tokenListManager.index].type == RETURN) {
        addChild(statement, parseReturnStatement());
    }
    */

    // Verify break statement
    else if (tokenListManager.tokens[tokenListManager.index].type == BREAK) {
        printf("Break\n");
        addChild(statement, parseBreakStatement());
    }

    // Verify continue statement
    else if (tokenListManager.tokens[tokenListManager.index].type == CONTINUE) {
        printf("Continue\n");
        addChild(statement, parseContinueStatement());
    }

    // Verify expression
    else if (tokenListManager.tokens[tokenListManager.index].type == VAR || isAnOperator(tokenListManager.tokens[tokenListManager.index]) || isALiteral(tokenListManager.tokens[tokenListManager.index])) {
        printf("Expression\n");
        addChild(statement, parseExpression());
    }

    // Verify if is the End Of File
    else if (tokenListManager.tokens[tokenListManager.index].type == EOF) {
        printf("EOF\n");
        statement.data.type = EOF;
    }

    // Verify if is the end of a block
    else if (tokenListManager.tokens[tokenListManager.index].type == RBRACE) {
        printf("End of block\n");
        statement.data.type = EOF;
    }

    else {
        printf("Error: unknown token at line %d: %s\n", tokenListManager.tokens[tokenListManager.index].line, tokenListManager.tokens[tokenListManager.index].text);
        exit(1);
    }

    // Verify semicolon
    if (tokenListManager.tokens[tokenListManager.index].type == SEMICOLON) {
        tokenListManager.index++;
    }

    return statement;
}

node* parseStatementList()
{
    node* statementList = (node *) malloc(2 * sizeof(node));
    printf("New statementList address %x\n", statementList);
    int length = 0;
    int capacity = 2;
    while (1) {
        if (++length > capacity) {
            capacity <<= 1;
            printf("Allocating to statementList (address %x) %d nodes (%d bytes per node) = %d bytes\n", statementList, capacity, sizeof(node), capacity * sizeof(node));
            statementList = (node *) realloc(statementList, sizeof(node) * capacity);
            printf("New address %x\n", statementList);

        }
        statementList[length - 1] = parseStatement();
        if (statementList[length - 1].data.type == EOF) break;
    }
    return statementList;
}

node parseBlock()
{
    node block;
    block.data.type = BLOCK;
    block.length = 0;
    block.children = (node*) malloc(sizeof(node));


    if (tokenListManager.tokens[tokenListManager.index].type != LBRACE) {
        printf("Error: Expected '{' at line %d\n",
            tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }
    tokenListManager.index++;

    node* statementList = parseStatementList();

    int len = 0;
    for (int i = 0; statementList[i].data.type != EOF; i++) {
        len++;
    }
    addNChildren(block, statementList, len);
    printf("Destroy statementList %x\n", statementList);
    free(statementList);

    if (tokenListManager.tokens[tokenListManager.index].type != RBRACE) {
        printf("Error: Expected '}' at line %d\n",
            tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }
    tokenListManager.index++;

    return block;
}

void parseProgram(node root)
{
    node* statementList = parseStatementList();

    if (tokenListManager.tokens[tokenListManager.index].type == RBRACE) {
        printf("Error: Unexpected '}' at line %d\n",
            tokenListManager.tokens[tokenListManager.index].line);
        exit(1);
    }

    int len = 0;
    for (int i = 0; statementList[i].data.type != EOF; i++) {
        len++;
    }
    addNChildren(root, statementList, len);

    return;
}

node runParser(Token *tokenList)
{
    //initExpressionParser((TLM *) &tokenListManager);

    tokenListManager.tokens = tokenList;
    tokenListManager.index = 0;

    node ASTRoot;
    ASTRoot.data.type = ROOT;
    printf("%d\n", sizeof(node));
    ASTRoot.children = (node*) malloc(sizeof(node));

    parseProgram(ASTRoot);

    return ASTRoot;
}
