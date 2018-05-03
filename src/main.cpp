#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"
#include "wordAnalyze.h"
#include "grammarAnalyze.h"
#include "midcode.h"
#include "mips.h"
#include "optimization.h"
int main(int argc, char* argv[])
{
    fp = fopen(argv[2], "r");
    if(argc > 3)
    {
        printf("too many parameters\n");
        printf("Usage:\t [filepath][mode](absolute or relative)\n");
        exit(-1);
    }
    if(fp == NULL)
    {
        printf("Unable to open the file");
        return -1;
    }
    else
    {
        getsym();
        printf("\n----------- Build: Debug in compiler (compiler: Personal Compiler)------------\n");
        program();
        if(strcmp(argv[1], "-o1") == 0)
        {
            DAGopt();
            printMidCode();
            formatMidcode();
            genTargetCode();
        }
        else if(strcmp(argv[1], "-o2") == 0)
        {
            DAGopt();
            Globalopt();
            printMidCode();
            formatMidcode();
            o_genTargetCode();
        }
        else if(strcmp(argv[1], "-no") == 0)
        {
            printMidCode();
            formatMidcode();
            genTargetCode();
        }
        printf("\n------------ Process terminated with status 0------------\n");
    }
    fclose(fp);
    fclose(fpmid);
    fclose(fpmips);
    fclose(fpfmid);
    return 0;
}
