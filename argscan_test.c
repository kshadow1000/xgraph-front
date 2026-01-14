#include "argscan.c"
#include "stdio.h"
int ascommon(char *z){
	printf("common:%s\n",z);
	return 0;
}
int ascall(char *z){
	printf("call:%s\n",z);
	return 0;
}
double dval;
int ival;
char *p;
struct argtype ats[]={
	{"a","A",{.callback=ascall,},AT_CALL,0},
	{"b","B",{.dval=&dval,},AT_DOUBLE,0},
	{"c","C",{.ival=&ival,},AT_INT,0},
	{"d","D",{.ival=&ival,},AT_BOOL,0},
	{"e","E",{.ival=&ival,},AT_BOOLX,0},
	{"f","F",{.ival=&ival,},AT_BOOLA,0},
	{"g","G",{.sval=&p,},AT_STR,0},
};
int main(int argc,char **argv){
	if(argscan(argc-1,argv+1,0,ats,ascommon)<0)
		return -1;
	printf("d:%lf,i:%d,p:%s\n",dval,ival,p);
}
