#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED
#include <cstring>
#define NUM_OF_TABLE 10000

#define VAROBJ 1
#define ARRAYOBJ 3
#define FUNCOBJ 4
#define PARAMOBJ 2
#define CONSTOBJ 0

#define INTTYPE 0
#define CHARTYPE 1
#define VOIDTYPE 2

#define NAMEMAX 100
#define DISPLAYMAX 100
#define STRTABMAX 100
#define TCODEMAX 10000
#define MIPSMAX 100
#define FOURUNITMAX 100

#define TABMAX 10000
#define MIDCODEMAX 10000

//sign table
struct tabnode{
	char name[NAMEMAX];	//symbol name
	int link;           //同一个分程序中上一个标识符在tab表中的位置
	int obj;		    //obj: constant--0, variable--1, parameter--2, array--3, function--4.
	int typ;			//type: int--0, char--1, void--2(function)
    int refer;			/**
                         *
                         * identifier == array --> refer point to the location of the atab
                         * identifier == function --> refer point to the location of the btab
                         * else refer = 0
                         *
                         */
    int lev;            /*分程序的静态层次，主程序(不是主函数！)层次初始为1*/
    bool normal;        /*如果形参是标识符时false，否则是true*/
    int adr;            /**
                         *
                         * variable --> address of the storage unit allocated in the run stack
                         * function --> the entry address of the corresponding object code
                         * integer, character constant --> their value
                         *
                         */
};

//Sub-program table
struct btabnode{
    int last;           //The last identifier currently in the program
    int lastpar;        //The last parameter currently in the program
    int psize;          //the number of the parameter and sub-program storage units in the running stack
    int vsize;          //the number of the localVar storage units in the running stack
};

//array information table
struct atabnode{
    int low;
    int high;
    int elsize;     //element size
    int arsize;     //array size
};

//display table
typedef struct display{
    int subindex[DISPLAYMAX];
    int level;
}DISPLAY;

typedef struct table{
    struct tabnode mytab[TABMAX];
    int t;      //Subscript
}TAB;

typedef struct btable{
    struct btabnode mybtab[TABMAX];
    int b;      //Subscript
}BTAB;

typedef struct atable{
    struct atabnode myatab[TABMAX];
    int a;      //Subscript
}ATAB;

//constant string table
typedef struct stable{
    //todo: finish the string table
    //string strtab[STRTABMAX];
    int sx;     //Subscript
}STAB;

//
typedef struct mipsorder{
    char order[MIPSMAX];
    char destReg[MIPSMAX];
    char srcReg1[MIPSMAX];
    char srcReg2[MIPSMAX];
}MIPSORDER;

//target code table
typedef struct tcode{
    MIPSORDER code[TCODEMAX];
}TCODETAB;

typedef struct fourunit{
    char op[FOURUNITMAX];
    char arg1[FOURUNITMAX];
    char arg2[FOURUNITMAX];
    char res[FOURUNITMAX];
}FOURUNIT;

extern TAB tab;
extern ATAB atab;
extern BTAB btab;
extern STAB stab;
extern DISPLAY dp;
extern FOURUNIT midcode[MIDCODEMAX];
int tabInsert(char name[], int obj, int type, int refer, int level, bool normal, int addr);
int tabSearch(char name[]);
#endif // TABLE_H_INCLUDED
