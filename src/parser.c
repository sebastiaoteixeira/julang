#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "expressionParser.h"


void addChild(node parent, node child)
{
    parent.length++;
    parent.children = realloc(parent.children, sizeof(node) * parent.length);
    parent.children[parent.length - 1] = child;
}

node parseExpression() {
    node expression;
    expression.data.type = EXPRESSION;
    expression.length = 0;
    expression.children = NULL;

    node expressionRootOperation = getExpressionAST();
    addChild(expression, expressionRootOperation);

    return expression;
}

node parseIfStatement() {
    node ifElseStatement;
    ifElseStatement.data.type = IFSTATEMENT;
    ifElseStatement.length = 0;
    ifElseStatement.children = NULL;

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

node parseStatement() 
{
    node statement;
    statement.data.type = STATEMENT;
    statement.length = 0;
    statement.children = NULL;

    // Verify block
    if (tokenListManager.tokens[tokenListManager.index].type == LBRACE) {
        addChild(statement, parseBlock());
        return statement;
    }

    // Verify declaration
    if (tokenListManager.tokens[tokenListManager.index].type == INT || tokenListManager.tokens[tokenListManager.index].type == LONG || tokenListManager.tokens[tokenListManager.index].type == CHAR || tokenListManager.tokens[tokenListManager.index].type == FLOAT || tokenListManager.tokens[tokenListManager.index].type == DOUBLE || tokenListManager.tokens[tokenListManager.index].type == BOOL) {
        addChild(statement, parseDeclaration());
    }

    // Verify assignment
    if (tokenListManager.tokens[tokenListManager.index].type == VAR) {
        if (tokenListManager.tokens[tokenListManager.index].type == ASSIGN) {
            addChild(statement, parseAssignment());

        }
    }

    // Verify while loop
    if (tokenListManager.tokens[tokenListManager.index].type == WHILE) {
        addChild(statement, parseWhileLoop());
    }

    // Verify do while loop
    if (tokenListManager.tokens[tokenListManager.index].type == DO) {
        addChild(statement, parseDoWhileLoop());
    }

    // Verify for loop
    if (tokenListManager.tokens[tokenListManager.index].type == FOR) {
        addChild(statement, parseForLoop());
    }

    // Verify if statement
    if (tokenListManager.tokens[tokenListManager.index].type == IF) {
        addChild(statement, parseIfStatement());
    }

    // Verify return statement
    if (tokenListManager.tokens[tokenListManager.index].type == RETURN) {
        addChild(statement, parseReturnStatement());
    }

    // Verify break statement
    if (tokenListManager.tokens[tokenListManager.index].type == BREAK) {
        addChild(statement, parseBreakStatement());
    }

    // Verify continue statement
    if (tokenListManager.tokens[tokenListManager.index].type == CONTINUE) {
        addChild(statement, parseContinueStatement());
    }

    // Verify expression
    if (tokenListManager.tokens[tokenListManager.index].type == VAR || isAnOperator(tokenListManager.tokens[tokenListManager.index]) || isALiteral(tokenListManager.tokens[tokenListManager.index])) {
        addChild(statement, parseExpression());
    }

    // Verify semicolon
    if (tokenListManager.tokens[tokenListManager.index].type == SEMICOLON) {
        tokenListManager.index++;
    }

    return statement;
}

node* parseStatementList()
{
    node* statementList = malloc(sizeof(node));
    int length = 0;
    while (1) {
        length++;
        statementList = realloc(statementList, sizeof(node) * length);
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
    block.children = NULL;

    /*
    TODO: Add check for '{'
    if (tokenList[0].type != LBRACE) {
        printf("Error: Expected '{' at line %d\n", tokenList[0].line);
        exit(1);
    }
    */
   tokenListManager.index++;

    node* statmentList = parseStatementList();
    for (int i = 0; i < statmentList[i].data.type != EOF; i++) {
        addChild(block, statmentList[i]);
    }

    /*
    TODO: Add check for '}'
    if (tokenList[0].type != RBRACE) {
        printf("Error: Expected '}' at line %d\n", tokenList[0].line);
        exit(1);
    }
    */
    tokenListManager.index++;

    return block;
}

node parseProgram()
{
    node program;
    program.data.type = PROGRAM;
    program.length = 0;
    program.children = NULL;

    node* statmentList = parseStatementList();
    for (int i = 0; i < statmentList[i].data.type != EOF; i++) {
        addChild(program, statmentList[i]);
    }

    return program;
}

node runParser(Token *tokenList)
{
    tokenListManager.tokenList = tokenList;
    tokenListManager.index = 0;

    node ASTRoot;
    ASTRoot.data.type = ROOT;
    addChild(ASTRoot, parseProgram());
    return ASTRoot;
}
