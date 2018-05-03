#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midcode.h"
#include "table.h"
#include "main.h"

#define LABELMAX 1000
#define TEMPVARMAX 1000

char constop[6] =   "const";
char intop[6] =     "int  ";
char charop[6] =    "char ";
char voidop[6] =    "void ";
char addop[6] =     "+    ";
char subop[6] =     "-    ";
char multop[6] =    "*    ";
char diviop[6] =    "/    ";
char assignop[6] =  "=    ";
char lessop[6] =    "<    ";
char greatop[6] =   ">    ";
char lessorequalop[6] =    "<=   ";
char greatorequalop[6] =    ">=   ";
char equalop[6] = "==   ";
char notequalop[6] = "!=   ";
char valuetoarrayop[6] = "[]=  ";
char arraytovalueop[6] = "=[]  ";
char callop[6] = "call ";
char ifnotop[6] = "jne  ";
char ifop[6] = "jmp  ";
char labop[6] = "lab: ";
char retop[6] = "ret  ";
char scanfop[6] = "scan ";
char printfop[6] = "print";
char funcop[6] = "func ";
char paraop[6] = "para ";
char blank[6] = "     ";
char myzero[6] = "0";
char vparaop[6] = "vpara";
char myendop[6] = "end  ";
char exitop[6] = "exit ";
char vendop[6] = "vend ";

FILE *fpmid = fopen("mymidcode.txt", "w");
FILE *fpfmid = fopen("midcode.txt", "w");
FOURUNIT midcode[MIDCODEMAX];
int midindex = 0;
int labelIndex = 0;
int tempVarIndex = 0;

char* int2str(int a)
{
    char *temp = (char*)malloc(sizeof(char)*TEMPVARMAX);
    sprintf(temp, "%d", a);
    return temp;
}

char* genLabel()
{
    char* label = (char*)malloc(sizeof(char)*LABELMAX);
    sprintf(label, "LABEL_%04d", labelIndex++);
    return label;
}

char* genTempVar()
{
    char* tempVar = (char*)malloc(sizeof(char)*TEMPVARMAX);
    sprintf(tempVar, "$%d", tempVarIndex++);
    return tempVar;
}

void genMidCode(char op[], char arg1[], char arg2[], char res[])
{
    //fprintf(fpmid, "%-5s, %-5s, %-5s, %-5s\n", op, arg1, arg2, res);
    strcpy(midcode[midindex].op, op);
    strcpy(midcode[midindex].arg1, arg1);
    strcpy(midcode[midindex].arg2, arg2);
    strcpy(midcode[midindex].res, res);
    midindex++;
}

void genMidCode(char op[], char arg1[], int arg2, char res[])
{
    char arg2str[TEMPVARMAX] = {'\0'};
    sprintf(arg2str, "%d", arg2);
    //fprintf(fpmid, "%-5s, %-5s, %-5s, %-5s\n", op, arg1, arg2str, res);
    strcpy(midcode[midindex].op, op);
    strcpy(midcode[midindex].arg1, arg1);
    strcpy(midcode[midindex].arg2, arg2str);
    strcpy(midcode[midindex].res, res);
    midindex++;
}

void printMidCode()
{
    for(int i = 0 ; i < midindex ; i++)
    {
        if(strcmp(midcode[i].op, blank) != 0)
        {
            fprintf(fpmid, "%-5s, %-5s, %-5s, %-5s\n", midcode[i].op, midcode[i].arg1, midcode[i].arg2, midcode[i].res);
        }
        else
        {
            ;
        }
    }
}

void formatMidcode()
{
    for(int midnum = 0 ; midnum < midindex ; midnum++)
    {
        if(strcmp(midcode[midnum].op, scanfop) == 0)
        {
            fprintf(fpfmid, "scanf(%s)\n", midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, printfop) == 0)
        {
            if(strcmp(midcode[midnum].arg1, blank) == 0)
            {
                fprintf(fpfmid, "printf(%s)\n", midcode[midnum].arg2);
            }
            else if(strcmp(midcode[midnum].arg2, blank) == 0)
            {
                fprintf(fpfmid, "printf(%s)\n", midcode[midnum].arg1);
            }
            else
            {
                fprintf(fpfmid, "printf(%s, %s)\n", midcode[midnum].arg1, midcode[midnum].arg2);
            }
        }
        else if(strcmp(midcode[midnum].op, labop) == 0)
        {
            fprintf(fpfmid, "%s:\n", midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, funcop) == 0)
        {
            fprintf(fpfmid, "%s %s()\n", midcode[midnum].arg1, midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, addop) == 0)
        {
            fprintf(fpfmid, "%s = %s %c %s\n", midcode[midnum].res, midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, subop) == 0)
        {
            fprintf(fpfmid, "%s = %s %c %s\n", midcode[midnum].res, midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, multop) == 0)
        {
            fprintf(fpfmid, "%s = %s %c %s\n", midcode[midnum].res, midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, diviop) == 0)
        {
            fprintf(fpfmid, "%s = %s %c %s\n", midcode[midnum].res, midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, assignop) == 0)
        {
            fprintf(fpfmid, "%s = %s\n", midcode[midnum].res, midcode[midnum].arg1);
        }
        else if(strcmp(midcode[midnum].op, equalop) == 0)
        {
            fprintf(fpfmid, "%s %c%c %s\n", midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].op[1], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, notequalop) == 0)
        {
            fprintf(fpfmid, "%s %c%c %s\n", midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].op[1], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, lessop) == 0)
        {
            fprintf(fpfmid, "%s %c %s\n", midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, greatop) == 0)
        {
            fprintf(fpfmid, "%s %c %s\n", midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, lessorequalop) == 0)
        {
            fprintf(fpfmid, "%s %c%c %s\n", midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].op[1], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, greatorequalop) == 0)
        {
            fprintf(fpfmid, "%s %c%c %s\n", midcode[midnum].arg1, midcode[midnum].op[0], midcode[midnum].op[1], midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, ifnotop) == 0)
        {
            fprintf(fpfmid, "BZ %s\n", midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, ifop) == 0)
        {
            fprintf(fpfmid, "GOTO %s\n", midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, constop) == 0)
        {
            fprintf(fpfmid, "const %s %s = %s\n", midcode[midnum].arg1, midcode[midnum].res, midcode[midnum].arg2);
        }
        else if((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) == 0)
        {
            fprintf(fpfmid, "var %s %s\n", midcode[midnum].op, midcode[midnum].res);
        }
        else if((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) != 0)
        {
            fprintf(fpfmid, "array %s %s[%s]\n", midcode[midnum].op, midcode[midnum].res, midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, paraop) == 0)
        {
            fprintf(fpfmid, "para %s %s\n", midcode[midnum].arg1, midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, callop) == 0)
        {
            fprintf(fpfmid, "call %s\n", midcode[midnum].arg1);
            if(strcmp(midcode[midnum].res, blank) != 0)
            {
                fprintf(fpfmid, "%s = RET\n", midcode[midnum].res);
            }
        }
        else if(strcmp(midcode[midnum].op, vparaop) == 0)
        {
            fprintf(fpfmid, "push %s\n", midcode[midnum].res);
        }
        else if(strcmp(midcode[midnum].op, myendop) == 0)
        {
            ;
        }
        else if(strcmp(midcode[midnum].op, retop) == 0)
        {
            if(strcmp(midcode[midnum].res, blank) != 0)
            {
                fprintf(fpfmid, "ret %s\n", midcode[midnum].res);
            }
        }
        else if(strcmp(midcode[midnum].op, valuetoarrayop) == 0)
        {
            fprintf(fpfmid, "%s[%s] = %s\n", midcode[midnum].res, midcode[midnum].arg2, midcode[midnum].arg1);
        }
        else if(strcmp(midcode[midnum].op, arraytovalueop) == 0)
        {
            fprintf(fpfmid, "%s = %s[%s]\n", midcode[midnum].res, midcode[midnum].arg1, midcode[midnum].arg2);
        }
        else if(strcmp(midcode[midnum].op, exitop) == 0)
        {
            ;
        }
        else if(strcmp(midcode[midnum].op, vendop) == 0)
        {
            ;
        }
    }
}
