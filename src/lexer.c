#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "lexer.h"

unsigned int linecount = 1;

int isLetter(char c)
{
    return (c>='a' && c<='z') || (c>='A' && c<='Z');
}

int isWordChar(char c)
{
    return isLetter(c) || c == "_";
}

int isDigit(char c)
{
    return c>='0' && c<='9';
}

void printTokenList(Token *tokenList)
{
    int i = 0;
    while(tokenList[i].type != EOF) {
        printf("%s\t", tokenList[i].text);
        printf("%d\n", tokenList[i].type);
        i++;
    }
}

char *charToString(char chr)
{
    char* chrstr = malloc(sizeof(char) * 2);
    *chrstr = chr;
    *(chrstr + 1) = '\0';
    return chrstr;
}

char *stringReader(FILE *iCode)
{
    fseek(iCode, -1, SEEK_CUR);
    char c = -1;
    char stringDelimitter = fgetc(iCode);
    char* string = (char *) malloc(sizeof(char) * 1);

    int length = 0;
    while(c != '\0') {
        c = fgetc(iCode);
        if (c == stringDelimitter) c = '\0';
        length++;
        string = (char *) realloc(string, sizeof(char) * length);
        *(string + length - 1) = c;
    }
    length++;
    string = (char *) realloc(string, sizeof(char) * length);
    *(string + length - 1) = '\0';

    return string;
}

Token numberReader(FILE* iCode)
{
    fseek(iCode, -1, SEEK_CUR);
    char c = fgetc(iCode);
    char* string = (char *) malloc(sizeof(char) * 1);

    Token t;
    t.type = INUM;
    int point = 0;

    int length = 0;
    while(isDigit(c) || c == '.') {
        if (c == '.')
            if (!point) {
                point = 1;
                t.type = FNUM;
            }
        // else error(); TODO

        length++;
        string = (char *) realloc(string, sizeof(char) * length);
        *(string + length - 1) = c;

        c = fgetc(iCode);
    }
    length++;
    string = (char *) realloc(string, sizeof(char) * length);
    *(string + length - 1) = '\0';
    t.text = string;
    fseek(iCode, -1, SEEK_CUR);
    return t;
}

int reservedWordVerifier(char* string)
{
    if (strcmp(string, "if") == 0) return IF;
    if (strcmp(string, "else") == 0) return ELSE;
    if (strcmp(string, "while") == 0) return WHILE;
    if (strcmp(string, "for") == 0) return FOR;
    if (strcmp(string, "do") == 0) return DO;
    if (strcmp(string, "call") == 0) return CALL;
    if (strcmp(string, "asm") == 0) return ASM;
    if (strcmp(string, "true") == 0) return TRUE;
    if (strcmp(string, "false") == 0) return FALSE;
    if (strcmp(string, "char") == 0) return CHAR;
    if (strcmp(string, "int") == 0) return INT;
    if (strcmp(string, "long") == 0) return LONG;
    if (strcmp(string, "float") == 0) return FLOAT;
    if (strcmp(string, "double") == 0) return DOUBLE;
    if (strcmp(string, "bool") == 0) return BOOL;
    if (strcmp(string, "null") == 0) return NLL;
    if (strcmp(string, "import") == 0) return IMPORT;
    if (strcmp(string, "return") == 0) return RETURN;
    return 0;
}

Token wordReader(FILE* iCode)
{
    fseek(iCode, -1, SEEK_CUR);
    char c = fgetc(iCode);
    char* string = (char *) malloc(sizeof(char));

    Token t;

    int length = 0;
    while(isDigit(c) || c == '_' || isLetter(c)) {
        printf("%c", c);
        length++;
        string = (char *) realloc(string, sizeof(char) * length);
        *(string + length - 1) = c;

        c = fgetc(iCode);
    }
    length++;
    string = (char *) realloc(string, sizeof(char) * length);
    *(string + length - 1) = '\0';
    printf("\n");
    if (reservedWordVerifier(string)) t.type = reservedWordVerifier(string);
    else t.type = VAR;
    t.text = string;
    fseek(iCode, -1, SEEK_CUR);
    return t;
}

Token nextToken(FILE *iCode)
{
    Token t;
    char c;
    while (1) {
        c = fgetc(iCode);
        if (feof(iCode)) break;
        switch(c) {
        case '\n':
            linecount++;
        case ' ':
        case '\t':
        case '\r':
            continue;
        // Sigle
        case ',':
            t.type = COMMA;
            t.text = charToString(c);
            break;
        case ';':
            t.type = SEMICOLON;
            t.text = charToString(c);
            break;
        case '(':
            t.type = LBRACK;
            t.text = charToString(c);
            break;
        case ')':
            t.type = RBRACK;
            t.text = charToString(c);
            break;
        case '[':
            t.type = LSQBRACK;
            t.text = charToString(c);
            break;
        case ']':
            t.type = RSQBRACK;
            t.text = charToString(c);
            break;
        case '{':
            t.type = LBRACE;
            t.text = charToString(c);
            break;
        case '}':
            t.type = RBRACE;
            t.text = charToString(c);
            break;
        case '"':
        case '\'':
            t.type = TXT;
            t.text = stringReader(iCode);
            break;
        case '!':
            if (fgetc(iCode) == '=') {
                t.type = NEQ;
                t.text = "!=";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = NOT;
                t.text = charToString(c);
            }
            break;
        case '=':
            if (fgetc(iCode) == '=') {
                t.type = EQ;
                t.text = "==";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = ASSIGN;
                t.text = charToString(c);
            }
            break;
        case '<':
            if (fgetc(iCode) == '=') {
                t.type = LTE;
                t.text = "<=";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = LT;
                t.text = charToString(c);
            }
            break;
        case '>':
            if (fgetc(iCode) == '=') {
                t.type = GTE;
                t.text = ">=";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = GT;
                t.text = charToString(c);
            }
            break;
        case '+':
            t.type = PLUS;
            t.text = charToString(c);
            break;
        case '-':
            t.type = MINUS;
            t.text = charToString(c);
            break;
        case '*':
            t.type = MULT;
            t.text = charToString(c);
            break;
        case '/':
            t.type = DIV;
            t.text = charToString(c);
            break;
        case '%':
            t.type = MOD;
            t.text = charToString(c);
            break;
        case '&':
            if (fgetc(iCode) == '&') {
                t.type = AND;
                t.text = "&&";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = BAND;
                t.text = charToString(c);
            }
            break;
        case '|':
            if (fgetc(iCode) == '|') {
                t.type = OR;
                t.text = "||";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = BOR;
                t.text = charToString(c);
            }
            break;
        case '^':
            if (fgetc(iCode) == '^') {
                t.type = XOR;
                t.text = "^^";
            } else {
                fseek(iCode, -1, SEEK_CUR);
                t.type = BXOR;
                t.text = charToString(c);
            }
            break;
        case '~':
            t.type = BNOT;
            t.text = charToString(c);
            break;

        }

        if (isDigit(c)) {
            t = numberReader(iCode);
        } else if (isWordChar(c)) {
            t = wordReader(iCode);
        }
        t.line = linecount;
        return t;

    }
    t.type = EOF;
    t.line = linecount;
    return t;
}

Token *runLexer(FILE *iCode)
{
    Token *tokenList;
    tokenList = (Token*) malloc(sizeof(Token));

    long length = 0;

    do {
        length++;
        tokenList = (Token *) realloc(tokenList, sizeof(Token) * length);
        *(tokenList + (length-1)) = nextToken(iCode);
    } while ((tokenList + (length-1))->type != EOF);

    return tokenList;
}
