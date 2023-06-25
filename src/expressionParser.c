#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "expressionParser.h"
#include "parser.h"



const int OPERATORS_COUNT = 19;
const char PRIORITY_LEVELS[11][4] = {{ASSIGN, 0, 0, 0},
                                     {OR, 0, 0, 0},
                                     {AND, 0, 0, 0},
                                     {BOR, 0, 0, 0},
                                     {BXOR, 0, 0, 0},
                                     {BAND, 0, 0, 0},
                                     {EQ, NEQ, 0, 0},
                                     {GT, LT, GTE, LTE},
                                     {BSL, BSR, 0, 0},
                                     {PLUS, MINUS, 0, 0},
                                     {MULT, DIV, MOD, 0}};


Operator* operators;
TLM* tokenListManagerRef;


void initExpressionParser(TLM* _tokenListManagerRef) {
    tokenListManagerRef = _tokenListManagerRef;
    operators = (Operator *) malloc(sizeof(Operator) * OPERATORS_COUNT);
    int n = 0;
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 4; j++) {
            if (PRIORITY_LEVELS[i][j] == 0) continue;
            printf("Setting operator %x\n", PRIORITY_LEVELS[i][j]);
            operators[n].op = PRIORITY_LEVELS[i][j];
            operators[n].precedence = i;
            n++;
        }
    }
    return;
}

int isOperator(Token op) {
    for (int i = 0; i < OPERATORS_COUNT; i++) {
        if (operators[i].op == op.type) return 1;
    }
    return 0;
}

int getOperatorPrecedence(Token op) {
    for (int i = 0; i < OPERATORS_COUNT; i++) {
        if (operators[i].op == op.type) return operators[i].precedence;
    }
    return -1;
}

int getExpressionLength() {
    int parenthesisCount = 0;
    int index = 0;

    while(1) {
        // Verify if expression is finished
        if (// Verify parenthesis count
            parenthesisCount == -1 || (parenthesisCount == 0 &&
            // Verify if is a semicolon
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == SEMICOLON ||
            // Verify if is a comma
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == COMMA ||
            // Verify if is the start of a block
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == LBRACE ||
            // Verify if is the end of a block
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == RBRACE ||
            // Verify if is the end of the file
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == EOF ||
            // Verify if there are two non-operators in sequence
            (index == 0 ? 0 :
            !isAnOperator(tokenListManagerRef->tokens[tokenListManagerRef->index - 1]) &&
            !isAnOperator(tokenListManagerRef->tokens[tokenListManagerRef->index]))
        )) {
            tokenListManagerRef->index -= index;
            return index;
            }

        // Count parenthesis
        if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == LBRACK) {
            parenthesisCount++;
        } else if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == RBRACK) {
            parenthesisCount--;
        }

        tokenListManagerRef->index++;
        index++;
    }
}

/*
// Verify if is an operator
        if (isAnOperator(tokenListManagerRef->tokens[tokenListManagerRef->index])) {
            // Verify if is a unary operator
            if (isAnUnaryOperator(tokenListManagerRef->tokens[tokenListManagerRef->index]) && (index == 0 || isAnOperator(tokenListManagerRef->tokens[tokenListManagerRef->index - 1] || tokenListManagerRef->tokens[tokenListManagerRef->index - 1].type == LBRACK))) {
                tokenList[index].token = tokenListManagerRef->tokens[tokenListManagerRef->index];
                tokenList[index].unary = 1;
                tokenList[index].precedence = getOperatorPrecedence(tokenListManagerRef->tokens[tokenListManagerRef->index].text) + getOperatorPrecedence(tokenListManagerRef->tokens[tokenListManagerRef->index].text) *
            } else {

                tokenList[index].
            }
            tokenList[index].precedence += 10 * parenthesisCount;
        } else {
            tokenList[index].literal = 1;
            tokenList[index].value = tokenListManagerRef->tokens[tokenListManagerRef->index].value;
            index++;
        }
*/

node getExpressionAST() {
    node rootOperation;

    // Get expression length
    int expressionLength = getExpressionLength();


    // Verify parenthized expression
    if ((*tokenListManagerRef).tokens[(*tokenListManagerRef).index].type == LBRACK) {
        (*tokenListManagerRef).index++;
        rootOperation = getExpressionAST();
        if ((*tokenListManagerRef).tokens[(*tokenListManagerRef).index].type == RBRACK) {
            (*tokenListManagerRef).index++;
        } else {
            printf("Error: expected ')' at line %d\n", (*tokenListManagerRef).tokens[(*tokenListManagerRef).index].line);
            exit(1);
        }
    }

    // Verify unary operation
    else if (isAnUnaryOperator((*tokenListManagerRef).tokens[(*tokenListManagerRef).index])) {
        // TODO: Verify unary operation
    }

    return rootOperation;
}

/*def calcLessPriotity(expr):
    PRIORITY_LEVELS = {'+': 1, '-': 1, '*': 2, '/': 2, '^': 3}
    OPERATIONS = {'+': soma, '-': sub, '*': mult, '/': div, '^': pot}
    if expr.count('.') <= 1 and expr.replace('.', '').isdigit():
        return expr

    minPriorityIndex = 0
    minPriority = len(expr) * 3

    parenteses = 0

    calcPriority = lambda c: PRIORITY_LEVELS[c] + parenteses * 3 if PRIORITY_LEVELS.get(c) else minPriority

    for i in range(len(expr)):
        char = expr[i]
        priority = calcPriority(char)
        if priority < minPriority:
            minPriorityIndex = i
            minPriority = priority
        if char == '(':
            parenteses += 1
        elif char == ')':
            parenteses -= 1

    expr1 = expr[:minPriorityIndex]
    expr2 = expr[minPriorityIndex + 1:]

    if expr1[0] == '(' and expr1[-1] == ')':
        expr1 = expr1[1:-1]
    if expr2[0] == '(' and expr2[-1] == ')':
        expr2 = expr2[1:-1]

    return OPERATIONS[expr[minPriorityIndex]](expr1, expr2)

def soma(a, b):
    return float(calcLessPriotity(a)) + float(calcLessPriotity(b))

def sub(a, b):
    return float(calcLessPriotity(a)) - float(calcLessPriotity(b))

def mult(a, b):
    return float(calcLessPriotity(a)) * float(calcLessPriotity(b))

def div(a, b):
    return float(calcLessPriotity(a)) / float(calcLessPriotity(b))

def pot(a, b):
    return float(calcLessPriotity(a)) ** float(calcLessPriotity(b))


def main():
    while True:
        expression = input('Insert expression: ').replace(' ', '')
        print(calcLessPriotity(expression), end='\n\n')

if __name__ == '__main__':
    main()
*/
