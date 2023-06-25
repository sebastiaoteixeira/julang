#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "token.h"
#include "expressionParser.h"
#include "parser.h"


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

node parseLiteralArray() {
    // Create a new node
    node array;
    array.data.type = ARRAY;
    array.children = (node *) malloc(sizeof(node));


    // An array starts with a left bracket
    if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == LSQBRACK) {
        tokenListManagerRef->index++;
    } else {
        printf("Error: Expected '[' at line %d\n", tokenListManagerRef->tokens[tokenListManagerRef->index].line);
        exit(1);
    }

    // Alternate between expressions and commas until a right bracket is found
    while (1) {
        // Parse expression
        addChild(array, getExpressionAST(0));

        if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == COMMA) {
            tokenListManagerRef->index++;
            continue;
        }
        
        if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == RSQBRACK) {
            tokenListManagerRef->index++;
            return array;
        }
        else {
            printf("Error: Expected ']' at line %d\n", tokenListManagerRef->tokens[tokenListManagerRef->index].line);
            exit(1);
        }
    }
}

node getParenthisedExpression() {
    // Verify if the expression is parenthised
    if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == LBRACK) {
        tokenListManagerRef->index++;
        node expression = getExpressionAST(0);
        if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == RBRACK) {
            tokenListManagerRef->index++;
            return expression;
        } else {
            printf("Error: Expected ')' at line %d\n", tokenListManagerRef->tokens[tokenListManagerRef->index].line);
            exit(1);
        }
    } else {
        printf("Error: Expected '(' at line %d\n", tokenListManagerRef->tokens[tokenListManagerRef->index].line);
        exit(1);
    }
}


node getOperand() {
    // Verify if the operand is a literal or a variable
    if (tokenListManagerRef->tokens[tokenListManagerRef->index].type & 0xf0 == 0x30
        || tokenListManagerRef->tokens[tokenListManagerRef->index].type == VAR) {
        node operand;
        operand.data = tokenListManagerRef->tokens[tokenListManagerRef->index];
        tokenListManagerRef->index++;
        return operand;
    }

    // Verify if the operand is a Literal Array
    else if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == LSQBRACK) {
        return parseLiteralArray();
    }

    // Verify if the operand is a parenthised expression
    else if (tokenListManagerRef->tokens[tokenListManagerRef->index].type == LBRACK) {
        return getParenthisedExpression();
    }

    else {
        printf("Error: Expected operand at line %d\n", tokenListManagerRef->tokens[tokenListManagerRef->index].line);
        exit(1);
    }
}

node getExpressionAST(int minPrecedence) {
    node childNode = getOperand();

    while(1) {
        // If the next token is not an operator, return the child node
        if (!isOperator(tokenListManagerRef->tokens[tokenListManagerRef->index])
            || getOperatorPrecedence(tokenListManagerRef->tokens[tokenListManagerRef->index]) < minPrecedence) {
            return childNode;
        }

        node rootOperation;
        addChild(rootOperation, childNode);

        // Get the operator
        rootOperation.data = tokenListManagerRef->tokens[tokenListManagerRef->index];
        tokenListManagerRef->index++;

        // Get the second operand
        addChild(rootOperation, getExpressionAST(getOperatorPrecedence(rootOperation.data)));

        childNode = rootOperation;
    }

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
