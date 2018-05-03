#include <stdio.h>
#include <iostream>
#include <vector>
#include <set>
#include <string>

#include "midcode.h"
#include "optimization.h"
#include "table.h"

using namespace std;

vector<DYENODE> gRegQueue[100];
vector<int> callBlock;
int gRegNum = 0;
int callFunc = 0;
bool isBlockEnd(FOURUNIT f)
{
    if(strcmp(f.op, ifop) == 0 || strcmp(f.op, ifnotop) == 0
       /*|| strcmp(f.op, callop)  == 0*/ || strcmp(f.op, retop) == 0
       || strcmp(f.op, myendop) == 0 || strcmp(f.op, exitop) == 0
       || strcmp(f.op, vendop) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool isBlockHead(FOURUNIT f)
{
    if(strcmp(f.op, labop) == 0 || strcmp(f.op, funcop) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int dagTabSearch(DAGTABNODE dagtab[], char name[], int dagTabIndex)
{
    for(int i = 1 ; i < dagTabIndex ; i++)
    {
        if(strcmp(dagtab[i].name, name) == 0)
        {
            return dagtab[i].index;
        }
        else
        {
            continue;
        }
    }
    return -1;
}

int dagSearch(DAGNODE dag[], char name[], int dagIndex, int arg1num, int arg2num)
{
    if(arg1num == -1)
    {
        for(int i = 1 ; i < dagIndex ; i++)
        {
            if(dag[i].Rson == arg2num)
            {
                return i;
            }
            else
            {
                continue;
            }
        }
    }
    else if(arg2num == -1)
    {
        for(int i = 1 ; i < dagIndex ; i++)
        {
            if(dag[i].Lson == arg1num)
            {
                return i;
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        for(int i = 1 ; i < dagIndex ; i++)
        {
            if(dag[i].Lson == arg1num && dag[i].Rson == arg2num)
            {
                return i;
            }
            else
            {
                continue;
            }
        }
    }
    return -1;
}

void DAG(int bhead, int bend)
{
    for(int i = bhead ; i <= bend ; i++)
    {
        int arg1num = -1;
        int arg2num = -1;
        int opnum = -1;
        int dagIndex = 1;
        int dagTabIndex = 1;
        int midnode = 0;
        int head = 0;
        int tail = 0;
        if(strcmp(midcode[i].op, addop) == 0 || strcmp(midcode[i].op, subop) == 0
           || strcmp(midcode[i].op, multop) == 0 || strcmp(midcode[i].op, diviop) == 0)
        {
            head = i;
            DAGNODE dag[50];
            DAGTABNODE dagtab[50];
            while(strcmp(midcode[i].op, addop) == 0 || strcmp(midcode[i].op, subop) == 0
                    || strcmp(midcode[i].op, multop) == 0 || strcmp(midcode[i].op, diviop) == 0)
            {
                if(strcmp(midcode[i].arg1, blank) == 0)
                {
                    arg1num = -1;
                }
                else
                {
                    int arg1Search = dagTabSearch(dagtab, midcode[i].arg1, dagTabIndex);
                    if(arg1Search == -1)
                    {
                        strcpy(dag[dagIndex].name, midcode[i].arg1);
                        dag[dagIndex].flag = 0;
                        dag[dagIndex].Lson = 0;
                        dag[dagIndex].Rson = 0;
                        dagIndex++;
                        strcpy(dagtab[dagTabIndex].name, midcode[i].arg1);
                        dagtab[dagTabIndex].index = dagTabIndex;
                        dagtab[dagTabIndex].flag = 0;
                        arg1num = dagTabIndex;
                        dagTabIndex++;
                    }
                    else
                    {
                        arg1num = arg1Search;
                    }
                }
                if(strcmp(midcode[i].arg2, blank) == 0)
                {
                    arg2num = -1;
                }
                else
                {
                    int arg2Search = dagTabSearch(dagtab, midcode[i].arg2, dagTabIndex);
                    if(arg2Search == -1)
                    {
                        strcpy(dag[dagIndex].name, midcode[i].arg2);
                        dag[dagIndex].flag = 0;
                        dag[dagIndex].Lson = 0;
                        dag[dagIndex].Rson = 0;
                        dagIndex++;
                        strcpy(dagtab[dagTabIndex].name, midcode[i].arg2);
                        dagtab[dagTabIndex].index = dagTabIndex;
                        dagtab[dagTabIndex].flag = 0;
                        arg2num = dagTabIndex;
                        dagTabIndex++;
                    }
                    else
                    {
                        arg2num = arg2Search;
                    }
                }

                int opSearch = dagSearch(dag, midcode[i].op, dagIndex, arg1num, arg2num);
                if(opSearch == -1)
                {
                    strcpy(dag[dagIndex].name, midcode[i].op);
                    dag[dagIndex].Lson = arg1num;
                    dag[dagIndex].Rson = arg2num;
                    dag[dagIndex].flag = 0;
                    dag[arg1num].Rparent.push_back(dagIndex);
                    dag[arg2num].Lparent.push_back(dagIndex);
                    opnum = dagIndex;
                    dagIndex++;
                    midnode++;
                }
                else
                {
                    opnum = opSearch;
                }
                int resSearch = dagTabSearch(dagtab, midcode[i].res, dagTabIndex);
                if(resSearch == -1)
                {
                    dagtab[dagTabIndex].index = opnum;
                    strcpy(dagtab[dagTabIndex].name, midcode[i].res);
                    dagtab[dagTabIndex].flag = 0;
                    dagTabIndex++;
                }
                else
                {
                    dagtab[resSearch].index = opnum;
                }
                i++;
            }
            tail = i;
            vector<int> Queue;
            //printf("%d %d\n", head+1, tail+1);
            for(int m = 1 ; (signed)Queue.size() != midnode ; m = (m % dagIndex) + 1)
            {
                bool willin = true;
                if(dag[m].Lson >= 1 && dag[m].Lson <= dagIndex
                   && dag[m].Rson >= 1 && dag[m].Rson <= dagIndex)
                {
                    if(dag[m].flag == 0)
                    {
                        for(int j = 0 ; j < (signed)dag[m].Lparent.size() ; j++)
                        {
                            if(dag[dag[m].Lparent[j]].flag == 0)
                            {
                                willin = false;
                                break;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        for(int j = 0 ; j < (signed)dag[m].Rparent.size() ; j++)
                        {
                            if(dag[dag[m].Rparent[j]].flag == 0)
                            {
                                willin = false;
                                break;
                            }
                            else
                            {
                                continue;
                            }
                        }
                        if(willin)
                        {
                            Queue.push_back(m);
                            dag[m].flag = 1;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }
            }
            for(int m = head ; m < tail ; m++)
            {
                if( m <= head + midnode )
                {
                    for(int n = (signed)Queue.size()-1 ; n >= 0 ; n--)
                    {
                        for(int j = 1 ; j < dagTabIndex ; j++)
                        {
                            if(dagtab[j].index == Queue[n])
                            {
                                //printf("m = %d\n", m+1);
                                strcpy(midcode[m].op, dag[dagtab[j].index].name);
                                strcpy(midcode[m].res, dagtab[j].name);
                                //printf("op: %s, res: %s\n", midcode[m].op, midcode[m].res);
                                if(strcmp(dag[dag[dagtab[j].index].Lson].name, addop) == 0 ||
                                   strcmp(dag[dag[dagtab[j].index].Lson].name, subop) == 0 ||
                                   strcmp(dag[dag[dagtab[j].index].Lson].name, multop) == 0 ||
                                   strcmp(dag[dag[dagtab[j].index].Lson].name, diviop) == 0)
                                {
                                    for(int k = 1 ; k < dagTabIndex ; k++)
                                    {
                                        if(dag[dagtab[j].index].Lson == dagtab[k].index && dagtab[k].flag == 1)
                                        {
                                            strcpy(midcode[m].arg1, dagtab[k].name);
                                            //printf("arg1: %s\n", midcode[m].arg1);
                                        }
                                    }
                                }
                                else
                                {
                                    strcpy(midcode[m].arg1, dag[dag[dagtab[j].index].Lson].name);
                                    //printf("arg1: %s\n", midcode[m].arg1);
                                }
                                if(strcmp(dag[dag[dagtab[j].index].Rson].name, addop) == 0 ||
                                   strcmp(dag[dag[dagtab[j].index].Rson].name, subop) == 0 ||
                                   strcmp(dag[dag[dagtab[j].index].Rson].name, multop) == 0 ||
                                   strcmp(dag[dag[dagtab[j].index].Rson].name, diviop) == 0)
                                {
                                    for(int k = 1 ; k < dagTabIndex ; k++)
                                    {
                                        if(dag[dagtab[j].index].Rson == dagtab[k].index && dagtab[k].flag == 1)
                                        {
                                            strcpy(midcode[m].arg2, dagtab[k].name);
                                            //printf("arg2: %s\n", midcode[m].arg2);
                                        }
                                    }
                                }
                                else
                                {
                                    strcpy(midcode[m].arg2, dag[dag[dagtab[j].index].Rson].name);
                                    //printf("arg2: %s\n", midcode[m].arg2);
                                }
                                dagtab[j].flag = 1;
                                m++;
                                break;
                            }
                        }
                    }
                    if(m != tail)
                    {
                        strcpy(midcode[m].op, blank);
                        strcpy(midcode[m].arg1, blank);
                        strcpy(midcode[m].arg2, blank);
                        strcpy(midcode[m].res, blank);
                    }
                    else
                    {
                        ;
                    }
                }
                else
                {
                    if(m != tail)
                    {
                        strcpy(midcode[m].op, blank);
                        strcpy(midcode[m].arg1, blank);
                        strcpy(midcode[m].arg2, blank);
                        strcpy(midcode[m].res, blank);
                    }
                    else
                    {
                        ;
                    }
                }
            }

            /*
            printf("\n*****DAG*****\n");
            for(int j = 1 ; j < dagIndex ; j++)
            {
                printf("name=%s index=%d Lson=%d Rson=%d\n", dag[j].name, j, dag[j].Lson, dag[j].Rson);
            }
            printf("\n*****DAG TAB*****\n");
            for(int j = 1 ; j < dagTabIndex ; j++)
            {
                printf("name=%s index=%d\n", dagtab[j].name, dagtab[j].index);
            }
            printf("midnode: %d\n", midnode);
            printf("\n*****Queue*****\n");
            for(int j = 0 ; j < (signed)Queue.size() ; j++)
            {
                printf("midnode[%d]: %d\n", j, Queue[j]);
            }
            */
        }
        else
        {
            continue;
        }
    }
}

void DAGopt()
{
    int bhead = 0;
    int bend = 0;
    for(int i = 0 ; i < midindex ; i++)
    {
        if(isBlockHead(midcode[i]) || i == bend + 1)
        {
            bhead = i;
            i++;
            for( ; i < midindex ; i++)
            {
                if(strcmp(midcode[i].op, blank) == 0)
                {
                    continue;
                }
                if(!isBlockEnd(midcode[i]) && !isBlockHead(midcode[i]))
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            bend = i;
            if(isBlockHead(midcode[i]))
            {
                bend--;
                i--;
            }
            //printf("b: %d %d\n", bhead+1, bend+1);
            DAG(bhead, bend);
        }
        else
        {
            continue;
        }
    }
}

void Globalopt()
{
    int bhead = 0;
    int bend = 0;
    int blockNum = 0;
    set<string> use[100];
    set<string> def[100];
    set<string> in[100];
    set<string> out_def[100];
    set<string> out[100];
    set<string> prein[100];
    set<string> preout[100];
    vector<int> after[100];
    vector<int> block[100];
    for(int i = 0 ; i < midindex ; i++)
    {
        if(isBlockHead(midcode[i]) || i == bend + 1)
        {
            bhead = i;
            i++;
            for( ; i < midindex ; i++)
            {
                if(strcmp(midcode[i].op, blank) == 0)
                {
                    continue;
                }
                if(!isBlockEnd(midcode[i]) && !isBlockHead(midcode[i]))
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            bend = i;
            if(isBlockHead(midcode[i]))
            {
                bend--;
                i--;
            }
            block[blockNum].push_back(bhead);
            block[blockNum].push_back(bend);
            //printf("%d, %d\n", bhead, bend);
            for(int j = bhead ; j <= bend ; j++)
            {
                if(strcmp(midcode[j].op, addop) == 0 || strcmp(midcode[j].op, subop) == 0
                    || strcmp(midcode[j].op, multop) == 0 || strcmp(midcode[j].op, diviop) == 0
                    || strcmp(midcode[j].op, assignop) == 0)
                {
                    if(midcode[j].arg1[0] == '_' || (midcode[j].arg1[0] >= 'a' && midcode[j].arg1[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].arg1);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].arg1) == use[blockNum].end()
                            && def[blockNum].find(midcode[j].arg1) == def[blockNum].end())
                            {
                                use[blockNum].insert(midcode[j].arg1);
                            }
                            else;
                        }
                    }
                    if(midcode[j].arg2[0] == '_' || (midcode[j].arg2[0] >= 'a' && midcode[j].arg2[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].arg2);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].arg2) == use[blockNum].end()
                               && def[blockNum].find(midcode[j].arg2) == def[blockNum].end())
                            {
                                use[blockNum].insert(midcode[j].arg2);
                            }
                            else;
                        }
                    }
                    if(midcode[j].res[0] == '_' || (midcode[j].res[0] >= 'a' && midcode[j].res[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].res);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].res) == use[blockNum].end()
                               && def[blockNum].find(midcode[j].res) == def[blockNum].end())
                            {
                                def[blockNum].insert(midcode[j].res);
                            }
                            else;
                        }
                    }
                }
                else if(strcmp(midcode[j].op, arraytovalueop) == 0)
                {
                    if(midcode[j].res[0] == '_' || (midcode[j].res[0] >= 'a' && midcode[j].res[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].res);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].res) == use[blockNum].end()
                               && def[blockNum].find(midcode[j].res) == def[blockNum].end())
                            {
                                def[blockNum].insert(midcode[j].res);
                            }
                            else;
                        }
                    }
                }
                else if(strcmp(midcode[j].op, valuetoarrayop) == 0)
                {
                    if(midcode[j].arg1[0] == '_' || (midcode[j].arg1[0] >= 'a' && midcode[j].arg1[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].arg1);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].arg1) == use[blockNum].end()
                            && def[blockNum].find(midcode[j].arg1) == def[blockNum].end())
                            {
                                use[blockNum].insert(midcode[j].arg1);
                            }
                            else;
                        }
                    }
                }
                else if(strcmp(midcode[j].op, equalop) == 0 || strcmp(midcode[j].op, notequalop) == 0
                        || strcmp(midcode[j].op, greatop) == 0 || strcmp(midcode[j].op, lessop) == 0
                        || strcmp(midcode[j].op, lessorequalop) == 0 || strcmp(midcode[j].op, greatorequalop) == 0)
                {
                    if(midcode[j].arg1[0] == '_' || (midcode[j].arg1[0] >= 'a' && midcode[j].arg1[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].arg1);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].arg1) == use[blockNum].end()
                            && def[blockNum].find(midcode[j].arg1) == def[blockNum].end())
                            {
                                use[blockNum].insert(midcode[j].arg1);
                            }
                            else;
                        }
                    }
                    if(midcode[j].arg2[0] == '_' || (midcode[j].arg2[0] >= 'a' && midcode[j].arg2[0] <= 'z'))
                    {
                        int tempres = tabSearch(midcode[j].arg2);
                        if(tempres != -1 && tab.mytab[tempres].obj == VAROBJ)
                        {
                            if(use[blockNum].find(midcode[j].arg2) == use[blockNum].end()
                               && def[blockNum].find(midcode[j].arg2) == def[blockNum].end())
                            {
                                use[blockNum].insert(midcode[j].arg2);
                            }
                            else;
                        }
                    }
                }
                else if(strcmp(midcode[j].op, callop) == 0)
                {
                    callFunc = 1;
                }
                else
                {
                    continue;
                }
            }
            blockNum++;
            if(strcmp(midcode[bend].op, myendop) == 0 || strcmp(midcode[bend].op, exitop) == 0 || strcmp(midcode[bend].op, vendop) == 0)
            {
                //ensure the relationship
                for(int j = 0 ; j < blockNum ; j++)
                {
                    if(strcmp(midcode[block[j][1]].op, ifop) == 0)
                    {
                        char tempLabel[100];
                        strcpy(tempLabel, midcode[block[j][1]].res);
                        int nextline = block[j][1] + 1;
                        for( ; !(strcmp(midcode[nextline].res, tempLabel) == 0) ; nextline = (nextline + 1) % midindex);
                        int nextblock = 0;
                        for( ; nextblock < blockNum ; nextblock++)
                        {
                            if(block[nextblock][0] == nextline)
                            {
                                break;
                            }
                        }
                        after[j].push_back(nextblock);
                    }
                    else if(strcmp(midcode[block[j][1]].op, ifnotop) == 0)
                    {
                        after[j].push_back(j + 1);
                        char tempLabel[100];
                        strcpy(tempLabel, midcode[block[j][1]].res);
                        int nextline = block[j][1] + 1;
                        for( ; !(strcmp(midcode[nextline].res, tempLabel) == 0) ; nextline = (nextline + 1) % midindex);
                        int nextblock = 0;
                        for( ; nextblock < blockNum ; nextblock++)
                        {
                            if(block[nextblock][0] == nextline)
                            {
                                break;
                            }
                        }
                        after[j].push_back(nextblock);
                    }
                    else
                    {
                        after[j].push_back(j + 1);
                    }
                }

                //suan <set>in and <set>out
                while(true)
                {
                    for(int m = blockNum ; m >= 0 ; m--)
                    {
                        for(int n = 0 ; n < (signed)(after[m].size()) ; n++)
                        {
                            for(set<string>::iterator it = in[after[m][n]].begin() ; it != in[after[m][n]].end() ; it++)
                            {
                                out[m].insert(*it);
                                out_def[m].insert(*it);
                            }
                        }
                        //out set finished
                        for(set<string>::iterator it = def[m].begin() ; it != def[m].end() ; it++)
                        {
                            out_def[m].erase(*it);
                        }
                        for(set<string>::iterator it = out_def[m].begin() ; it != out_def[m].end() ; it++)
                        {
                            in[m].insert(*it);
                        }
                        for(set<string>::iterator it = use[m].begin() ; it != use[m].end() ; it++)
                        {
                            in[m].insert(*it);
                        }
                        //in set finished
                    }
                    int inSameNum = 0;
                    int outSameNum = 0;
                    for(int m = 0 ; m <= blockNum ; m++)
                    {
                        if(in[m] == prein[m])
                        {
                            inSameNum++;
                        }
                        else
                        {
                            for(set<string>::iterator it = in[m].begin() ; it != in[m].end() ; it++)
                            {
                                prein[m].insert(*it);
                            }
                        }
                    }
                    for(int m = 0 ; m <= blockNum ; m++)
                    {
                        if(out[m] == preout[m])
                        {
                            outSameNum++;
                        }
                        else
                        {
                            for(set<string>::iterator it = out[m].begin() ; it != out[m].end() ; it++)
                            {
                                preout[m].insert(*it);
                            }
                        }
                    }
                    if(inSameNum-1 == blockNum && outSameNum-1 == blockNum)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                //get conflict graph
                CONFLICTNODE conflict[100];
                int conNum = 0;
                set<string> allVar;
                for(int m = 0 ; m < blockNum ; m++)
                {
                    for(set<string>::iterator it1 = in[m].begin() ; it1 != in[m].end() ; it1++)
                    {
                        if(allVar.find(*it1) == allVar.end())
                        {
                            conflict[conNum].name = *it1;
                            conflict[conNum].active.push_back(m);
                            allVar.insert(*it1);
                            for(set<string>::iterator it2 = in[m].begin() ; it2 != in[m].end() ; it2++)
                            {
                                if(it1 == it2)
                                {
                                    continue;
                                }
                                else
                                {
                                    conflict[conNum].link.insert(*it2);
                                }
                            }
                            conNum++;
                        }
                        else
                        {
                            int n = 0;
                            for(n = 0 ; n < conNum ; n++)
                            {
                                if(conflict[n].name == *it1)
                                {
                                    conflict[n].active.push_back(m);
                                    break;
                                }
                            }
                            for(set<string>::iterator it2 = in[m].begin() ; it2 != in[m].end() ; it2++)
                            {
                                if(it1 == it2)
                                {
                                    continue;
                                }
                                else
                                {
                                    conflict[n].link.insert(*it2);
                                }
                            }
                        }
                    }
                }
                /*debug
                printf("blockNum: %d\n", blockNum);
                printf("****after****\n");
                for(int m = 0 ; m < blockNum ; m++)
                {
                    printf("block%d(%d~%d): ", m, block[m][0], block[m][1]);
                    for(int n = 0 ; n<(signed)(after[m].size()) ; n++)
                    {
                        printf("%d ", after[m][n]);
                    }
                    printf("\n");
                    printf("\tuse:\n");
                    for(set<string>::iterator it = use[m].begin() ; it != use[m].end() ; it++)
                    {
                        cout << "\t\t" <<*it << endl;
                    }
                    printf("\tdef:\n");
                    for(set<string>::iterator it = def[m].begin() ; it != def[m].end() ; it++)
                    {
                        cout << "\t\t" <<*it << endl;
                    }
                    printf("\tin:\n");
                    for(set<string>::iterator it = in[m].begin() ; it != in[m].end() ; it++)
                    {
                        cout << "\t\t" <<*it << endl;
                    }
                    printf("\tout:\n");
                    for(set<string>::iterator it = out[m].begin() ; it != out[m].end() ; it++)
                    {
                        cout << "\t\t" <<*it << endl;
                    }
                }
                for(int m = 0 ; m < conNum ; m++)
                {
                    cout << conflict[m].name << ": ";
                    for(set<string>::iterator it = conflict[m].link.begin() ; it != conflict[m].link.end() ; it++)
                    {
                        cout << *it << ", " ;
                    }
                    printf("\n");
                }
                debug*/
                vector<DYENODE> dyeQueue;
                int dyeIndex = 0;
                int varNum = conNum;
                for(int m = 0 ; m < conNum ; m++)
                {
                    for(set<string>::iterator it = conflict[m].link.begin() ; it != conflict[m].link.end() ; it++)
                    {
                        for(int n = 0 ; n < conNum ; n++)
                        {
                            if(*it == conflict[n].name)
                            {
                                conflict[n].link.erase(conflict[m].name);
                            }
                        }
                    }
                    if((signed)conflict[m].link.size() < REGNUM)
                    {
                        DYENODE dy;
                        dy.name = conflict[m].name;
                        dy.reg = 0;
                        dyeQueue.push_back(dy);
                    }
                    else
                    {
                        ;
                    }
                    conflict[m].name = "";
                    conflict[m].link.clear();
                    conflict[m].active.clear();
                    varNum--;
                    if(varNum == 0)
                    {
                        break;
                    }
                }
                //printf("****dyeGraph****\n");
                for(int m = (signed)dyeQueue.size()-1 ; m >= 0 ; m--)
                {
                    dyeQueue[m].reg = dyeIndex;
                    dyeIndex = (dyeIndex + 1) % REGNUM;
                    //cout << "name: " << dyeQueue[m].name << ", reg: $s" << dyeQueue[m].reg << endl;
                }
                gRegQueue[gRegNum++].assign(dyeQueue.begin(), dyeQueue.end());
                callBlock.push_back(callFunc);
                callFunc = 0;
                for(int m = 0 ; m < blockNum ; m++)
                {
                    use[m].clear();
                    def[m].clear();
                    after[m].clear();
                    block[m].clear();
                    in[m].clear();
                    out[m].clear();
                    prein[m].clear();
                    preout[m].clear();
                }
                blockNum = 0;
            }
        }
        else
        {
            continue;
        }
    }
}
