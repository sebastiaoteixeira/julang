#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "expressionParser.h"
#include "parser.h"

void initExpressionParser() {
    Operators = malloc(sizeof(char) * 18);
    for (int i = 0; i < 18; i++) {
        Operators[i].op = malloc(sizeof(char) * 1);
        Operators[i].precedence = i;
        for (int j = 0; PRIORITY_LEVELS[i][j] != '\\'; j++) {
            Operators[i].op = realloc(Operators[i].op, sizeof(char) * j + 2);
            Operators[i].op[j] = PRIORITY_LEVELS[i][j];
            Operators[i].op[j+1] = '\0';
        }

    }
    return;
}

int isOperator(char *op) {
    for (int i = 0; i < 18; i++) {
        if (strcmp(Operators[i].op, op) == 0) {
            return 1;
        }
    }
    return 0;
}

int getOperatorPrecedence(char *op) {
    for (int i = 0; i < 18; i++) {
        if (strcmp(Operators[i].op, op) == 0) {
            return Operators[i].precedence;
        }
    }
    return -1;
}

node getExpressionAST() {
    node rootOperation;

    // Verify parenthized expression
    if (tokenListManager.tokens[tokenListManager.index].type == LBRACK) {
        tokenListManager.index++;
        rootOperation = getExpressionAST();
        if (tokenListManager.tokens[tokenListManager.index].type == RBRACK) {
            tokenListManager.index++;
        } else {
            exit(1);
            printf("Error: expected ')' at line %d\n", tokenListManager.tokens[tokenListManager.index].line);
        }
    }

    // Verify unary operation
    else if (isAnOperator(tokenListManager.tokens[tokenListManager.index])) {
        
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