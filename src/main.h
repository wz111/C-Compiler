#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
/*
case, char, const, default, else, if, int, main, printf, return, scanf, switch, void, while
*/
#define CASE 1
#define CHAR 2
#define CONST 3
#define DEFAULT 4
#define ELSE 5
#define IF 6
#define INT 7
#define MAIN 8
#define PRINTF 9
#define RETURN 10
#define SCANF 11
#define SWITCH 12
#define VOID 13
#define WHILE 14

/*
relation operator: >, >=, <, <=, !=, ==
*/
#define GREATER 15
#define GREATEROREQUAL 16
#define LESS 17
#define LESSOREQUAL 18
#define NOTEQUAL 19
#define EQUAL 20

//arithmetic operator: +, -, *, /
#define PLUS 21
#define MINUS 22
#define MULT 23
#define DIVI 24

//operator: :, ,, ;, =
#define COLON 25
#define COMMA 26
#define SEMICOLON 27
#define ASSIGN 28

//Delimiter: (), [], {}, "", ''
#define LEFTBRACKET 29
#define RIGHTBRACKET 30
#define LEFTSQUAREBRACKET 31
#define RIGHTSQUAREBRACKET 32
#define LEFTCURLYBRACKET 33
#define RIGHTCURLYBRACKET 34
#define DOUBLEQUOTATION 35
#define SINGALQUOTATION 36

#define STRING 97
#define CHARCONST 98
#define INTCONST 99
#define IDENTIFIER 100

#define RESERVEDWORDSIZE 100
#define RESERVEDWORDNUM 14
#define OPERATORSIZE 10
#define OPERATORNUM 22

extern char reservedWord[RESERVEDWORDNUM][RESERVEDWORDSIZE];
extern char operatorSet[OPERATORNUM][OPERATORSIZE];
extern char helpMemory[RESERVEDWORDNUM+OPERATORNUM][RESERVEDWORDSIZE];

extern char ch; //current char
extern char token[RESERVEDWORDSIZE]; //string array
extern int num; //current integer
extern int symbol; //current word type
extern int INDEX; //number of the word
extern int charnums;

extern FILE *fpmid;
extern FILE *fpfmid;
extern FILE *fp;
extern FILE *fpmips;

extern int currentLine;

#endif // MAIN_H_INCLUDED
