#ifndef MIDCODE_H_INCLUDED
#define MIDCODE_H_INCLUDED
#include "table.h"
extern char constop[6];
extern char intop[6];
extern char charop[6];
extern char voidop[6];
extern char addop[6];
extern char subop[6];
extern char multop[6];
extern char diviop[6];
extern char assignop[6];
extern char lessop[6];
extern char greatop[6];
extern char lessorequalop[6];
extern char greatorequalop[6];
extern char equalop[6];
extern char notequalop[6];
extern char valuetoarrayop[6];
extern char arraytovalueop[6];
extern char callop[6];
extern char ifnotop[6];
extern char ifop[6];
extern char labop[6];
extern char retop[6];
extern char scanfop[6];
extern char printfop[6];
extern char funcop[6];
extern char paraop[6];
extern char blank[6];
extern char myzero[6];
extern char vparaop[6];
extern char myendop[6];
extern char exitop[6];
extern char vendop[6];
extern FOURUNIT midcode[MIDCODEMAX];
extern FILE *fpmid;

extern int midindex;

char* genLabel();
char* genTempVar();
void genMidCode(char op[], char arg1[], char arg2[], char res[]);
void genMidCode(char op[], char arg1[], int arg2, char res[]);
char* int2str(int a);
void printMidCode();
void formatMidcode();
#endif // MIDCODE_H_INCLUDED
