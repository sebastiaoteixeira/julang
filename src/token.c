#include "token.h"

int isAnUnaryOperator(Token c) {
    return c.type >> 4 == 7;
}

int isAnOperator(Token c) {
    return c.type >> 4 == 7 || c.type >> 4 == 8;
}

int isALitteral(Token c) {
    return c.type >> 4 == 3;
}