#ifndef GRAMMARANALYZE_H_INCLUDED
#define GRAMMARANALYZE_H_INCLUDED

void addOperator();
void assignmentStatement();
void caseStatement(char tempIdName[]);
void caseTable(char tempIdName[]);
void character();
void charString();
void compoundStatement();
void condition();
void conditionalStatement();
void constant();
void constDeclr();
void constDef();
void declrHead();
void defaultStatement();
void expression();
void factor();
void identifier();
void item();
void integer();
int letter(char s);
void loopStatement();
void multOperator();
void mainFunc();
int notZeroNum(char s);
int number(char s);
void parameter();
void paramTable();
void program();
void readStatement();
void relaOperator();
void retFuncCall();
void retFuncDefine();
void retStatement();
void situationStatement();
void statement();
void statementColumn();
void syntaxerror();
void typeIdentifier();
int unsignInteger(char s[]);
void valueParamTable();
void varDeclr();
void varDef();
void voidFuncCall();
void voidFuncDefine();
void writeStatement();

#endif // GRAMMARANALYZE_H_INCLUDED
