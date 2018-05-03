#ifndef OPTIMIZATION_H_INCLUDED
#define OPTIMIZATION_H_INCLUDED
#include <iostream>
#include <vector>
#include <set>

#define REGNUM 8
using namespace std;

typedef struct dagnode{
    char name[100];
    vector<int> Lparent;
    vector<int> Rparent;
    int Lson;
    int Rson;
    int flag;
}DAGNODE;

typedef struct dagtabnode{
    char name[100];
    int index;
    int flag;
}DAGTABNODE;

typedef struct conflictnode{
    string name;
    vector<int> active;
    set<string> link;
}CONFLICTNODE;

typedef struct dyenode{
    string name;
    int reg;
}DYENODE;

extern vector<DYENODE> gRegQueue[100];
extern vector<int> callBlock;

void DAG(int bhead, int bend);
void DAGopt();
void Globalopt();

#endif // OPTIMIZATION_H_INCLUDED
