#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "wordAnalyze.h"
#include "error.h"

char errormsg[ERRORMSGNUM][ERRORMSGSIZE] =
{
    "000: identifier not defined.",
    "001: identifier duplicate defined.",
    "002: expected identifier.",
    "003: right bracket expected.",
    "004: colon expected.",
    "005: illegal symbol.",
    "006: parameter should have type.",
    "007: left bracket expected.",
    "008: var should have type.",
    "009: left square bracket expected.",
    "010: right square bracket expected.",
    "011: comma expected.",
    "012: semicolon expected.",
    "013: return value and definition do not match.",
    "014: assign symbol expected.",
    "015: equal symbol expected.",
    "016: illegal condition type.",
    "017: left curly bracket expected.",
    "018: right curly bracket expected.",
    "019: case value type illegal.",
    "020: illegal character.",
    "021: illegal constant.",
    "022: array length should be greater than zero.",
    "023: array access out of bound.",
    "024: illegal factor type.",
    "025: expected single quotation.",
    "026: expected double quotation.",
    "027: the function need to return value.",
    "028: RAM overflow.",
    "029: operator expected.",
    "030: relation operator expected.",
    "031: string expected.",
    "032: main function not found.",
    "033: lack of the type declaration.",
    "034: not support pre-zero.",
    "035: integer expected.",
    "036: illegal integer.",
    "037: function type expected.",
    "038: usage of return statement: 'return (ret-value)' or 'return ;' .",
    "039: illegal statement head.",
    "040: illegal function call, maybe the function not defined.",
    "041: 'else' must exist after 'if'.",
    "042: illegal assignment statement left part.",
    "043: illegal printf statement; Usage: printf(\"<string>\", <expression>);",
    "044: invalid types for array subscript.",
    "045: invalid 'scanf' type, must be var.",
    "046: constant types do not match."
};

void skipNextLine()
{
    do
    {
        nextchar();
    }while(ch != ';' && ch != '\n');
    retract();
    getsym();
}
void skipNextComma()
{
    do
    {
        nextchar();
    }while(ch != ',' && ch != ';');
    retract();
    getsym();
}
void error(int index)
{
    printf("\n**** line %d: ERROR %s\n", currentLine, errormsg[index]);
}

void error(int index, char str[])
{
    if(index == ERROR_DUPLICATE_IDENTIFIER)
    {
        printf("\n**** line %d: ERROR 001: identifier '%s' duplicate defined.\n", currentLine, str);
    }
    else if(index == ERROR_UNDEFINED_IDENTIFIER)
    {
        printf("\n**** line %d: ERROR 000: identifier '%s' undefined.\n", currentLine, str);
    }
    else
    {
        printf("\n**** line %d: ERROR %s\n", currentLine, errormsg[index]);
    }
}
