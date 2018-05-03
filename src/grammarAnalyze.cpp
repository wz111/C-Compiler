#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "main.h"
#include "wordAnalyze.h"
#include "grammarAnalyze.h"
#include "midcode.h"
#include "table.h"
#include "error.h"

char idName[NAMEMAX];
int obj;
int type;
int refer;
int level;
bool normal;
int addr;
char nowitem[FOURUNITMAX];
int factorType;
bool inMain = false;
bool inVparaTable = false;
int callLevel = 0;
int switchType = 0;

char switchEndLabel[FOURUNITMAX];
char caseValue[FOURUNITMAX];

void addOperator()
{
    if(symbol == PLUS)
    {
        //printf("This is an addition operator.\n");
        //fprintf(fpw, "%s", "This is an addition operator.\n");
    }
    else if(symbol == MINUS)
    {
        //printf("This is a subtraction operator.\n");
        //fprintf(fpw, "%s", "This is a subtraction operator.\n");
    }
    else
    {
        error(ERROR_EXPECTED_OPERATOR);
        skipNextLine();
    }
}

void multOperator()
{
    if(symbol == MULT)
    {
        //printf("This is a multiplication operator\n");
        //fprintf(fpw, "%s", "This is a multiplication operator\n");
    }
    else if(symbol == DIVI)
    {
        //printf("This is a division operator\n");
        //fprintf(fpw, "%s", "This is a division operator\n");
    }
    else
    {
        error(ERROR_EXPECTED_OPERATOR);
        skipNextLine();
    }
}

void relaOperator()
{
    if(symbol == GREATER)
    {
        //printf("This is a greater-than operator.\n" );
        //fprintf(fpw, "%s", "This is a greater-than operator.\n" );
        getsym();
    }
    else if(symbol == GREATEROREQUAL)
    {
        //printf("This is a greater-or-equal operator.\n" );
        //fprintf(fpw, "%s", "This is a greater-or-equal operator.\n" );
        getsym();
    }
    else if(symbol == LESS)
    {
        //printf("This is a less-than operator.\n" );
        //fprintf(fpw, "%s", "This is a less-than operator.\n" );
        getsym();
    }
    else if(symbol == LESSOREQUAL)
    {
        //printf("This is a less-or-equal operator.\n" );
        //fprintf(fpw, "%s", "This is a less-or-equal operator.\n" );
        getsym();
    }
    else if(symbol == EQUAL)
    {
        //printf("This is an equal-to operator.\n" );
        //fprintf(fpw, "%s", "This is an equal-to operator.\n" );
        getsym();
    }
    else if(symbol == NOTEQUAL)
    {
        //printf("This is a not-equal-to operator.\n" );
        getsym();
    }
    else
    {
        error(ERROR_EXPECTED_RELAOPERATOR);
        skipNextLine();
        return ;
    }
}

int letter(char s)
{
    if(s >= 'a' && s <= 'z')
    {
        //printf("This is a letter.\n");
        //fprintf(fpw, "%s", "This is a letter.\n");
        return 1;
    }
    else if(s >= 'A' && s <= 'Z')
    {
        //printf("This is a letter.\n");
        //fprintf(fpw, "%s", "This is a letter.\n");
        return 1;
    }
    else if(s == '_')
    {
        //printf("This is a letter.\n");
        //fprintf(fpw, "%s", "This is a letter.\n");
        return 1;
    }
    else
    {
        return -1;
    }
}

int number(char s)
{
    int value = s - '0';
    if(value == 0 || notZeroNum(s) != -1)
    {
        //printf("This is a number.\n");
        //fprintf(fpw, "%s", "This is a number.\n");
        return 1;
    }
    else
    {
        return -1;
    }
}

int notZeroNum(char s)
{
    int value = s - '0';
    if(value >= 1 && value <= 9)
    {
        //printf("This is a not-zero number.\n");
        //fprintf(fpw, "%s", "This is a not-zero number.\n");
        return 1;
    }
    else
        return -1;
}

void character()
{
    if(symbol == CHARCONST && charnums == 1)
    {
        charnums = 0;
        if(strcmp(token, "'+'") == 0 || strcmp(token, "'-'") == 0)
        {
            //printf("This is a character.\n");
            //fprintf(fpw, "%s", "This is a character.\n");
            getsym();
        }
        else if(strcmp(token, "'*'") == 0 || strcmp(token, "'/'") == 0)
        {
            //printf("This is a character.\n");
            //fprintf(fpw, "%s", "This is a character.\n");
            getsym();
        }
        else if(number(token[1]) == 1 || letter(token[1]) == 1)
        {
            //printf("This is a character.\n");
            //fprintf(fpw, "%s", "This is a character.\n");
            getsym();
        }
        else
        {
            error(ERROR_ILLEGAL_CONSTVALUE);
            skipNextComma();
        }
    }
    else
    {
        charnums = 0;
        error(ERROR_ILLEGAL_CONSTVALUE);
        skipNextComma();
    }
}

void charString()
{
    if(symbol == STRING)
    {
        for(int i = 1 ; i < (signed)strlen(token)-1 ; i++)
        {
            if( (token[i] >= 35 && token[i] <= 126) || token[i] == 32 || token[i] == 33)
            {
                continue;
            }
            else
            {
                error(ERROR_ILLEGAL_CHARACTER);
                skipNextLine();
                return ;
            }
        }
        //printf("This is a character string.\n");
        //fprintf(fpw, "%s", "This is a character string.\n");
        getsym();
    }
    else
    {
        error(ERROR_EXPECTED_STRING);
        skipNextLine();
        return ;
    }
}

void program()
{
    int mainflag = 0;
    if(symbol == CONST)
    {
        constDeclr();
    }
    if(symbol == INT || symbol == CHAR)
    {
        int flag = 0;
        int times = 0;
        while(ch != ';')
        {
            if(ch == '{')
            {
                flag = 1;
                break;
            }
            nextchar();
            times++;
        }
        fseek(fp, -1L*(times+1), SEEK_CUR);
        nextchar();
        if(flag == 1)
        {
            ;
        }
        else
        {
            varDeclr();
        }
    }
    while(symbol == INT || symbol == CHAR || symbol == VOID)
    {
        if(symbol == VOID)
        {
            getsym();
            if(symbol == MAIN)
            {
                fseek(fp, -1L * (signed)strlen(token), SEEK_CUR);
                symbol = VOID;
                break;
            }
            else
            {
                fseek(fp, -1L * (signed)strlen(token), SEEK_CUR);
                symbol = VOID;
            }
            voidFuncDefine();
        }
        else if(symbol == INT || symbol == CHAR)
        {
            retFuncDefine();
        }
    }
    if(symbol == VOID)
    {
        mainFunc();
        mainflag = 1;
    }
    if (mainflag != 1)
    {
        error(ERROR_EXPECTED_MAIN);
        return ;
    }
    //printf("This is a program.\n\n");
    //fprintf(fpw, "%s", "This is a program.\n\n");
}

void constDeclr()
{
    if(symbol == CONST)
    {
        getsym();
        constDef();
        while(symbol == SEMICOLON)
        {
            getsym();
            if(symbol == CONST)
            {
                getsym();
                constDef();
            }
            else
            {
                break;
            }
        }
        //printf("This is a constant declaration.\n");
        //fprintf(fpw, "%s", "This is a constant declaration.\n");
    }
    else
    {
        error(ERROR_EXPECTED_TYPE);
        skipNextLine();
        return ;
    }
}

void constDef()
//level here is decided by outsides, if function, level = 2, else level = 1
{
    if(symbol == INT)
    {
        type = INTTYPE;
        obj = CONSTOBJ;
        refer = 0;
        normal = true;
        getsym();
        if(symbol == IDENTIFIER)
        {
            strcpy(idName, token);
        }
        identifier();
        if(symbol == ASSIGN)
        {
            getsym();
            integer();
            addr = num;
            tabInsert(idName, obj, type, refer, level, normal, addr);
            genMidCode(constop, intop, int2str(addr), idName);
            //printf("This is a constant definition.\n");
            //fprintf(fpw, "%s", "This is a constant definition.\n");
        }
        else
        {
            error(ERROR_EXPECTED_ASSIGN);
            skipNextComma();
        }
        while(symbol == COMMA)
        {
            getsym();
            if(symbol == IDENTIFIER)
            {
                strcpy(idName, token);
            }
            identifier();
            if(symbol == ASSIGN)
            {
                getsym();
                integer();
                addr = num;
                tabInsert(idName, obj, type, refer, level, normal, addr);
                genMidCode(constop, intop, int2str(addr), idName);
                //printf("This is a constant definition.\n");
                //fprintf(fpw, "%s", "This is a constant definition.\n");
            }
            else
            {
                error(ERROR_EXPECTED_ASSIGN);
                skipNextComma();
            }
        }
    }
    else if (symbol == CHAR)
    {
        type = CHARTYPE;
        obj = CONSTOBJ;
        refer = 0;
        normal = true;
        getsym();
        if(symbol == IDENTIFIER)
        {
            strcpy(idName, token);
        }
        identifier();
        if(symbol == ASSIGN)
        {
            getsym();
            addr = token[1];
            character();

            tabInsert(idName, obj, type, refer, level, normal, addr);
            genMidCode(constop, charop, int2str(addr), idName);
            //printf("This is a constant definition.\n");
            //fprintf(fpw, "%s", "This is a constant definition.\n");
        }
        else
        {
            error(ERROR_EXPECTED_ASSIGN);
            skipNextComma();
        }
        while(symbol == COMMA)
        {
            getsym();
            if(symbol == IDENTIFIER)
            {
                strcpy(idName, token);
            }
            identifier();
            if(symbol == ASSIGN)
            {
                getsym();
                addr = token[1];
                character();
                tabInsert(idName, obj, type, refer, level, normal, addr);
                genMidCode(constop, charop, int2str(addr), idName);
                //printf("This is a constant definition.\n");
                //fprintf(fpw, "%s", "This is a constant definition.\n");
            }
            else
            {
                error(ERROR_EXPECTED_ASSIGN);
                skipNextComma();
            }
        }
    }
    else
    {
        error(ERROR_EXPECTED_TYPE);
        skipNextLine();
        return ;
    }
}

void identifier()
{
    if(symbol == IDENTIFIER)
    {
        if(letter(token[0]) == 1)
        {
            for(int i = 1 ; i < (signed)strlen(token) ; i++)
            {
                if(letter(token[i]) == 1 || number(token[i]) == 1)
                {
                    continue;
                }
                else
                {
                    error(ERROR_ILLEGAL_CHARACTER);
                    //break;
                }
            }
            //printf("This is an identifier.\n");
            //fprintf(fpw, "%s", "This is an identifier.\n");
            getsym();
        }
        else
        {
            error(ERROR_ILLEGAL_CHARACTER);
            //skipNextComma();
        }
    }
    else
    {
        error(ERROR_EXPECTED_IDENTIFIER);
        //skipNextComma();
    }
}

void integer()
{
    if(token[0] == '0')
    {
        if((signed)strlen(token) != 1)
        {
            error(ERROR_ILLEGAL_PREZERO);
            char temptoken[RESERVEDWORDSIZE];
            for(int i = 0 ; i<(signed)strlen(token) ; i++ )
            {
                if(token[i] >= '1' && token[i] <= '9')
                {
                    strcpy(temptoken, token+i);
                    unsignInteger(temptoken);
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
        else
        {
            //printf("This is an integer 0.\n");
            //fprintf(fpw, "%s", "This is an integer 0.\n");
            getsym();
        }
    }
    else if(token[0] == '-' || token[0] == '+')
    {
        getsym();
        unsignInteger(token);
        //printf("This is an integer %d.\n", num);
        //fprintf(fpw, "This is an integer %d.\n", num);
    }
    else if(token[0] >= '1' && token[0] <= '9')
    {
        unsignInteger(token);
        //printf("This is an integer %d.\n", num);
        //fprintf(fpw, "This is an integer %d.\n", num);
    }
    else
    {
        error(ERROR_EXPECTED_INTEGER);
        skipNextComma();
    }
}

int unsignInteger(char s[])
{
    if(notZeroNum(s[0]) == 1)
    {
        for(int i = 1 ; i<(signed)strlen(s) ; i++)
        {
            if(number(s[i]) == 1)
            {
                continue;
            }
            else
            {
                error(ERROR_ILLEGAL_INTEGER);
                return -1;
            }
        }
        getsym();
        return 0;
    }
    else
    {
        if(s[0] == '0' && (signed)strlen(s) > 0)
        {
            error(ERROR_ILLEGAL_PREZERO);
        }
        else //if(s[0] == '+' || s[0] == '-')
        {
            error(ERROR_ILLEGAL_INTEGER);
        }
        return -1;
    }
}

void declrHead()
{
    if(symbol == INT || symbol == CHAR)
    {
        type = (symbol == INT)?INTTYPE : CHARTYPE;
        getsym();
        if(symbol == IDENTIFIER)
        {
            strcpy(idName, token);
            obj = FUNCOBJ;
            level = 1;
            refer = 0;
            normal = true;
            addr = 0;    //record the present relative address so that call-statement can use the address
            tabInsert(idName, obj, type, refer, level, normal, addr);
            if(type == INTTYPE)
            {
                genMidCode(funcop, intop, blank, idName);
            }
            else
            {
                genMidCode(funcop, charop, blank, idName);
            }
        }
        identifier();
        //printf("This is a declaration head.\n");
        //fprintf(fpw, "This is a declaration head.\n");
    }
    else
    {
        error(ERROR_EXPECTED_TYPE);
        skipNextLine();
    }
}

void varDeclr()
{
    obj = VAROBJ;
    varDef();
    if(symbol == SEMICOLON)
    {
        getsym();
        while(symbol == INT || symbol == CHAR)
        {
            int flag = 0;
            int times = 0;
            while (ch != ';')
            {
                if (ch == '{' || ch == '(')
                {
                    flag = 1;
                    break;
                }
                nextchar();
                times++;
            }
            fseek(fp, -1L * (times + 1), SEEK_CUR);
            nextchar();
            if (flag == 1)
            {
                break;
            }
            else
            {
                varDef();
                if (symbol == SEMICOLON)
                {
                    getsym();
                }
                else
                {
                    error(ERROR_EXPECTED_SEMICOLON);
                    skipNextLine();
                }
            }
        }
        //printf("This is a var declaration.\n");
        //fprintf(fpw, "This is a var declaration.\n");
    }
    else
    {
        error(ERROR_EXPECTED_SEMICOLON);
        skipNextLine();
    }
}

void varDef()
{
    //level here is decided by the outside, if function, level is 2, else level is 1
    type = (symbol == INT)?INTTYPE: CHARTYPE;
    normal = true;
    refer = 0;
    typeIdentifier();
    if(symbol == IDENTIFIER)
    {
        strcpy(idName, token);
    }
    identifier();
    if(symbol == LEFTSQUAREBRACKET)
    {
        getsym();
        if(symbol == INTCONST)
        {
            refer = num;
            obj = ARRAYOBJ;
            if(refer > 0)
            {
                if(unsignInteger(token) == -1)
                {
                    while(ch != RIGHTSQUAREBRACKET)
                    {
                        nextchar();
                    }
                    getsym();
                }
                else
                {
                    ;
                }
            }
            else
            {
                error(ERROR_ILLEGAL_ARRAYSIZENOTZERO);
                getsym();
            }
        }
        else
        {
            error(ERROR_ILLEGAL_ARRAYSIZENOTZERO);
            while(ch != ']')
            {
                nextchar();
            }
            retract();
            getsym();
        }

        if(symbol == RIGHTSQUAREBRACKET)
        {
            addr+=num;
            tabInsert(idName, obj, type, refer, level, normal, addr);
            if(type == INTTYPE)
            {
                genMidCode(intop, blank, refer, idName);
            }
            else
            {
                genMidCode(charop, blank, refer, idName);
            }
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_RIGHTSQUAREBRACKET);
            skipNextComma();
        }
    }
    else
    {
        obj = VAROBJ;
        addr++;
        tabInsert(idName, obj, type, refer, level, normal, addr);
        if(type == INTTYPE)
        {
            genMidCode(intop, blank, blank, idName);
        }
        else
        {
            genMidCode(charop, blank, blank, idName);
        }
    }
    while(symbol == COMMA)
    {
        getsym();
        if(symbol == IDENTIFIER)
        {
            strcpy(idName, token);
        }
        identifier();
        if(symbol == LEFTSQUAREBRACKET)
        {
            getsym();
            refer = num;
            obj = ARRAYOBJ;
            unsignInteger(token);
            if(symbol == RIGHTSQUAREBRACKET)
            {
                addr+=num;
                tabInsert(idName, obj, type, refer, level, normal, addr);
                if(type == INTTYPE)
                {
                    genMidCode(intop, blank, refer, idName);
                }
                else
                {
                    genMidCode(charop, blank, refer, idName);
                }
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTSQUAREBRACKET);
                skipNextComma();
            }
        }
        else
        {
            obj = VAROBJ;
            addr++;
            tabInsert(idName, obj, type, refer, level, normal, addr);
            if(type == INTTYPE)
            {
                genMidCode(intop, blank, blank, idName);
            }
            else
            {
                genMidCode(charop, blank, blank, idName);
            }
        }
    }
    //printf("This is a var definition.\n");
    //fprintf(fpw, "This is a var definition.\n");
}

void constant()
{
    if(symbol == INTCONST)
    {
        strcpy(caseValue, token);
        integer();
        //printf("This is an integer constant.\n");
        //fprintf(fpw, "This is an integer constant.\n");
    }
    else if (symbol == PLUS || symbol == MINUS)
    {
        int sign = (symbol == PLUS)?1:-1;
        getsym();
        sprintf(caseValue, "%d", sign * num);
        unsignInteger(token);
        //printf("This is an integer constant.\n");
        //fprintf(fpw, "This is an integer constant.\n");
    }
    else if(symbol == CHARCONST)
    {
        strcpy(caseValue, token);
        character();
        //printf("This is a character constant.\n");
        //fprintf(fpw, "This is an integer constant.\n");
    }
    else
    {
        error(ERROR_ILLEGAL_CONSTVALUE);
        skipNextComma();
    }
}

void typeIdentifier()
{
    if(symbol == INT || symbol == CHAR)
    {
        //printf("This is a type identifier.\n");
        //fprintf(fpw, "This is a type identifier.\n");
        getsym();
    }
    else
    {
        error(ERROR_EXPECTED_TYPE);
    }
}

void retFuncDefine()
{
    declrHead();
    if(symbol == LEFTBRACKET)
    {
        getsym();
        parameter();
        if(symbol == RIGHTBRACKET)
        {
            getsym();
            if(symbol == LEFTCURLYBRACKET)
            {
                getsym();
                compoundStatement();
                if(symbol == RIGHTCURLYBRACKET)
                {
                    genMidCode(myendop, blank, blank, blank);
                    //printf("This is a return-function definition(have parameter).\n");
                    //fprintf(fpw, "This is a return-function definition(have parameter).\n");
                    getsym();
                }
                else
                {
                    error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
                    getsym();
                }
            }
            else
            {
                error(ERROR_EXPECTED_LEFTCURLYBRACKET);
                getsym();
            }
        }
        else //right bracket if
        {
            error(ERROR_EXPECTED_RIGHTBRACKET);
            getsym();
        }
    }
    else
    {
        if(symbol == LEFTCURLYBRACKET)
        {
            getsym();
            compoundStatement();
            if(symbol == RIGHTCURLYBRACKET)
            {
                genMidCode(myendop, blank, blank, blank);
                //printf("This is a return-function definition(no parameter).\n");
                //fprintf(fpw, "This is a return-function definition(no parameter).\n");
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
                getsym();
            }
        }
        else
        {
            error(ERROR_EXPECTED_LEFTCURLYBRACKET);
            getsym();
        }
    }
}

void voidFuncDefine()
{
    obj = FUNCOBJ;
    type = VOIDTYPE;
    refer = 0;
    if(symbol == VOID)
    {
        getsym();
        if(symbol == IDENTIFIER)
        {
            strcpy(idName, token);
            addr = 0;
            normal = true;
            level = 1;
            tabInsert(idName, obj, type, refer, level, normal, addr);
            genMidCode(funcop, voidop, blank, idName);
        }
        identifier();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            parameter();
            if(symbol != RIGHTBRACKET)
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
            }
            else
            {
                getsym();
            }
            if(symbol != LEFTCURLYBRACKET)
            {
                error(ERROR_EXPECTED_LEFTCURLYBRACKET);
            }
            else
            {
                getsym();
            }
            compoundStatement();
            if(symbol != RIGHTCURLYBRACKET)
            {
                error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
            }
            else
            {
                genMidCode(vendop, blank, blank, blank);
                getsym();
            }
        }
        else
        {
            if(symbol != LEFTCURLYBRACKET)
            {
                error(ERROR_EXPECTED_LEFTBRACKET);
                while(symbol != RIGHTBRACKET)
                {
                    getsym();
                }
                getsym();
            }
            if(symbol == LEFTCURLYBRACKET)
            {
                getsym();
                compoundStatement();
                if(symbol == RIGHTCURLYBRACKET)
                {
                    genMidCode(vendop, blank, blank, blank);
                    //printf("This is a void-function definition(no parameter).\n");
                    //fprintf(fpw, "This is a void-function definition(no parameter).\n");
                    getsym();
                }
                else
                {
                    error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
                    //getsym();
                }
            }
            else
            {
                error(ERROR_EXPECTED_LEFTCURLYBRACKET);
                compoundStatement();
                if(symbol == RIGHTCURLYBRACKET)
                {
                    genMidCode(vendop, blank, blank, blank);
                    //printf("This is a void-function definition(no parameter).\n");
                    //fprintf(fpw, "This is a void-function definition(no parameter).\n");
                    getsym();
                }
                else
                {
                    error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
                    getsym();
                }
            }
        }
    }
    else
    {
        error(ERROR_EXPECTED_FUNCTYPE);
        getsym();
    }
}

void parameter()
{
    paramTable();
    //printf("This is a parameter.\n");
    //fprintf(fpw, "This is a parameter.\n");
}

void paramTable()
{
    type = (symbol == INT) ? INTTYPE:CHARTYPE;
    obj = PARAMOBJ;
    level = 2;
    refer = 0;
    typeIdentifier();
    if(symbol == IDENTIFIER)
    {
        strcpy(idName, token);
        normal = false;
        addr++;
        tabInsert(idName, obj, type, refer, level, normal, addr);
        if(type == INTTYPE)
        {
            genMidCode(paraop, intop, blank, idName);
        }
        else
        {
            genMidCode(paraop, charop, blank, idName);
        }
    }
    identifier();
    while(symbol == COMMA)
    {
        getsym();
        type = (symbol == INT) ? INTTYPE:CHARTYPE;
        typeIdentifier();
        if(symbol == IDENTIFIER)
        {
            strcpy(idName, token);
            normal = false;
            addr++;
            tabInsert(idName, obj, type, refer, level, normal, addr);
            if(type == INTTYPE)
            {
                genMidCode(paraop, intop, blank, idName);
            }
            else
            {
                genMidCode(paraop, charop, blank, idName);
            }
        }
        identifier();
    }
    //printf("This is a parameter table.\n");
    //fprintf(fpw, "This is a parameter table.\n");
}

void mainFunc()
{
    inMain = true;
    if(symbol == VOID)
    {
        type = VOIDTYPE;
        obj = FUNCOBJ;
        level = 1;
        getsym();
        if(symbol == MAIN)
        {
            strcpy(idName, token);
            addr = 0;
            normal = true;
            refer = 0;
            tabInsert(idName, obj, type, refer, level, normal, addr);
            genMidCode(funcop, voidop, blank, idName);
            getsym();
            if(symbol != LEFTBRACKET)
            {
                error(ERROR_EXPECTED_LEFTBRACKET);
            }
            else
            {
                getsym();
            }
            if(symbol != RIGHTBRACKET)
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
            }
            else
            {
                getsym();
            }
            if(symbol != LEFTCURLYBRACKET)
            {
                error(ERROR_EXPECTED_LEFTCURLYBRACKET);
            }
            else
            {
                getsym();
            }
            compoundStatement();
            if(symbol != RIGHTCURLYBRACKET)
            {
                error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
            }
            genMidCode(exitop, blank, blank, blank);
            getsym();
        }
        else
        {
            //printf("\n****\n");
            //printf("%s\n", token);
            error(ERROR_EXPECTED_MAIN);
            getsym();
        }
    }
    else
    {
        error(ERROR_EXPECTED_FUNCTYPE);
        getsym();
    }
}

void compoundStatement()
{
    if(symbol == CONST)
    {
        constDeclr();
    }
    if(symbol == INT || symbol == CHAR)
    {
        varDeclr();
    }
    statementColumn();
    //printf("This is a compound statement.\n");
    //fprintf(fpw, "This is a compound statement.\n");
}

void statementColumn()
{
    do{
        statement();
        if(!(symbol == IF || symbol == WHILE ||
          symbol == LEFTCURLYBRACKET || symbol == IDENTIFIER ||
          symbol == SCANF || symbol == PRINTF ||
          symbol == SWITCH || symbol == RETURN ||
          symbol == SEMICOLON))
          {
              if(symbol == RIGHTCURLYBRACKET || symbol == INT || symbol == CHAR || symbol == VOID)
              {
                  return ;
              }
              skipNextLine();
              continue;
          }
    }while(symbol == IF || symbol == WHILE ||
          symbol == LEFTCURLYBRACKET || symbol == IDENTIFIER ||
          symbol == SCANF || symbol == PRINTF ||
          symbol == SWITCH || symbol == RETURN ||
          symbol == SEMICOLON);
    //printf("This is a statement column.\n");
    //fprintf(fpw, "This is a statement column.\n");
}

void statement()
{
    if(symbol == IF)
    {
        conditionalStatement();
    }
    else if(symbol == WHILE)
    {
        loopStatement();
    }
    else if(symbol == LEFTCURLYBRACKET)
    {
        getsym();
        statementColumn();
        if(symbol == RIGHTCURLYBRACKET)
        {
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
            getsym();
        }
    }
    else if(symbol == IDENTIFIER)
    {
        int fppre = (int)fp;
        char tokencpy[RESERVEDWORDSIZE] = { '\0' };
        strcpy(tokencpy, token);
        identifier();
        if(symbol == ASSIGN || symbol == LEFTSQUAREBRACKET)
        {
            fseek(fp, -1L * ((int)fp - fppre + 1), SEEK_CUR);
            symbol = IDENTIFIER;
            strcpy(token, tokencpy);
            assignmentStatement();
            if (symbol == SEMICOLON)
            {
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_SEMICOLON);
            }
        }
        else if(symbol == LEFTBRACKET || symbol == SEMICOLON )
        {
            int legal;
            legal = tabSearch(tokencpy);
            if(legal != -1 && tab.mytab[legal].obj == FUNCOBJ)
            {
                if(tab.mytab[legal].typ == VOIDTYPE)
                {
                    voidFuncCall();
                    genMidCode(callop, tokencpy, callLevel, blank);
                    callLevel--;
                }
                else if(tab.mytab[legal].typ == INTTYPE || tab.mytab[legal].typ == CHARTYPE)
                {
                    retFuncCall();
                    genMidCode(callop, tokencpy, callLevel, blank);
                    callLevel--;
                }
                else
                {
                    error(ERROR_EXPECTED_FUNCTYPE);
                    getsym();
                }
            }
            else
            {
                error(ERROR_ILLEGAL_FUNCCALL);
                skipNextLine();
            }
            if(symbol == SEMICOLON)
            {
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_SEMICOLON);
                getsym();
            }
        }
        else
        {
            error(ERROR_EXPECTED_VARTYPE);
            skipNextComma();
        }
    }
    else if(symbol == SCANF)
    {
        readStatement();
        if(symbol == SEMICOLON)
        {
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_SEMICOLON);
            getsym();
        }
    }
    else if(symbol == PRINTF)
    {
        writeStatement();
        if(symbol == SEMICOLON)
        {
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_SEMICOLON);
            getsym();
        }
    }
    else if(symbol == SEMICOLON)
    {
        if(ch == EOF)
        {
            error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
            symbol = RIGHTCURLYBRACKET;
        }
        else
        {
            getsym();
        }
    }
    else if(symbol == SWITCH)
    {
        situationStatement();
    }
    else if(symbol == RETURN)
    {
        retStatement();
        if(symbol == SEMICOLON)
        {
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_SEMICOLON);
            getsym();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        skipNextLine();
    }
}

void retFuncCall()
{
    callLevel++;
    if(symbol == LEFTBRACKET)
    {
        getsym();
        valueParamTable();
        if(symbol == RIGHTBRACKET)
        {
            //printf("This is a return-function call statement.\n");
            //fprintf(fpw, "This is a return-function call statement.\n");
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_RIGHTBRACKET);
            getsym();
        }
    }
    else
    {
        //printf("This is a return-function call statement.\n");
        //fprintf(fpw, "This is a return-function call statement.\n");
    }
}

void voidFuncCall()
{
    if(symbol == LEFTBRACKET)
    {
        getsym();
        valueParamTable();
        if(symbol == RIGHTBRACKET)
        {
            //printf("This is a void-function call statement.(with parameter)\n");
            //fprintf(fpw, "This is a void-function call statement(with parameter).\n");
            getsym();
        }
        else
        {
            error(ERROR_EXPECTED_RIGHTBRACKET);
            getsym();
        }
    }
    else if(symbol == SEMICOLON)
    {
        //printf("This is a void-function call statement.(no parameter)\n");
        //fprintf(fpw, "This is a void-function call statement.(no parameter)\n");
    }
    else
    {
        error(ERROR_ILLEGAL_FUNCCALL);
        getsym();
    }
}

void conditionalStatement()
{
    char elseBeginLabel[FOURUNITMAX];
    char elseEndLabel[FOURUNITMAX];
    strcpy(elseBeginLabel, genLabel());
    strcpy(elseEndLabel, genLabel());
    if(symbol == IF)
    {
        getsym();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            condition();
            genMidCode(ifnotop, blank, blank, elseBeginLabel);
            if(symbol == RIGHTBRACKET)
            {
                getsym();
                statement();
                genMidCode(ifop, blank, blank, elseEndLabel);
                if(symbol == ELSE)
                {
                    genMidCode(labop, blank, blank, elseBeginLabel);
                    getsym();
                    statement();
                    genMidCode(labop, blank, blank, elseEndLabel);
                    //printf("This is a condition statement.\n");
                    //fprintf(fpw, "This is a condition statement.\n");
                }
                else
                {
                    error(ERROR_EXPECTED_ELSE);
                    getsym();
                }
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
                getsym();
            }
        }
        else
        {
            error(ERROR_EXPECTED_LEFTBRACKET);
            getsym();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
}

void expression()
{
    factorType = 0;
    char temparg1[FOURUNITMAX];
    char temparg2[FOURUNITMAX];
    char tempres[FOURUNITMAX];
    if(symbol == PLUS || symbol == MINUS)
    {
        factorType = INT;
        if(symbol == PLUS)
        {
            getsym();
            item();
            strcpy(tempres, nowitem);
        }
        else
        {
            getsym();
            item();
            strcpy(temparg1, nowitem);
            strcpy(tempres, genTempVar());
            genMidCode(subop, myzero, temparg1, tempres);
        }
        while(symbol == PLUS || symbol == MINUS)
        {
            factorType = INT;
            strcpy(temparg1, tempres);
            if(symbol == PLUS)
            {
                getsym();
                item();
                strcpy(temparg2, nowitem);
                strcpy(tempres, genTempVar());
                genMidCode(addop, temparg1, temparg2, tempres);
            }
            else
            {
                getsym();
                item();
                strcpy(temparg2, nowitem);
                strcpy(tempres, genTempVar());
                genMidCode(subop, temparg1, temparg2, tempres);
            }
            factorType = INT;
        }
        strcpy(nowitem, tempres);
    }
    else
    {
        item();
        strcpy(tempres, nowitem);
        while(symbol == PLUS || symbol == MINUS)
        {
            factorType = INT;
            strcpy(temparg1, tempres);
            if(symbol == PLUS)
            {
                getsym();
                item();
                strcpy(temparg2, nowitem);
                strcpy(tempres, genTempVar());
                genMidCode(addop, temparg1, temparg2, tempres);
            }
            else
            {
                getsym();
                item();
                strcpy(temparg2, nowitem);
                strcpy(tempres, genTempVar());
                genMidCode(subop, temparg1, temparg2, tempres);
            }
            factorType = INT;
        }
    }
    strcpy(nowitem, tempres);
    //printf("This is an expression.\n");
    //fprintf(fpw, "This is an expression.\n");
}

void item()
{
    char temparg1[FOURUNITMAX];
    char temparg2[FOURUNITMAX];
    char tempres[FOURUNITMAX];
    factor();
    strcpy(tempres, nowitem);
    while(symbol == MULT || symbol == DIVI)
    {
        strcpy(temparg1, tempres);
        if(symbol == MULT)
        {
            getsym();
            factor();
            strcpy(temparg2, nowitem);
            strcpy(tempres, genTempVar());
            genMidCode(multop, temparg1, temparg2, tempres);
        }
        else
        {
            getsym();
            factor();
            strcpy(temparg2, nowitem);
            strcpy(tempres, genTempVar());
            genMidCode(diviop, temparg1, temparg2, tempres);
        }
    }
    strcpy(nowitem, tempres);
    //printf("This is an item.\n");
    //fprintf(fpw, "This is an item.\n");
}

void factor()
{
    int legal;
    char tempres[FOURUNITMAX];
    strcpy(tempres, blank);
    if(symbol == IDENTIFIER)
    {
        char tempIdName[NAMEMAX];
        strcpy(idName, token);
        strcpy(tempIdName, token);
        legal = tabSearch(tempIdName);
        if(legal == -1)
        {
            error(ERROR_UNDEFINED_IDENTIFIER, tempIdName);
            getsym();
            return ;
        }
        identifier();
        if(tab.mytab[legal].obj == FUNCOBJ || symbol == LEFTBRACKET)
        {
            strcpy(tempres, genTempVar());
            factorType = (tab.mytab[legal].typ == INTTYPE)?INT:CHAR;
            retFuncCall();
            genMidCode(callop, tempIdName, callLevel, tempres);
            callLevel--;
            strcpy(nowitem, tempres);
        }
        else if(symbol == LEFTSQUAREBRACKET)
        {
            getsym();
            int tempFactorType = factorType;
            expression();
            factorType = tempFactorType;
            //array index deal
            char temp[FOURUNITMAX];
            strcpy(temp, nowitem);
            legal = tabSearch(tempIdName);
            if(legal != -1)
            {
                if(tab.mytab[legal].obj == ARRAYOBJ)
                {
                    if(tab.mytab[legal].refer > atoi(temp))
                    {
                        factorType = (tab.mytab[legal].typ == INTTYPE)?INT:CHAR;;
                    }
                    else
                    {
                        error(ERROR_ILLEGAL_ARRAYOUTOFBOUND);
                    }
                }
                else
                {
                    error(ERROR_UNDEFINED_IDENTIFIER, tempIdName);
                    getsym();
                }
            }
            else
            {
                error(ERROR_UNDEFINED_IDENTIFIER, tempIdName);
                getsym();
            }
            if(symbol == RIGHTSQUAREBRACKET)
            {
                legal = tabSearch(tempIdName);
                strcpy(nowitem, genTempVar());
                genMidCode(arraytovalueop, tempIdName, temp, nowitem);
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTSQUAREBRACKET);
                getsym();
            }
        }
        else
        {
            legal = tabSearch(idName);
            if(legal != -1)
            {
                strcpy(nowitem, tab.mytab[legal].name);
                factorType = (tab.mytab[legal].typ == INTTYPE)?INT:CHAR;
            }
            else
            {
                error(ERROR_UNDEFINED_IDENTIFIER, idName);
                getsym();
            }
        }
    }
    else if(symbol == LEFTBRACKET)
    {
        getsym();
        expression();
        if(symbol == RIGHTBRACKET)
        {
            getsym();
        }
        else
        {
            ;
        }
    }
    else if(symbol == INTCONST)
    {
        sprintf(nowitem, "%d", num);
        factorType = INT;
        integer();
    }
    else if(symbol == CHARCONST)
    {
        sprintf(nowitem, "%d", token[1]);
        factorType = CHAR;
        character();
    }
    else if(symbol == PLUS || symbol == MINUS)
    {
        int sign = (symbol == PLUS)?1:-1;
        getsym();
        sprintf(nowitem, "%d", sign * num);
        factorType = INT;
        integer();
    }
    else
    {
        error(ERROR_ILLEGAL_FACTORTYPE);
        return ;
    }
    //printf("This is a factor.\n");
    //fprintf(fpw, "This is a factor.\n");
}

void condition()
{
    char temparg1[FOURUNITMAX];
    char temparg2[FOURUNITMAX];
    char tempop[FOURUNITMAX];
    expression();
    strcpy(temparg1, nowitem);
    if(symbol == LESS || symbol == GREATER ||
       symbol == LESSOREQUAL || symbol == GREATEROREQUAL ||
       symbol == EQUAL || symbol == NOTEQUAL)
    {
        if(symbol == LESS)
        {
            strcpy(tempop, lessop);
        }
        else if(symbol == GREATER)
        {
            strcpy(tempop, greatop);
        }
        else if(symbol == LESSOREQUAL)
        {
            strcpy(tempop, lessorequalop);
        }
        else if(symbol == GREATEROREQUAL)
        {
            strcpy(tempop, greatorequalop);
        }
        else if(symbol == EQUAL)
        {
            strcpy(tempop, equalop);
        }
        else if(symbol == NOTEQUAL)
        {
            strcpy(tempop, notequalop);
        }
        else
        {
            error(ERROR_EXPECTED_RELAOPERATOR);
        }
        relaOperator();
        expression();
        strcpy(temparg2, nowitem);
    }
    else
    {
        strcpy(tempop, notequalop);
        strcpy(temparg2, myzero);
    }
    genMidCode(tempop, temparg1, temparg2, blank);
    //printf("This is a condition.\n");
    //fprintf(fpw, "This is a condition.\n");
}

void loopStatement()
{
    char whileBeginLabel[FOURUNITMAX];
    char whileEndLabel[FOURUNITMAX];
    strcpy(whileBeginLabel, genLabel());
    strcpy(whileEndLabel, genLabel());
    if(symbol == WHILE)
    {
        genMidCode(labop, blank, blank, whileBeginLabel);
        getsym();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            condition();
            genMidCode(ifnotop, blank, blank, whileEndLabel);
            if(symbol == RIGHTBRACKET)
            {
                getsym();
                statement();
                genMidCode(ifop, blank, blank, whileBeginLabel);
                genMidCode(labop, blank, blank, whileEndLabel);
                //printf("This is a loop statement.\n");
                //fprintf(fpw, "This is a loop statement.\n");
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
                getsym();
            }
        }
        else
        {
            error(ERROR_EXPECTED_LEFTBRACKET);
            getsym();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
}

void assignmentStatement()
{
    if(symbol == IDENTIFIER)
    {
        char temparg1[FOURUNITMAX];
        char temparg2[FOURUNITMAX];
        char tempres[FOURUNITMAX];
        char tempIdName[FOURUNITMAX];
        strcpy(temparg1, blank);
        strcpy(temparg2, blank);
        strcpy(idName, token);
        strcpy(tempIdName, token);
        strcpy(tempres, token);
        int legal;
        identifier();
        if(symbol == ASSIGN)
        {
            legal = tabSearch(idName);
            if(legal == -1)
            {
                error(ERROR_UNDEFINED_IDENTIFIER, idName);
                getsym();
                expression();
                return ;
            }
            else
            {
                getsym();
                expression();
                strcpy(temparg1, nowitem);
                genMidCode(assignop, temparg1, blank, tempres);
            }
        }
        else if(symbol == LEFTSQUAREBRACKET)
        {
            legal = tabSearch(idName);
            if(legal == -1)
            {
                error(ERROR_UNDEFINED_IDENTIFIER, idName);
                getsym();
            }
            else if(tab.mytab[legal].obj != ARRAYOBJ)
            {
                error(ERROR_INVALID_ARRAYTYPE);
                getsym();
                expression();
            }
            else
            {
                getsym();
                expression();
                strcpy(temparg2, nowitem);
                int flag = 0;
                for(int i = 0 ; i<(signed)strlen(temparg2) ; i++)
                {
                    if(temparg2[i] >= '0' && temparg2[i] <= '9')
                    {
                        if(i == (signed)strlen(temparg2) - 1)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                if(flag == 1)
                {
                    if(atoi(temparg2) >= tab.mytab[legal].refer)
                    {
                        error(ERROR_ILLEGAL_ARRAYOUTOFBOUND);
                    }
                    else
                    {
                        ;
                    }
                }
                else
                {
                    ;
                }
            }
            if(symbol == RIGHTSQUAREBRACKET)
            {
                getsym();
                if(symbol == ASSIGN)
                {
                    getsym();
                    expression();
                    strcpy(temparg1, nowitem);
                    genMidCode(valuetoarrayop, temparg1, temparg2, tempres);
                }
                else
                {
                    error(ERROR_EXPECTED_ASSIGN);
                    getsym();
                }
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTSQUAREBRACKET);
                getsym();
            }
        }
        else
        {
            error(ERROR_ILLEGAL_ASSIGNMENTLEFT);
            getsym();
        }
    }
    else
    {
        error(ERROR_EXPECTED_IDENTIFIER);
        getsym();
    }
    //printf("This is an assignment statement.\n");
    //fprintf(fpw, "This is an assignment statement.\n");
}

void valueParamTable()
{
    inVparaTable = true;
    char tempres[FOURUNITMAX];
    expression();
    strcpy(tempres, nowitem);
    genMidCode(vparaop, (factorType == INT)?intop:charop, callLevel, tempres);
    while(symbol == COMMA)
    {
        getsym();
        expression();
        strcpy(tempres, nowitem);
        genMidCode(vparaop, (factorType == INT)?intop:charop, callLevel, tempres);
    }
    inVparaTable = false;
    //printf("This is a value parameter table.\n");
    //fprintf(fpw, "This is a value parameter table.\n");
}

void readStatement()
{
    if(symbol == SCANF)
    {
        getsym();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            strcpy(idName, token);
            identifier();
            char tempType[FOURUNITMAX];
            for(int i = tab.t ; i>=0 ; i--)
            {
                if(strcmp(idName, tab.mytab[i].name) == 0 && tab.mytab[i].obj != PARAMOBJ)
                {
                    if(tab.mytab[i].obj != VAROBJ)
                    {
                        //printf("%s %d", tab.mytab[i].name, tab.mytab[i].obj);
                        error(ERROR_ILLEGAL_SCANF);
                        break;
                    }
                    if(tab.mytab[i].typ == INTTYPE)
                    {
                        strcpy(tempType, intop);
                    }
                    else
                    {
                        strcpy(tempType, charop);
                    }
                    break;
                }
                if(i == 0)
                {
                    error(ERROR_UNDEFINED_IDENTIFIER);
                    break;
                }
            }
            genMidCode(scanfop, blank, tempType, idName);
            while(symbol == COMMA)
            {
                getsym();
                strcpy(idName, token);
                identifier();
                for(int i = tab.t ; i>=0 ; i--)
                {
                    if(strcmp(idName, tab.mytab[i].name) == 0 && tab.mytab[i].obj != PARAMOBJ)
                    {
                        if(tab.mytab[i].obj != VAROBJ)
                        {
                            error(ERROR_ILLEGAL_SCANF);
                            break;
                        }
                        if(tab.mytab[i].typ == INTTYPE)
                        {
                            strcpy(tempType, intop);
                        }
                        else
                        {
                            strcpy(tempType, charop);
                        }
                        break;
                    }
                    if(i == 0)
                    {
                        error(ERROR_UNDEFINED_IDENTIFIER);
                        break;
                    }
                }
                genMidCode(scanfop, blank, tempType, idName);
            }
            if(symbol == RIGHTBRACKET)
            {
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
                getsym();
            }
        }
        else
        {
            error(ERROR_EXPECTED_LEFTBRACKET);
            getsym();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
    //printf("This is a read statement.\n");
    //fprintf(fpw, "This is a read statement.\n");
}

void writeStatement()
{
    char temparg1[FOURUNITMAX];
    char temparg2[FOURUNITMAX];
    if(symbol == PRINTF)
    {
        getsym();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            if(token[0] == '"')
            {
                strcpy(temparg1, token);
                charString();
                if(symbol == COMMA)
                {
                    getsym();
                    expression();
                    if(symbol == RIGHTBRACKET)
                    {
                        strcpy(temparg2, nowitem);
                        genMidCode(printfop, temparg1, temparg2, (factorType == INT) ? intop: charop);
                        getsym();
                    }
                    else
                    {
                        error(ERROR_EXPECTED_RIGHTBRACKET);
                        getsym();
                    }
                }
                else if(symbol == RIGHTBRACKET)
                {
                    strcpy(temparg2, blank);
                    genMidCode(printfop, temparg1, temparg2, blank);
                    getsym();
                }
                else
                {
                    error(ERROR_ILLEGAL_PRINTF);
                    skipNextLine();
                    return ;
                }
            }
            else
            {
                strcpy(temparg1, blank);
                expression();
                if(symbol == RIGHTBRACKET)
                {
                    strcpy(temparg2, nowitem);
                    genMidCode(printfop, temparg1, temparg2, (factorType == INT) ? intop: charop);
                    getsym();
                }
                else
                {
                    error(ERROR_EXPECTED_RIGHTBRACKET);
                    getsym();
                }
            }
        }
        else
        {
            error(ERROR_EXPECTED_LEFTBRACKET);
            getsym();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
    //printf("This is a write statement.\n");
    //fprintf(fpw, "This is a write statement.\n");
}

void retStatement()
{
    char tempres[FOURUNITMAX];
    if(symbol == RETURN)
    {
        getsym();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            expression();
            strcpy(tempres, nowitem);
            if(symbol == RIGHTBRACKET)
            {
                genMidCode(retop, blank, blank, tempres);
                if(inMain)
                {
                    genMidCode(exitop, blank, blank, blank);
                }
                getsym();
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
                getsym();
            }
        }
        else if(symbol == SEMICOLON)
        {
            genMidCode(retop, blank, blank, blank);
            if(inMain)
            {
                genMidCode(exitop, blank, blank, blank);
            }
        }
        else
        {
            error(ERROR_ILLEGAL_RETURN);
            skipNextLine();
            return ;
        }
    }
    else
    {
        error(ERROR_ILLEGAL_RETURN);
        skipNextLine();
        return ;
    }
    //printf("This is a return statement.\n");
    //fprintf(fpw, "This is a return statement.\n");
}

void situationStatement()
{
    strcpy(switchEndLabel, genLabel());
    char tempIdName[FOURUNITMAX];
    //int legal;
    if(symbol == SWITCH)
    {
        getsym();
        if(symbol == LEFTBRACKET)
        {
            getsym();
            expression();
            switchType = factorType;
            strcpy(tempIdName, nowitem);
            if(symbol == RIGHTBRACKET)
            {
                getsym();
                if(symbol == LEFTCURLYBRACKET)
                {
                    getsym();
                    caseTable(tempIdName);
                    if(symbol == DEFAULT)
                    {
                        defaultStatement();
                    }
                    if(symbol == RIGHTCURLYBRACKET)
                    {
                        getsym();
                        genMidCode(labop, blank, blank, switchEndLabel);
                    }
                    else
                    {
                        error(ERROR_EXPECTED_RIGHTCURLYBRACKET);
                        getsym();
                    }
                }
                else
                {
                    error(ERROR_EXPECTED_LEFTCURLYBRACKET);
                    getsym();
                }
            }
            else
            {
                error(ERROR_EXPECTED_RIGHTBRACKET);
                getsym();
            }

        }
        else
        {
            error(ERROR_EXPECTED_LEFTBRACKET);
            getsym();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
    //printf("This is a situation statement.\n");
    //fprintf(fpw, "This is a situation statement.\n");
}

void caseTable(char tempIdName[])
{
    caseStatement(tempIdName);
    while(symbol == CASE)
    {
        caseStatement(tempIdName);
    }
    //printf("This is a case table.\n");
    //fprintf(fpw, "This is a case table.\n");
}

void caseStatement(char tempIdName[])
{
    char caseEndLabel[FOURUNITMAX];
    strcpy(caseEndLabel, genLabel());
    if(symbol == CASE)
    {
        getsym();
        constant();
        if(symbol == COLON)
        {
            getsym();
            if(caseValue[0] == '\'')
            {
                if(switchType == INT)
                {
                    error(ERROR_UNMATCH_TYPE);
                }
                genMidCode(equalop, tempIdName, (int)caseValue[1], blank);
            }
            else
            {
                if(switchType == CHAR)
                {
                    error(ERROR_UNMATCH_TYPE);
                }
                genMidCode(equalop, tempIdName, caseValue, blank);
            }
            genMidCode(ifnotop, blank, blank, caseEndLabel);
            statement();
            genMidCode(ifop, blank, blank, switchEndLabel);
            genMidCode(labop, blank, blank, caseEndLabel);
        }
        else
        {
            error(ERROR_EXPECTED_COLON);
            statement();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
    //printf("This is a case statement.\n");
    //fprintf(fpw, "This is a case statement.\n");
}

void defaultStatement()
{
    if(symbol == DEFAULT)
    {
        getsym();
        if(symbol == COLON)
        {
            getsym();
            statement();
        }
        else
        {
            error(ERROR_EXPECTED_COLON);
            statement();
        }
    }
    else
    {
        error(ERROR_ILLEGAL_STATEMENTHEAD);
        getsym();
    }
    //printf("This is a default statement.\n");
    //fprintf(fpw, "This is a default statement.\n");
}
