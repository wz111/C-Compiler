#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include "table.h"
#include "midcode.h"
#include "optimization.h"

#define LCONSTMAX 512
#define GCONSTMAX 512
#define VARMAX 512

int whichFunc = -1;

char REG[8][4] = {
"$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7"
};

FILE *fpmips = fopen("tarcode.asm", "w");

typedef struct globalconst{
    char name[GCONSTMAX];
    char value[GCONSTMAX];
}GCONST;

typedef struct localVar{
    char name[VARMAX];
    int addr;
}LVAR;

typedef struct localconst{
    char name[GCONSTMAX];
    char value[GCONSTMAX];
}LCONST;

GCONST gconst[GCONSTMAX];
int gIndex = 0;
GCONST lconst[LCONSTMAX];
int lIndex = 0;
LVAR lvar[VARMAX];
int lvarnum;
int offset = 0;
int midnum = 0;     //the num of the finished midcode
int strIndex = 0;
int paranum = 0;
char currentFunc[VARMAX] = {'\0'};
char retReg[VARMAX];
int vparamnum = 0;

int getaddr(char name[])
{
    for(int i = 0 ; i<lvarnum ; i++)
    {
        if(strcmp(lvar[i].name, name) == 0)
        {
            return lvar[i].addr;
        }
    }
    return -1;
}

void lvarInsert(char name[])
{
    strcpy(lvar[lvarnum].name, name);
    lvar[lvarnum].addr = offset;
    //fprintf(fpmips, "\n####%s %d\n", name, offset);
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    offset+=4;
    lvarnum++;
}

void gconstDef()
{
    for(int i = 0 ; strcmp(midcode[i].op, constop) == 0 ; i++)
    {
        strcpy(gconst[gIndex].name, midcode[i].res);
        strcpy(gconst[gIndex].value, midcode[i].arg2);
        fprintf(fpmips, "%s:\t.word\t%s\n", midcode[i].res, midcode[i].arg2);
        gIndex++;
    }
}

void gvarDef()
{
    for(int i = gIndex ; (strcmp(midcode[i].op, intop) == 0 || strcmp(midcode[i].op, charop) == 0) ; i++)
    {
        if(strcmp(midcode[i].arg2, blank) == 0)  //simple var
        {
            strcpy(gconst[gIndex].name, midcode[i].res);
            strcpy(gconst[gIndex].value, midcode[i].arg2);
            fprintf(fpmips, "%s:\t.space\t4\n", midcode[i].res);
            gIndex++;
        }
        else if(strcmp(midcode[i].arg2, blank) != 0)  //array
        {
            strcpy(gconst[gIndex].name, midcode[i].res);
            strcpy(gconst[gIndex].value, blank);
            fprintf(fpmips, "%s:\t.space\t%d\n", midcode[i].res, atoi(midcode[i].arg2) * 4);
            gIndex++;
        }
    }
    midnum = gIndex;
}

void initasm()
{
    fprintf(fpmips, ".data\n");
    gconstDef();
    gvarDef();
    for(int i = midnum ; i<midindex ; i++)
    {
        if(strcmp(midcode[i].op, printfop) == 0)
        {
            if(strcmp(midcode[i].arg1, blank) != 0)
            {
                char temp[512] = {'\0'};
                sprintf(temp, "$string%d", strIndex);
                strcpy(gconst[midnum].name, temp);
                strcpy(gconst[midnum].value, midcode[i].arg1);
                strcpy(midcode[i].arg1, temp);
                fprintf(fpmips, "%s:\t.asciiz\t%s\n", temp, gconst[midnum].value);
                gIndex++;
                strIndex++;
            }
        }
    }
    fprintf(fpmips, ".text\n");
    fprintf(fpmips, ".globl main\n");
    fprintf(fpmips, "\t\tj\tmain\n");
}

void func_mips()
{
    //fprintf(fpmips, "#func init:\n");
    fprintf(fpmips, "%s:\n", midcode[midnum].res);
    strcpy(currentFunc, midcode[midnum].res);
    if(strcmp(midcode[midnum].res, "main") == 0)
    {
        fprintf(fpmips, "\t\tmove\t$fp\t$sp\n");
        fprintf(fpmips, "\t\tsw\t$fp\t($sp)\n"); //  use $ra save return address, $fp save the pre $sp
        fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    }
    int legal = tabSearch(midcode[midnum].res);
    int parameternum = -1;
    if(legal != -1)
    {
        for(int i = legal+1 ; tab.mytab[i].obj == PARAMOBJ ; i++)
        {
            parameternum++;
        }
    }
    fprintf(fpmips, "\t\tsw\t$ra\t($sp)\n");
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    if(parameternum == -1)
    {
        offset = 8;
    }
    else
    {
        fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t%d\n", 4 * parameternum);
        offset = 8 + parameternum * 4;
    }
    paranum = 0;
    /*
    if(strcmp(midcode[midnum].res, "main") == 0)
    {
        fprintf(fpmips, "\t\tmove\t$fp\t$sp\n");
        fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t8\n");
    }
    else
    {
        fprintf(fpmips, "\t\tmove\t$fp\t$k0\n");
    }
    fprintf(fpmips, "\t\tsw\t$ra\t-4($fp)\n");
    offset = 8;
    */
}

void para_mips()
{
    if(paranum < 4)
    {
        fprintf(fpmips, "\t\tmove\t$t0\t$a%d\n", paranum);
        fprintf(fpmips, "\t\tsw\t$t0\t%d($fp)\n", -8-paranum*4);
        offset = 8 + paranum * 4;
    }
    lvarInsert(midcode[midnum].res);
    paranum++;
}

void vpara_mips()
{
    int addr = -1 * getaddr(midcode[midnum].res);
    if(vparamnum < 4)
    {
        if(addr == 1)
        {
            if((midcode[midnum].res[0] >= '0' && midcode[midnum].res[0] <= '9') || midcode[midnum].res[0] == '-')
            {
                fprintf(fpmips, "\t\tli\t$a%d\t%s\n", vparamnum, midcode[midnum].res);
            }
            else
            {
                fprintf(fpmips, "\t\tla\t$t0\t%s\n", midcode[midnum].res);
                fprintf(fpmips, "\t\tlw\t$a%d\t($t0)\n", vparamnum);
            }
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$a%d\t%d($fp)\n", vparamnum, addr);
        }
    }
    else
    {
        if(addr == 1)
        {
            if((midcode[midnum].res[0] >= '0' && midcode[midnum].res[0] <= '9') || midcode[midnum].res[0] == '-')
            {
                fprintf(fpmips, "\t\tli\t$t8\t%s\n", midcode[midnum].res);
            }
            else
            {
                fprintf(fpmips, "\t\tla\t$t0\t%s\n", midcode[midnum].res);
                fprintf(fpmips, "\t\tlw\t$t8\t($t0)\n");
            }
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$t8\t%d($fp)\n", addr);
        }
        fprintf(fpmips, "\t\tsw\t$t8\t%d($sp)\n", -1*((vparamnum - 4) * 4 + 24));
    }
    vparamnum++;
}

void end_mips()
{
    lvarnum = 0;
}

void call_mips()
{
    vparamnum = 0;
    //fprintf(fpmips, "#callfunc begin: save the situation\n");
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    fprintf(fpmips, "\t\tsw\t$fp\t($sp)\n"); //  use $ra save return address, $fp save the pre $sp
    fprintf(fpmips, "\t\tmove\t$fp\t$sp\n");
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    fprintf(fpmips, "\t\tjal\t%s\n", midcode[midnum].arg1);
    strcpy(retReg, midcode[midnum].res);
    if(strcmp(midcode[midnum].res, blank) != 0)
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$a2\t%d($fp)\n", addr);
    }
}

void o_call_mips()
{
    vparamnum = 0;
    //fprintf(fpmips, "#callfunc begin: save the situation\n");
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t8\n");
    fprintf(fpmips, "\t\tsw\t$fp\t4($sp)\n"); //  use $ra save return address, $fp save the pre $sp
    fprintf(fpmips, "\t\taddi\t$fp\t$sp\t4\n");
    fprintf(fpmips, "\t\tjal\t%s\n", midcode[midnum].arg1);
    strcpy(retReg, midcode[midnum].res);
    if(strcmp(midcode[midnum].res, blank) != 0)
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$a2\t%d($fp)\n", addr);
    }
}

void ret_mips()
//use $a2 to save the return value
{
    //fprintf(fpmips, " #return statement:\n");
    if(strcmp(midcode[midnum].res, blank) != 0)
    {
        if((midcode[midnum].res[0] >= '0' && midcode[midnum].res[0] <= '9') || midcode[midnum].res[0] == '-')
        {
            fprintf(fpmips, "\t\tli\t$a2\t%s\n", midcode[midnum].res);
        }
        else
        {
            int addr = -1 * getaddr(midcode[midnum].res);
            if(addr == 1)   //global
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
                fprintf(fpmips, "\t\tlw\t$a2\t($a0)\n");
            }
            else
            {
                fprintf(fpmips, "\t\tlw\t$a2\t%d($fp)\n", addr);
            }
        }
    }
    if(strcmp(currentFunc, "main") == 0)
    {
        ;
    }
    else
    {
        fprintf(fpmips, "\t\tlw\t$ra\t-4($fp)\n");
        fprintf(fpmips, "\t\tmove\t$t0\t$ra\n");
        fprintf(fpmips, "\t\tmove\t$sp\t$fp\n");
        fprintf(fpmips, "\t\tlw\t$fp\t($fp)\n");
        fprintf(fpmips, "\t\tjr\t$t0\n");
    }
}

void o_ret_mips()
//use $a2 to save the return value
{
    //fprintf(fpmips, " #return statement:\n");
    if(strcmp(midcode[midnum].res, blank) != 0)
    {
        if((midcode[midnum].res[0] >= '0' && midcode[midnum].res[0] <= '9') || midcode[midnum].res[0] == '-')
        {
            fprintf(fpmips, "\t\tli\t$a2\t%s\n", midcode[midnum].res);
        }
        else
        {
            int addr = -1 * getaddr(midcode[midnum].res);
            if(addr == 1)   //global
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
                fprintf(fpmips, "\t\tlw\t$a2\t($a0)\n");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                    {
                        fprintf(fpmips, "\t\tmove\t$a2\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$a2\t%d($fp)\n", addr);
                }
            }
        }
    }
    if(strcmp(currentFunc, "main") == 0)
    {
        ;
    }
    else
    {
        fprintf(fpmips, "\t\tlw\t$ra\t-4($fp)\n");
        fprintf(fpmips, "\t\tmove\t$t0\t$ra\n");
        fprintf(fpmips, "\t\tmove\t$sp\t$fp\n");
        fprintf(fpmips, "\t\tlw\t$fp\t($fp)\n");
        fprintf(fpmips, "\t\tjr\t$t0\n");
    }
}

void localConst_mips()
{
    strcpy(lvar[lvarnum].name, midcode[midnum].res);
    lvar[lvarnum].addr = offset;
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    fprintf(fpmips, "\t\tli\t$t0\t%d\n", atoi(midcode[midnum].arg2));
    fprintf(fpmips, "\t\tsw\t$t0\t%d($fp)\n", -1*offset);
    lvarnum++;
    offset+=4;
}

void localVar_mips()
{
    strcpy(lvar[lvarnum].name, midcode[midnum].res);
    lvar[lvarnum].addr = offset;
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t4\n");
    lvarnum++;
    offset+=4;
}

void o_localVar_mips()
{
    strcpy(lvar[lvarnum].name, midcode[midnum].res);
    lvar[lvarnum].addr = offset;
    lvarnum++;
    offset+=4;
}

void o_localConst_mips()
{
    strcpy(lvar[lvarnum].name, midcode[midnum].res);
    lvar[lvarnum].addr = offset;
    fprintf(fpmips, "\t\tli\t$t0\t%d\n", atoi(midcode[midnum].arg2));
    fprintf(fpmips, "\t\tsw\t$t0\t%d($fp)\n", -1*offset);
    lvarnum++;
    offset+=4;
}

void localArray_mips()
{
    strcpy(lvar[lvarnum].name, midcode[midnum].res);
    lvar[lvarnum].addr = offset;
    fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t%d\n", atoi(midcode[midnum].arg2)*4);
    lvarnum++;
    offset+=atoi(midcode[midnum].arg2)*4;
}

void add_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tadd\t$v0\t$v0\t$v1\n");
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
        }
    }
}

void sub_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tsub\t$v0\t$v0\t$v1\n");
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
        }
    }
}

void mul_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tmul\t$v0\t$v0\t$v1\n");
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
        }
    }
}

void div_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tdiv\t$v0\t$v0\t$v1\n");
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
        }
    }
}

void printf_mips()
{
    if(strcmp(midcode[midnum].arg1, blank) != 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t4\n");
        fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
        fprintf(fpmips, "\t\tsyscall\n");
    }
    if(strcmp(midcode[midnum].arg2, blank) != 0)
    {
        if(midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9')
        {
            fprintf(fpmips, "\t\tli\t$a0\t%s\n", midcode[midnum].arg2);
        }
        else
        {
            int addr = -1 * getaddr(midcode[midnum].arg2);
            if(addr == 1)
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg2);
                fprintf(fpmips, "\t\tlw\t$a0\t($a0)\n" );
            }
            else
            {
                fprintf(fpmips, "\t\tlw\t$a0\t%d($fp)\n", addr);
            }
        }
    }
    else
    {
        ;
    }
    if(strcmp(midcode[midnum].res, intop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t1\n");
        fprintf(fpmips, "\t\tsyscall\n");
    }
    else if(strcmp(midcode[midnum].res, charop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t11\n");
        fprintf(fpmips, "\t\tsyscall\n");
    }
    else
    {
        ;
    }
}

void o_printf_mips()
{
    if(strcmp(midcode[midnum].arg1, blank) != 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t4\n");
        fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
        fprintf(fpmips, "\t\tsyscall\n");
    }
    if(strcmp(midcode[midnum].arg2, blank) != 0)
    {
        if(midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9')
        {
            fprintf(fpmips, "\t\tli\t$a0\t%s\n", midcode[midnum].arg2);
        }
        else
        {
            int addr = -1 * getaddr(midcode[midnum].arg2);
            if(addr == 1)
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg2);
                fprintf(fpmips, "\t\tlw\t$a0\t($a0)\n" );
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                    {
                        fprintf(fpmips, "\t\tmove\t$a0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$a0\t%d($fp)\n", addr);
                }
            }
        }
    }
    else
    {
        ;
    }
    if(strcmp(midcode[midnum].res, intop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t1\n");
        fprintf(fpmips, "\t\tsyscall\n");
    }
    else if(strcmp(midcode[midnum].res, charop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t11\n");
        fprintf(fpmips, "\t\tsyscall\n");
    }
    else
    {
        ;
    }
}

void scanf_mips()
{
    if(strcmp(midcode[midnum].arg2, intop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t5\n");
    }
    else if(strcmp(midcode[midnum].arg2, charop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t12\n");
    }
    fprintf(fpmips, "\t\tsyscall\n");
    int addr = -1 * getaddr(midcode[midnum].res);
    if(addr == 1)
    {
        fprintf(fpmips, "\t\tla\t$t0\t%s\n", midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t($t0)\n");
    }
    else
    {
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
}

void o_scanf_mips()
{
    if(strcmp(midcode[midnum].arg2, intop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t5\n");
    }
    else if(strcmp(midcode[midnum].arg2, charop) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v0\t12\n");
    }
    fprintf(fpmips, "\t\tsyscall\n");
    int addr = -1 * getaddr(midcode[midnum].res);
    if(addr == 1)
    {
        fprintf(fpmips, "\t\tla\t$t0\t%s\n", midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t($t0)\n");
    }
    else
    {
        bool useReg = false;
        for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
        {
            if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
            {
                fprintf(fpmips, "\t\tmove\t$s%d\t$v0\n", gRegQueue[whichFunc][i].reg);
                if(callBlock[whichFunc] == 1)
                {
                    fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
                }
                useReg = true;
                break;
            }
        }
        if(!useReg)
        {
            fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
        }
    }
}

void label_mips()
{
    fprintf(fpmips, "%s:\n", midcode[midnum].res);
}

void equal_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v1\t0\n");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tbne\t$v0\t$v1\t");
}

void notequal_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v1\t0\n");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tbeq\t$v0\t$v1\t");
}

void lessequal_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v1\t0\n");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {

            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tbgt\t$v0\t$v1\t");
}

void greatequal_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v1\t0\n");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tblt\t$v0\t$v1\t");
}

void less_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v1\t0\n");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tbge\t$v0\t$v1\t");
}

void great_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        fprintf(fpmips, "\t\tli\t$v1\t0\n");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
        }
    }
    fprintf(fpmips, "\t\tble\t$v0\t$v1\t");
}

void assign_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($a0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
        }
    }
    int addr = -1 * getaddr(midcode[midnum].res);
    if(addr == 1)
    {
        fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
    }
    else
    {
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
}

void o_assign_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($a0)\n");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                ;
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                }
            }
        }
    }
    int addr = -1 * getaddr(midcode[midnum].res);
    if(addr == 1)
    {
        fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
    }
    else
    {
        for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
        {
            if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
            {
                fprintf(fpmips, "\t\tmove\t$s%d\t$v0\n", gRegQueue[whichFunc][i].reg);
                break;
            }
        }
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
}

void o_add_mips()
{
    char reg1[100];
    char reg2[100];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v1\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tadd\t$v0\t%s\t%s\n", reg1, reg2);
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                {
                    fprintf(fpmips, "\t\tmove\t$s%d\t$v0\n", gRegQueue[whichFunc][i].reg);
                    if(callBlock[whichFunc] == 1)
                    {
                        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
                    }
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
            }
        }
    }
}

void o_sub_mips()
{
    char reg1[100];
    char reg2[100];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v1\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tsub\t$v0\t%s\t%s\n", reg1, reg2);
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                {
                    fprintf(fpmips, "\t\tmove\t$s%d\t$v0\n", gRegQueue[whichFunc][i].reg);
                    if(callBlock[whichFunc] == 1)
                    {
                        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
                    }
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
            }
        }
    }
}

void o_mul_mips()
{
    char reg1[100];
    char reg2[100];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v1\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tmul\t$v0\t%s\t%s\n", reg1, reg2);
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                {
                    fprintf(fpmips, "\t\tmove\t$s%d\t$v0\n", gRegQueue[whichFunc][i].reg);
                    if(callBlock[whichFunc] == 1)
                    {
                        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
                    }
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
            }
        }
    }
}

void o_div_mips()
{
    char reg1[100];
    char reg2[100];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v1\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tdiv\t$v0\t%s\t%s\n", reg1, reg2);
    if(midcode[midnum].res[0] == '$')
    {
        lvarInsert(midcode[midnum].res);
        int addr = -1 * getaddr(midcode[midnum].res);
        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].res);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                {
                    fprintf(fpmips, "\t\tmove\t$s%d\t$v0\n", gRegQueue[whichFunc][i].reg);
                    if(callBlock[whichFunc] == 1)
                    {
                        fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
                    }
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", addr);
            }
        }
    }
}

void o_vpara_mips()
{
    int addr = -1 * getaddr(midcode[midnum].res);
    if(vparamnum < 4)
    {
        if(addr == 1)
        {
            if((midcode[midnum].res[0] >= '0' && midcode[midnum].res[0] <= '9') || midcode[midnum].res[0] == '-')
            {
                fprintf(fpmips, "\t\tli\t$a%d\t%s\n", vparamnum, midcode[midnum].res);
            }
            else
            {
                fprintf(fpmips, "\t\tla\t$t0\t%s\n", midcode[midnum].res);
                fprintf(fpmips, "\t\tlw\t$a%d\t($t0)\n", vparamnum);
            }
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                {
                    fprintf(fpmips, "\t\tmove\t$a%d\t$s%d\n", vparamnum, gRegQueue[whichFunc][i].reg);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$a%d\t%d($fp)\n", vparamnum, addr);
            }
        }
    }
    else
    {
        if(addr == 1)
        {
            if((midcode[midnum].res[0] >= '0' && midcode[midnum].res[0] <= '9') || midcode[midnum].res[0] == '-')
            {
                fprintf(fpmips, "\t\tli\t$t8\t%s\n", midcode[midnum].res);
            }
            else
            {
                fprintf(fpmips, "\t\tla\t$t0\t%s\n", midcode[midnum].res);
                fprintf(fpmips, "\t\tlw\t$t8\t($t0)\n");
            }
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].res)
                {
                    fprintf(fpmips, "\t\tmove\t$t8\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$t8\t%d($fp)\n", addr);
            }
        }
        fprintf(fpmips, "\t\tsw\t$t8\t%d($sp)\n", -1*((vparamnum - 4) * 4 + 24));
    }
    vparamnum++;
}

void o_equal_mips()
{
    char reg1[5];
    char reg2[5];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0
               || strcmp(midcode[midnum-1].op, assignop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else if(strcmp(midcode[midnum-1].op, labop) == 0
                    && (strcmp(midcode[midnum-2].op, addop) == 0 || strcmp(midcode[midnum-2].op, subop) == 0
                        || strcmp(midcode[midnum-2].op, multop) == 0 || strcmp(midcode[midnum-2].op, diviop) == 0
                        || strcmp(midcode[midnum-2].op, assignop) == 0)
                    && (strcmp(midcode[midnum-2].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        //fprintf(fpmips, "\t\tli\t$v1\t0\n");
        strcpy(reg2, "$0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tbne\t%s\t%s\t", reg1, reg2);
}

void o_notequal_mips()
{
    char reg1[5];
    char reg2[5];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0
               || strcmp(midcode[midnum-1].op, assignop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else if(strcmp(midcode[midnum-1].op, labop) == 0
                    && (strcmp(midcode[midnum-2].op, addop) == 0 || strcmp(midcode[midnum-2].op, subop) == 0
                        || strcmp(midcode[midnum-2].op, multop) == 0 || strcmp(midcode[midnum-2].op, diviop) == 0
                        || strcmp(midcode[midnum-2].op, assignop) == 0)
                    && (strcmp(midcode[midnum-2].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        //fprintf(fpmips, "\t\tli\t$v1\t0\n");
        strcpy(reg2, "$0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tbeq\t%s\t%s\t", reg1, reg2);
}

void o_lessequal_mips()
{
    char reg1[5];
    char reg2[5];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0
               || strcmp(midcode[midnum-1].op, assignop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else if(strcmp(midcode[midnum-1].op, labop) == 0
                    && (strcmp(midcode[midnum-2].op, addop) == 0 || strcmp(midcode[midnum-2].op, subop) == 0
                        || strcmp(midcode[midnum-2].op, multop) == 0 || strcmp(midcode[midnum-2].op, diviop) == 0
                        || strcmp(midcode[midnum-2].op, assignop) == 0)
                    && (strcmp(midcode[midnum-2].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        //fprintf(fpmips, "\t\tli\t$v1\t0\n");
        strcpy(reg2, "$0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tbgt\t%s\t%s\t", reg1, reg2);
}

void o_greatequal_mips()
{
    char reg1[5];
    char reg2[5];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0
               || strcmp(midcode[midnum-1].op, assignop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else if(strcmp(midcode[midnum-1].op, labop) == 0
                    && (strcmp(midcode[midnum-2].op, addop) == 0 || strcmp(midcode[midnum-2].op, subop) == 0
                        || strcmp(midcode[midnum-2].op, multop) == 0 || strcmp(midcode[midnum-2].op, diviop) == 0
                        || strcmp(midcode[midnum-2].op, assignop) == 0)
                    && (strcmp(midcode[midnum-2].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        //fprintf(fpmips, "\t\tli\t$v1\t0\n");
        strcpy(reg2, "$0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tblt\t%s\t%s\t", reg1, reg2);
}

void o_less_mips()
{
    char reg1[5];
    char reg2[5];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0
               || strcmp(midcode[midnum-1].op, assignop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else if(strcmp(midcode[midnum-1].op, labop) == 0
                    && (strcmp(midcode[midnum-2].op, addop) == 0 || strcmp(midcode[midnum-2].op, subop) == 0
                        || strcmp(midcode[midnum-2].op, multop) == 0 || strcmp(midcode[midnum-2].op, diviop) == 0
                        || strcmp(midcode[midnum-2].op, assignop) == 0)
                    && (strcmp(midcode[midnum-2].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        //fprintf(fpmips, "\t\tli\t$v1\t0\n");
        strcpy(reg2, "$0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tbge\t%s\t%s\t", reg1, reg2);
}

void o_great_mips()
{
    char reg1[5];
    char reg2[5];
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
        strcpy(reg1, "$v0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg1);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($v0)\n");
            strcpy(reg1, "$v0");
        }
        else
        {
            if((strcmp(midcode[midnum-1].op, addop) == 0 || strcmp(midcode[midnum-1].op, subop) == 0
               || strcmp(midcode[midnum-1].op, multop) == 0 || strcmp(midcode[midnum-1].op, diviop) == 0
               || strcmp(midcode[midnum-1].op, assignop) == 0)
               && (strcmp(midcode[midnum-1].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else if(strcmp(midcode[midnum-1].op, labop) == 0
                    && (strcmp(midcode[midnum-2].op, addop) == 0 || strcmp(midcode[midnum-2].op, subop) == 0
                        || strcmp(midcode[midnum-2].op, multop) == 0 || strcmp(midcode[midnum-2].op, diviop) == 0
                        || strcmp(midcode[midnum-2].op, assignop) == 0)
                    && (strcmp(midcode[midnum-2].res, midcode[midnum].arg1) == 0))
            {
                strcpy(reg1, "$v0");
            }
            else
            {
                bool useReg = false;
                for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
                {
                    if(gRegQueue[whichFunc][i].name == midcode[midnum].arg1)
                    {
                        //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                        strcpy(reg1, REG[gRegQueue[whichFunc][i].reg]);
                        useReg = true;
                        break;
                    }
                }
                if(!useReg)
                {
                    fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", addr);
                    strcpy(reg1, "$v0");
                }
            }
        }
    }
    if((midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9') || midcode[midnum].arg2[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v1\t%s\n", midcode[midnum].arg2);
        strcpy(reg2, "$v1");
    }
    else if(strcmp(midcode[midnum].arg2, blank) == 0)
    {
        //fprintf(fpmips, "\t\tli\t$v1\t0\n");
        strcpy(reg2, "$0");
    }
    else
    {
        int addr = -1 * getaddr(midcode[midnum].arg2);
        if(addr == 1)
        {
            fprintf(fpmips, "\t\tla\t$v1\t%s\n", midcode[midnum].arg2);
            fprintf(fpmips, "\t\tlw\t$v1\t($v1)\n");
            strcpy(reg2, "$v1");
        }
        else
        {
            bool useReg = false;
            for(int i = 0 ; i < (signed)gRegQueue[whichFunc].size() ; i++)
            {
                if(gRegQueue[whichFunc][i].name == midcode[midnum].arg2)
                {
                    //fprintf(fpmips, "\t\tmove\t$v0\t$s%d\n", gRegQueue[whichFunc][i].reg);
                    strcpy(reg2, REG[gRegQueue[whichFunc][i].reg]);
                    useReg = true;
                    break;
                }
            }
            if(!useReg)
            {
                fprintf(fpmips, "\t\tlw\t$v1\t%d($fp)\n", addr);
                strcpy(reg2, "$v1");
            }
        }
    }
    fprintf(fpmips, "\t\tble\t%s\t%s\t", reg1, reg2);
}

void jne_mips()
{
    fprintf(fpmips, "%s\n", midcode[midnum].res);
}

void jmp_mips()
{
    fprintf(fpmips, "\t\tj\t%s\n", midcode[midnum].res);
}

void valuetoarrayAssign_mips()
{
    if((midcode[midnum].arg1[0] >= '0' && midcode[midnum].arg1[0] <= '9') || midcode[midnum].arg1[0] == '-')
    {
        fprintf(fpmips, "\t\tli\t$v0\t%s\n", midcode[midnum].arg1);
    }
    else
    {
        int tempaddr = -1  * getaddr(midcode[midnum].arg1);
        if(tempaddr == 1)
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tlw\t$v0\t($a0)\n");
        }
        else
        {
            fprintf(fpmips, "\t\tlw\t$v0\t%d($fp)\n", tempaddr);
        }
    }
    int addr = -1 * getaddr(midcode[midnum].res);
    if(addr == 1)
    {
        if(midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9')
        {
            fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tsw\t$v0\t%d($a0)\n", atoi(midcode[midnum].arg2)*4);
        }
        else
        {
            int addr1 = -1 * getaddr(midcode[midnum].arg2);
            if(addr1 == 1)
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg2);
            }
            else
            {
                fprintf(fpmips, "\t\tlw\t$a0\t%d($fp)\n", addr1);
            }
            fprintf(fpmips, "\t\tmul\t$a0\t$a0\t4\n");
            fprintf(fpmips, "\t\tla\t$a1\t%s\n", midcode[midnum].res);
            fprintf(fpmips, "\t\tadd\t$a0\t$a0\t$a1\n");
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
    }
    else
    {
        if(midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9')
        {
            fprintf(fpmips, "\t\tla\t$a1\t%d($fp)\n", addr);
            fprintf(fpmips, "\t\tla\t$a0\t%d($a1)\n", atoi(midcode[midnum].arg2)*-4);
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
        else
        {
            int addr1 = -1 * getaddr(midcode[midnum].arg2);
            if(addr1 == 1)
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg2);
            }
            else
            {
                fprintf(fpmips, "\t\tlw\t$a0\t%d($fp)\n", addr1);
            }
            fprintf(fpmips, "\t\tla\t$a1\t%d($fp)\n", addr);
            fprintf(fpmips, "\t\tmul\t$a0\t$a0\t-4\n");
            fprintf(fpmips, "\t\tadd\t$a0\t$a0\t$a1\n");
            fprintf(fpmips, "\t\tsw\t$v0\t($a0)\n");
        }
    }
}

void arraytovalueAssign_mips()
{
    int addr = -1 * getaddr(midcode[midnum].arg1);
    if(addr == 1)
    {
        if(midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9')
        {
            fprintf(fpmips, "\t\tla\t$a1\t%s\n", midcode[midnum].arg1);
            fprintf(fpmips, "\t\tla\t$a0\t%d($a1)\n", atoi(midcode[midnum].arg2)*4);
        }
        else
        {
            int addr1 = -1 * getaddr(midcode[midnum].arg2);
            if(addr1 == 1)
            {
                fprintf(fpmips, "\t\tla\t$a1\t%s\n", midcode[midnum].arg2);
                fprintf(fpmips, "\t\tlw\t$a1\t($a1)\n");
                fprintf(fpmips, "\t\tmul\t$a1\t$a1\t4\n");
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
                fprintf(fpmips, "\t\tadd\t$a0\t$a0\t$a1\n");  //get the addr of the array element

            }
            else
            {
                fprintf(fpmips, "\t\tlw\t$a1\t%d($fp)\n", addr1);
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg1);
                fprintf(fpmips, "\t\tmul\t$a1\t$a1\t4\n");
                fprintf(fpmips, "\t\tadd\t$a0\t$a0\t$a1\n");  //get the addr of the array element
            }
        }
    }
    else
    {
        if(midcode[midnum].arg2[0] >= '0' && midcode[midnum].arg2[0] <= '9')
        {
            fprintf(fpmips, "\t\tla\t$a0\t%d($fp)\n", addr - atoi(midcode[midnum].arg2)*4);
        }
        else
        {
            int addr1 = -1 * getaddr(midcode[midnum].arg2);
            if(addr1 == 1)
            {
                fprintf(fpmips, "\t\tla\t$a0\t%s\n", midcode[midnum].arg2);
                fprintf(fpmips, "\t\tla\t$a1\t%d($fp)\n", addr);
                fprintf(fpmips, "\t\tmul\t$a0\t$a0\t-4\n");
                fprintf(fpmips, "\t\tadd\t$a0\t$a0\t$a1\n");
            }
            else
            {
                fprintf(fpmips, "\t\tlw\t$a0\t%d($fp)\n", addr1);
                fprintf(fpmips, "\t\tla\t$a1\t%d($fp)\n", addr);
                fprintf(fpmips, "\t\tmul\t$a0\t$a0\t-4\n");
                fprintf(fpmips, "\t\tadd\t$a0\t$a0\t$a1\n");
            }
        }
    }
    fprintf(fpmips, "\t\tlw\t$v0\t($a0)\n");
    lvarInsert(midcode[midnum].res);
    int tempresaddr = -1 * getaddr(midcode[midnum].res);
    fprintf(fpmips, "\t\tsw\t$v0\t%d($fp)\n", tempresaddr);
}

void exit_mips()
{
    fprintf(fpmips, "\t\tli\t$v0\t10\n");
    fprintf(fpmips, "\t\tsyscall\n");
}

void vend_mips()
{
    lvarnum = 0;
    fprintf(fpmips, "\t\tlw\t$ra\t-4($fp)\n");
    fprintf(fpmips, "\t\tmove\t$t0\t$ra\n");
    fprintf(fpmips, "\t\tmove\t$sp\t$fp\n");
    fprintf(fpmips, "\t\tlw\t$fp\t($fp)\n");
    fprintf(fpmips, "\t\tjr\t$t0\n");
}

void genTargetCode()
{
    initasm();
    for( ; midnum < midindex ; midnum++)
    {
        if(strcmp(midcode[midnum].op, scanfop) == 0)
        {
            scanf_mips();
        }
        else if(strcmp(midcode[midnum].op, printfop) == 0)
        {
            printf_mips();
        }
        else if(strcmp(midcode[midnum].op, labop) == 0)
        {
            label_mips();
        }
        else if(strcmp(midcode[midnum].op, funcop) == 0)
        {
            whichFunc++;
            func_mips();
        }
        else if(strcmp(midcode[midnum].op, addop) == 0)
        {
            add_mips();
        }
        else if(strcmp(midcode[midnum].op, subop) == 0)
        {
            sub_mips();
        }
        else if(strcmp(midcode[midnum].op, multop) == 0)
        {
            mul_mips();
        }
        else if(strcmp(midcode[midnum].op, diviop) == 0)
        {
            div_mips();
        }
        else if(strcmp(midcode[midnum].op, assignop) == 0)
        {
            assign_mips();
        }
        else if(strcmp(midcode[midnum].op, equalop) == 0)
        {
            equal_mips();
        }
        else if(strcmp(midcode[midnum].op, notequalop) == 0)
        {
            notequal_mips();
        }
        else if(strcmp(midcode[midnum].op, lessop) == 0)
        {
            less_mips();
        }
        else if(strcmp(midcode[midnum].op, greatop) == 0)
        {
            great_mips();
        }
        else if(strcmp(midcode[midnum].op, lessorequalop) == 0)
        {
            lessequal_mips();
        }
        else if(strcmp(midcode[midnum].op, greatorequalop) == 0)
        {
            greatequal_mips();
        }
        else if(strcmp(midcode[midnum].op, ifnotop) == 0)
        {
            jne_mips();
        }
        else if(strcmp(midcode[midnum].op, ifop) == 0)
        {
            jmp_mips();
        }
        else if(strcmp(midcode[midnum].op, constop) == 0)
        {
            localConst_mips();
        }
        else if((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) == 0)
        {
            localVar_mips();
        }
        else if((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) != 0)
        {
            localArray_mips();
        }
        else if(strcmp(midcode[midnum].op, paraop) == 0)
        {
            para_mips();
        }
        else if(strcmp(midcode[midnum].op, callop) == 0)
        {
            int tempmidnum = midnum;
            int nowCallLevel = atoi(midcode[midnum].arg2);
            int legal = tabSearch(midcode[midnum].arg1);
            int vparam[100];
            int index = 0;
            int nowParamNum = 0;
            int backNum = 0;
            for(int i = legal + 1 ; tab.mytab[i].obj == PARAMOBJ ; i++)
            {
                nowParamNum++;
            }
            if(nowParamNum == 0)  //void call
            {
                ;
            }
            else    //ret value call
            {
                for(int i = tempmidnum ; backNum != nowParamNum ; i--)
                {
                    if(strcmp(midcode[i].op, vparaop) == 0 && atoi(midcode[i].arg2) == nowCallLevel)
                    {
                        backNum++;
                        vparam[index++] = i;
                        continue;
                    }
                }
                for(int i = index - 1 ; i >= 0 ; i--)
                {
                    midnum = vparam[i];
                    vpara_mips();
                }
                midnum = tempmidnum;
            }
            call_mips();
        }
        else if(strcmp(midcode[midnum].op, myendop) == 0)
        {
            end_mips();
        }
        else if(strcmp(midcode[midnum].op, retop) == 0)
        {
            ret_mips();
        }
        else if(strcmp(midcode[midnum].op, valuetoarrayop) == 0)
        {
            valuetoarrayAssign_mips();
        }
        else if(strcmp(midcode[midnum].op, arraytovalueop) == 0)
        {
            arraytovalueAssign_mips();
        }
        else if(strcmp(midcode[midnum].op, exitop) == 0)
        {
            exit_mips();
        }
        else if(strcmp(midcode[midnum].op, vendop) == 0)
        {
            vend_mips();
        }
    }
}

void o_genTargetCode()
{
    initasm();
    for( ; midnum < midindex ; midnum++)
    {
        if(strcmp(midcode[midnum].op, scanfop) == 0)
        {
            o_scanf_mips();
        }
        else if(strcmp(midcode[midnum].op, printfop) == 0)
        {
            o_printf_mips();
        }
        else if(strcmp(midcode[midnum].op, labop) == 0)
        {
            label_mips();
        }
        else if(strcmp(midcode[midnum].op, funcop) == 0)
        {
            whichFunc++;
            func_mips();
        }
        else if(strcmp(midcode[midnum].op, addop) == 0)
        {
            o_add_mips();
        }
        else if(strcmp(midcode[midnum].op, subop) == 0)
        {
            o_sub_mips();
        }
        else if(strcmp(midcode[midnum].op, multop) == 0)
        {
            o_mul_mips();
        }
        else if(strcmp(midcode[midnum].op, diviop) == 0)
        {
            o_div_mips();
        }
        else if(strcmp(midcode[midnum].op, assignop) == 0)
        {
            o_assign_mips();
        }
        else if(strcmp(midcode[midnum].op, equalop) == 0)
        {
            o_equal_mips();
        }
        else if(strcmp(midcode[midnum].op, notequalop) == 0)
        {
            o_notequal_mips();
        }
        else if(strcmp(midcode[midnum].op, lessop) == 0)
        {
            o_less_mips();
        }
        else if(strcmp(midcode[midnum].op, greatop) == 0)
        {
            o_great_mips();
        }
        else if(strcmp(midcode[midnum].op, lessorequalop) == 0)
        {
            o_lessequal_mips();
        }
        else if(strcmp(midcode[midnum].op, greatorequalop) == 0)
        {
            o_greatequal_mips();
        }
        else if(strcmp(midcode[midnum].op, ifnotop) == 0)
        {
            jne_mips();
        }
        else if(strcmp(midcode[midnum].op, ifop) == 0)
        {
            jmp_mips();
        }
        else if(strcmp(midcode[midnum].op, constop) == 0)
        {
            int i = 0;
            o_localConst_mips();
            i++;
            midnum++;
            while(strcmp(midcode[midnum].op, constop) == 0)
            {
                o_localConst_mips();
                i++;
                midnum++;
            }
            midnum--;
            i = i * 4;
            fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t%d\n", i);
        }
        else if((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) == 0)
        {
            int i = 0;
            o_localVar_mips();
            i++;
            midnum++;
            while((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) == 0)
            {
                o_localVar_mips();
                i++;
                midnum++;
            }
            midnum--;
            i = i * 4;
            fprintf(fpmips, "\t\tsubi\t$sp\t$sp\t%d\n", i);
        }
        else if((strcmp(midcode[midnum].op, intop) == 0 || strcmp(midcode[midnum].op, charop) == 0) && strcmp(midcode[midnum].arg2, blank) != 0)
        {
            localArray_mips();
        }
        else if(strcmp(midcode[midnum].op, paraop) == 0)
        {
            para_mips();
        }
        else if(strcmp(midcode[midnum].op, callop) == 0)
        {
            int tempmidnum = midnum;
            int nowCallLevel = atoi(midcode[midnum].arg2);
            int legal = tabSearch(midcode[midnum].arg1);
            int vparam[100];
            int index = 0;
            int nowParamNum = 0;
            int backNum = 0;
            for(int i = legal + 1 ; tab.mytab[i].obj == PARAMOBJ ; i++)
            {
                nowParamNum++;
            }
            if(nowParamNum == 0)  //void call
            {
                ;
            }
            else    //ret value call
            {
                for(int i = tempmidnum ; backNum != nowParamNum ; i--)
                {
                    if(strcmp(midcode[i].op, vparaop) == 0 && atoi(midcode[i].arg2) == nowCallLevel)
                    {
                        backNum++;
                        vparam[index++] = i;
                        continue;
                    }
                }
                for(int i = index - 1 ; i >= 0 ; i--)
                {
                    midnum = vparam[i];
                    o_vpara_mips();
                }
                midnum = tempmidnum;
            }
            o_call_mips();
        }
        else if(strcmp(midcode[midnum].op, myendop) == 0)
        {
            end_mips();
        }
        else if(strcmp(midcode[midnum].op, retop) == 0)
        {
            o_ret_mips();
        }
        else if(strcmp(midcode[midnum].op, valuetoarrayop) == 0)
        {
            valuetoarrayAssign_mips();
        }
        else if(strcmp(midcode[midnum].op, arraytovalueop) == 0)
        {
            arraytovalueAssign_mips();
        }
        else if(strcmp(midcode[midnum].op, exitop) == 0)
        {
            exit_mips();
        }
        else if(strcmp(midcode[midnum].op, vendop) == 0)
        {
            vend_mips();
        }
    }
}
