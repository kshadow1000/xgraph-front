#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "xgraph/header/expr.h"
#include <time.h>
#include <err.h>
#include <errno.h>
#include "readall.c"
//#define fprintf(...) 1
//#define puts(...) 1
void add_common_symbols(struct expr_symset *es);
int main(int argc,char **argv){
	char *buf,*p,*p1;
	char *e;
	int flag=0;
	struct expr ep[1];
	long count=1,x=0;
	double result;
	//init_expr(ep,"(t^2+1)*sum(n,1,10,1,sin(n*t))","t",NULL);
	//expr_free(ep);
	if(argc<2)
	errx(EXIT_FAILURE,"no expression input");
	struct expr_symset *es=new_expr_symset();
	//char buf[9999]={"sum(n,1,100,1,n)"};
	//double n=6.6;
	srand48(getpid()^time(NULL));
	//printf("%lu\n",(unsigned long)&n);
	//expr_symset_add(esp,"sb",&n,EXPR_PARAMETER);
	//printf("Input your expression:");
//	fgets(buf,9999,stdin);
	for(int i=1;i<argc-1;++i){
		if(!strcmp(argv[i],"-f")&&i<argc-3){
			p1=argv[++i];
			if((p=strchr(argv[++i],':'))){
				buf=argv[i];
				*(p++)=0;
			}else {
				buf="t";
				p=argv[i];
			}
			expr_symset_add(es,p1,EXPR_HOTFUNCTION,p,buf);
		}else if(!strcmp(argv[i],"-x"))x=1;
		else if(!strcmp(argv[i],"-n")&&i<argc-2){
				count=atol(argv[++i]);
		}else if(!strcmp(argv[i],"--no"))
			flag|=EXPR_IF_NOOPTIMIZE;
		else if(!strcmp(argv[i],"--nb"))
			flag|=EXPR_IF_NOBUILTIN;
		else if(!strcmp(argv[i],"--nk"))
			flag|=EXPR_IF_NOKEYWORD;
		else if(!strcmp(argv[i],"--i"))
			flag|=EXPR_IF_INJECTION;
		else if(!strcmp(argv[i],"--p"))
			flag|=EXPR_IF_PROTECT;
		else if(!strcmp(argv[i],"--is"))
			flag|=EXPR_IF_INJECTION_S;
	}
	add_common_symbols(es);
	if(!strcmp(e=argv[argc-1],"-")){
		ssize_t r=readall(STDIN_FILENO,(void **)&e);
		if(r<0){
			expr_symset_free(es);
			errx(EXIT_FAILURE,"cannot read stdin:%s",strerror(-r));
		}
	}
	if(init_expr5(ep,e,"t",es,flag)<0){
		expr_symset_free(es);
		if(e!=argv[argc-1])free(e);
		errx(EXIT_FAILURE,"expression error:%s (%s)",expr_error(ep->error),ep->errinfo);
	}
redo:
	result=expr_eval(ep,0);
	if(--count)goto redo;
	//asprintf(&buf,"%.64lf",result);
	asprintf(&buf,x?"%.1024la":"%.1024lf",result);
	p=strchr(buf,'.');
	if(p){
		p+=strlen(p);
		while(*(--p)=='0')*p=0;
		if(*p=='.')*p=0;

	}
	puts(buf);
	free(buf);
	expr_free(ep);
	expr_symset_free(es);
	if(e!=argv[argc-1])free(e);
	return 0;
}
