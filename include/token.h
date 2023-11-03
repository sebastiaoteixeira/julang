#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

// Delimiters
#define COMMA 0x10
#define SEMICOLON 0x11
#define LBRACK 0x12
#define RBRACK 0x13
#define LSQBRACK 0x14
#define RSQBRACK 0x15
#define LBRACE 0x16
#define RBRACE 0x17
// Primitive Types
#define CHAR 0x20
#define INT 0x21
#define LONG 0x22
#define FLOAT 0x23
#define DOUBLE 0x24
#define BOOL 0x25
// Abstract Types
#define CHARPTR 0x26
#define INTPTR 0x27
#define LONGPTR 0x28
#define FLOATPTR 0x29
#define DOUBLEPTR 0x2A
#define BOOLPTR 0x2B

// Literals
#define INUM 0x30
#define FNUM 0x31
#define TXT 0x32
#define TRUE 0x33
#define FALSE 0x34
#define NLL 0x35
// Keywords
//   Flow Statements
#define IF 0x40
#define ELSE 0x41
#define WHILE 0x42
#define FOR 0x43
#define DO 0x44
#define BREAK 0x45
#define CONTINUE 0x46
//    Assembly and Syscalls
#define CALL 0x50
#define ARGDEF 0x51
#define ASM 0x52
#define IMPORT 0x53
#define RETURN 0x54
// Variables
#define VAR 0x60
// Operators
#define NOT 0x70
#define BNOT 0x71
#define PLUS 0x72
#define MINUS 0x73
#define MULT 0x80
#define DIV 0x81
#define MOD 0x82
#define AND 0x83
#define OR 0x84
#define XOR 0x85
#define EQ 0x86
#define NEQ 0x87
#define GT 0x88
#define LT 0x89
#define GTE 0x8A
#define LTE 0x8B
#define BAND 0x8C
#define BOR 0x8D
#define BXOR 0x8E
#define ASSIGN 0x8F
#define BSL 0x90
#define BSR 0x91
#define DOT 0x92

// Abstract Tokens
#define EXPRESSION 0xB1
#define DECLARATION 0xB2
#define FUNCTION 0xB3
#define ARGUMENT 0xB4
#define PARAMETER 0xB5
#define BLOCK 0xB6
#define IFSTATEMENT 0xB7
#define WHILELOOPSTATEMENT 0xB8
#define DOWHILELOOPSTATEMENT 0xB9
#define FORLOOPSTATEMENT 0xBA
#define ASSIGNMENT 0xBB
#define PROGRAM 0xC0
#define ROOT 0xC1
#define ARRAY 0xD0
#define DARRAY 0xD1
#define OBJECT 0xD2


typedef struct {
    short type;
    char *text;
    unsigned int line;
} Token;

int isAnUnaryOperator(Token c);
int isAnOperator(Token c);
int isALiteral(Token c);
int isAType(Token c);

#endif
