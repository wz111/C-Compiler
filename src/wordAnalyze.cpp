#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "error.h"
/*
first kind:
    identifier: letter(without digit, including underline){letter|digit}

second kind:
    constant integer: digit{digit}
    constant char: 'letter('+', '-', '*', '/', 'a'~'z', 'A'~'Z', digit)'

third kind:
    reversed word(size: 20, number: 14):
        case, char, const, default, else, if, int, main, printf, return, scanf, switch, void, while

forth kind:
    Delimiter: (), [], {}, "", ''

fifth kind:
    relation operator: >, <, >=, <=, !=, ==
    arithmetic operator: +, -, *, /
    operator: :, ,, ;, =

sixth kind:
    strings: "letter{letter}"
*/

char ch = '\0';
char token[RESERVEDWORDSIZE] = {'\0'};
int num = 0;
int INDEX = 0;
int symbol = -1;
int currentLine = 1;
int charnums = 0;
FILE *fp;
FILE *fpw;

char reservedWord[RESERVEDWORDNUM][RESERVEDWORDSIZE] = {
    "case", "char", "const", "default", "else", "if",
    "int", "main", "printf", "return", "scanf", "switch",
    "void", "while"
};
char operatorSet[OPERATORNUM][OPERATORSIZE] = {
    "+", "-", "*", "/", ">", ">=", "<", "<=", "!=", "==",
    ":", ";", ",", "=", "(", ")", "[", "]", "{", "}", "\"",
    "'"
};

char helpMemory[RESERVEDWORDNUM+OPERATORNUM][RESERVEDWORDSIZE] = {
    "CASE", "CHAR", "CONST", "DEFAULT", "ELSE", "IF", "INT", "MAIN",
    "PRINTF", "RETURN", "SCANF", "SWITCH", "VOID", "WHILE",
    "GREATER", "GREATEROREQUAL", "LESS", "LESSOREQUAL", "NOTEQUAL", "EQUAL",
    "PLUS", "MINUS", "MULT", "DIVI", "COLON", "COMMA", "SEMICOLON", "ASSIGN",
    "LEFTBRACKET", "RIGHTBRACKET", "LEFTSQUAREBRACKET", "RIGHTSQUAREBRACKET",
    "LEFTCURLYBRACKET", "RIGHTCURLYBRACKET", "DOUBLEQUOTATION", "SINGALQUOTATION"
};

void clearToken()
{
    for(int i = 0; i < RESERVEDWORDSIZE ;i++)
    {
        token[i] = '\0';
    }
}

bool isSpace()
{
    return (ch == ' ');
}

bool isNewline()
{
    if(ch == '\n')
    {
        currentLine++;
        return true;
    }
    else
    {
        return false;
    }
}

bool isTab()
{
    return (ch == '\t');
}

bool isDigit()
{
    return (ch >= '0' && ch <= '9');
}

bool isLetter()
{
    return (tolower(ch) >= 'a' && tolower(ch) <= 'z');
}

bool isPlus()
{
    return (ch == '+');
}

bool isMinus()
{
    return (ch == '-');
}

bool isMult()
{
    return (ch == '*');
}

bool isDivi()
{
    return (ch == '/');
}

bool isGreater()
{
    return (ch == '>');
}

bool isLess()
{
    return (ch == '<');
}

bool isEqual()
{
    return (ch == '=');
}

bool isExcla()
{
    return (ch == '!');
}

bool isColon()
{
    return (ch == ':');
}

bool isComma()
{
    return (ch == ',');
}

bool isSemi()
{
    return (ch == ';');
}

bool isLbracket()
{
    return (ch == '(');
}

bool isRbracket()
{
    return (ch == ')');
}

bool isLSqrbracket()
{
    return (ch == '[');
}

bool isRSqrbracket()
{
    return (ch == ']');
}

bool isLClubracket()
{
    return (ch == '{');
}

bool isRClubracket()
{
    return (ch == '}');
}

bool isDoubleQuo()
{
    return (ch == '\"');
}

bool isSingalQuo()
{
    return (ch == '\'');
}

bool isUnderline()
{
    return (ch == '_');
}

int transNum()
{
    int sign = 1;
    int nums = 0;
    int i = 0;
    if(token[0] == '-')
    {
        sign = -1;
        i = 1;
    }
    else if(token[0] == '+')
    {
        sign = 1;
        i = 1;
    }

    else if(token[0] >= '0' && token[0] <= '9')
    {
        i = 0;
        sign = 1;
    }

    for(; i < (signed)strlen(token) ; i++)
    {
        nums = nums * 10 + token[i] - '0';
    }
    return (sign*nums);
}

void catToken()
{
    int last = strlen(token);
    token[last] = ch;
}

void retract()
{
    ungetc(ch, fp);
}

void nextchar()
{
    ch = fgetc(fp);
}

int reserver(char s[])
{
    for(int i = 0 ; i < RESERVEDWORDNUM ; i++)
    {
        if(strcmp(reservedWord[i], s) == 0)
        {
            return i+1;
        }
    }
    return -1;
}

int getsym()
{
    //printf("prech = %c\n", ch);
    nextchar();
    clearToken();
    while(isSpace()||isNewline()||isTab())
    {
        nextchar();
        if(ch == EOF)
        {
            return -1;
        }
    }
    //printf("ch = %c\n", ch);
    if(isLetter() || isUnderline())
    {
        while(isLetter()||isDigit()||isUnderline())
        {
            catToken();
            nextchar();
        }
        retract();
        //char tokenCopy[RESERVEDWORDSIZE] = {'\0'};
        for(int i = 0 ; i<(signed)strlen(token) ; i++)
        {
            token[i] = tolower(token[i]);
        }
        int reserveValue = reserver(token);
        //printf("%s %d\n", token, reserveValue);
        if (reserveValue == -1)
        {
            symbol = IDENTIFIER;
            INDEX++;
            //printf("%d %s %s\n", INDEX, "IDENTIFIER", token);
            //fprintf(fpw, "%d %s %s\n", INDEX, "IDENTIFIER", token);
        }
        else
        {
            symbol = reserveValue;
            /*
            for(int i = 0 ; i<(signed)strlen(token) ; i++)
            {
                token[i] = tokenCopy[i];
            }
            */
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
    }
    else if(isDigit())
    {
        while(isDigit())
        {
            catToken();
            nextchar();
        }
        retract();
        num = transNum();
        symbol = INTCONST;
        INDEX++;
        //printf("%d %s %s\n", INDEX, "INTCONST", token);
        //fprintf(fpw, "%d %s %s\n", INDEX, "INTCONST", token);
    }
    else if(isColon())
    {
        catToken();
        symbol = COLON;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isComma())
    {
        catToken();
        symbol = COMMA;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isSemi())
    {
        catToken();
        symbol = SEMICOLON;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isPlus())
    {
        catToken();
        symbol = PLUS;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol - 1], token);
    }
    else if(isMinus())
    {
        catToken();
        symbol = MINUS;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol - 1], token);
    }
    else if(isMult())
    {
        catToken();
        symbol = MULT;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isDivi())
    {
        catToken();
        symbol = DIVI;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isGreater())
    {
        catToken();
        nextchar();
        if(isEqual())
        {
            catToken();
            symbol = GREATEROREQUAL;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
        else
        {
            retract();
            symbol = GREATER;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
    }
    else if(isLess())
    {
        catToken();
        nextchar();
        if(isEqual())
        {
            catToken();
            symbol = LESSOREQUAL;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
        else
        {
            retract();
            symbol = LESS;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
    }
    else if(isExcla())
    {
        catToken();
        nextchar();
        if(isEqual())
        {
            catToken();
            symbol = NOTEQUAL;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
        else
        {
            retract();
            error(ERROR_ILLEGAL_CONDITION);
        }
    }
    else if(isEqual())
    {
        catToken();
        nextchar();
        if(isEqual())
        {
            catToken();
            symbol = EQUAL;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
        else
        {
            retract();
            symbol = ASSIGN;
            INDEX++;
            //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
            //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        }
    }
    else if(isLbracket())
    {
        catToken();
        symbol = LEFTBRACKET;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isRbracket())
    {
        catToken();
        symbol = RIGHTBRACKET;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isLSqrbracket())
    {
        catToken();
        symbol = LEFTSQUAREBRACKET;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isRSqrbracket())
    {
        catToken();
        symbol = RIGHTSQUAREBRACKET;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isLClubracket())
    {
        catToken();
        symbol = LEFTCURLYBRACKET;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isRClubracket())
    {
        catToken();
        symbol = RIGHTCURLYBRACKET;
        INDEX++;
        //printf("%d %s %s\n", INDEX, helpMemory[symbol-1], token);
        //fprintf(fpw, "%d %s %s\n", INDEX, helpMemory[symbol-1], token);
    }
    else if(isDoubleQuo())
    {
        catToken();
        nextchar();
        while(!isDoubleQuo())
        {
            catToken();
            nextchar();
            if(ch == EOF)
            {
                error(ERROR_EXPECTED_DOUBLEQUOTATION);
            }
        }
        catToken();
        symbol = STRING;
        INDEX++;
        //printf("%d %s %s\n", INDEX, "STRING", token);
        //fprintf(fpw, "%d %s %s\n", INDEX, "STRING", token);
    }
    else if(isSingalQuo())
    {
        catToken();
        nextchar();
        while(!isSingalQuo())
        {
            catToken();
            charnums++;
            nextchar();
            if(ch == EOF)
            {
                error(ERROR_EXPECTED_SINGLEQUOTATION);
            }
        }
        catToken();
        symbol = CHARCONST;
        INDEX++;
        //printf("%d %s %s\n", INDEX, "CHARCONST", token);
        //fprintf(fpw, "%d %s %s\n", INDEX, "CHARCONST", token);
    }
    else if (ch == EOF)
    {
        return -1;
    }
    else
    {
        error(ERROR_ILLEGAL_SYMBOL);
    }
    return 0;
}
