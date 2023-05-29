#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "expressionParser.h"
#include "parser.h"


char *PRIORITY_LEVELS[] = {"||\\",
                          "&&\\",
                          "|\\",
                          "^\\",
                          "&\\",
                          "==\\!=\\",
                          "<\\>\\<=\\>=\\",
                          "<<\\>>\\",
                          "+\\-\\",
                          "*\\/\\%\\"};

Operator* operators;

TLM* tokenListManagerRef;

void initExpressionParser(TLM* _tokenListManagerRef) {
    tokenListManagerRef = _tokenListManagerRef;
    operators = malloc(sizeof(char) * 18);
    unsigned char n = 0;
    unsigned char k;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; PRIORITY_LEVELS[i][j] != '\0'; j++) {
            k = 0;
            operators[n].op = malloc(sizeof(char) * 1);
            operators[n].precedence = i;
            while (PRIORITY_LEVELS[i][j] != '\\') {
                operators[n].op = realloc(operators[n].op, sizeof(char) * j + 2);
                operators[n].op[k] = PRIORITY_LEVELS[i][j];
                operators[n].op[k+1] = '\0';
                j++;
                k++;
            }
            n++;
        }
    }
    return;
}

int isOperator(char *op) {
    for (int i = 0; i < 18; i++) {
        if (strcmp(operators[i].op, op) == 0) {
            return 1;
        }
    }
    return 0;
}

int getOperatorPrecedence(char *op) {
    for (int i = 0; i < 18; i++) {
        if (strcmp(operators[i].op, op) == 0) {
            return operators[i].precedence;
        }
    }
    return -1;
}

ExpressionToken* getExpressionTokens() {
    ExpressionToken* tokenList;
    int index = 0;
    int parenthesisCount = 0;

    while(1) {
        // Count parenthesis
        if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == LBRACK) {
            parenthesisCount++;
        } else if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == RBRACK) {
            parenthesisCount--;
        }

        // Verify if expression is finished
        if (// Verify parenthesis count
            parenthesisCount == -1 || 
            // Verify if is a semicolon
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == SEMICOLON ||
            // Verify if is a comma
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == COMMA ||
            // Verify if is the end of a block
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == RBRACE ||
            // Verify if is the end of the file
            tokenListManagerRef->tokens[tokenListManagerRef->index].type == EOF ||
            // Verify if there are two non-operators in sequence
            (index == 0 ? 0 : 
            !isAnOperator(tokenListManagerRef->tokens[tokenListManagerRef->index]) &&
            !isAnOperator(tokenListManagerRef->tokens[tokenListManagerRef->index]))
            ) break;
        



        index++;
    }
}

node getExpressionAST() {
    node rootOperation;


    // Verify parenthized expression
    if ((*tokenListManagerRef).tokens[(*tokenListManagerRef).index].type == LBRACK) {
        (*tokenListManagerRef).index++;
        rootOperation = getExpressionAST();
        if ((*tokenListManagerRef).tokens[(*tokenListManagerRef).index].type == RBRACK) {
            (*tokenListManagerRef).index++;
        } else {
            printf("Error: expected ')' at line %d\n", (*tokenListManagerRef).tokens[(*tokenListManagerRef).index].line);
            //exit(1);
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
