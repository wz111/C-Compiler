#include <stdio.h>

#include "main.h"
#include "wordAnalyze.h"
#include "grammarAnalyze.h"
#include "table.h"
#include "error.h"

TAB tab;
ATAB atab;
BTAB btab;
STAB stab;
DISPLAY dp;

void initTable()
{
    tab.t = -1;
    atab.a = 0;
    btab.b = 0;
    stab.sx = -1;
    dp.level = -1;
}

int tabInsert(char name[], int obj, int type, int refer, int level, bool normal, int addr)
/** insert constant to tab
 * \param :refer(the upper of the array)
 * \return
 *  0: successful
 *  -1: repeated definition
 *  -2: full sign table
 *  -3: full array table
 *  -4: type of the array illegal
 *  -5: full function table
 */
{
    if(tab.t == TABMAX-1)
    {
        error(ERROR_OVERFLOW_MEMORY);  //tab is full
        return -2;
    }
    else
    {
        struct tabnode tnode;
        tnode.typ = type;
        strcpy(tnode.name, name);
        for(int i = tab.t ; tab.mytab[i].adr != 0 ; i--)
        {
            if((strcmp(tab.mytab[i].name, tnode.name)) == 0)
            {
                error(ERROR_DUPLICATE_IDENTIFIER, tnode.name);
                return -1;
            }
        }
        tnode.adr = addr;
        tnode.obj = obj;
        tnode.refer = refer;
        tnode.normal = normal;
        tnode.lev = level;
        if(tab.t == -1)
        {
            tnode.link = 0;
        }
        else
        {
            int temp = 0;
            for(int i = tab.t ; i>=0 ; i--)
            {
                if(tab.mytab[i].link != 0)
                {
                    temp = tab.mytab[i].link;
                    break;
                }
            }
            tnode.link = temp;
        }
        tab.mytab[++tab.t] = tnode;
        return 0;
    }
}

int tabSearch(char name[])
{
    for(int i = tab.t ; i>=0 ; i--)
    {
        if(strcmp(tab.mytab[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;  //not find
}
