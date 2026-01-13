#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "xgraph/header/expr.h"
#include <time.h>
#include <err.h>
#include <assert.h>
#include <sys/random.h>
void add_common_symbols(struct expr_symset *es);
double dtime(void);
struct expr_symset es[1];
volatile sig_atomic_t vssuc;
volatile sig_atomic_t vsi;
void psig(int sig){
	fprintf(stderr,"final %d/%d\n",vssuc,vsi);
}
const char pool[]={"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_@"};
void randstr(char *buf,size_t sz){
	if(!sz)sz=1;
	do
		*(buf++)=pool[rand()%sizeof(pool)];
	while(--sz);
	*buf=0;
}
char buf[4294967296UL/2]={0};
char *p=buf;
void *alloc_hook(size_t sz){
	void *r=p;
	p+=sz;
	assert(p<=buf+sizeof(buf));
	return r;
}
void free_hook(void *r){
}
ssize_t getrandom(void* _Nonnull __buffer, size_t __buffer_size, unsigned int __flags);
int main(int argc,char **argv){
	char buf[32];
	char target[32]={"mustin"};
	double st;
	size_t k,suc=0,l=0,count=argc>2?atol(argv[2]):1000,n=argc>1?atol(argv[1]):1000;
	size_t i;
	struct expr_symbol *esp;
	unsigned int in;
	if(argc>3)strcpy(target,argv[3]);
	init_expr_symset(es);
	//add_common_symbols(es);
	fputs("creating\n",stderr);
	st=dtime();
	getrandom(&in,4,GRND_NONBLOCK);
	srand(in);
	signal(SIGABRT,psig);
	expr_allocator=alloc_hook;
	expr_deallocator=free_hook;
	for(i=1;suc<n;++i){
		//sfprintf(stderr,buf,"x%zu",i);
		randstr(buf,1+in%8);
		if(expr_symset_add(es,buf,EXPR_CONSTANT,0,(double)i))++suc;
		vssuc=suc;
		vsi=i;
		if(i-l>=25000){
			fprintf(stderr,"added %zu/%zuth %-32s\r",suc,i,buf);
			fflush(stdout);
			l=i;
			getrandom(&in,4,GRND_NONBLOCK);
			srand(in);
		}
	}
	expr_symset_add(es,"mustin",EXPR_CONSTANT,0,-1.0);
	fprintf(stderr,"added %zu/%zuth %-32s\n",suc,i-1,buf);
	fprintf(stderr,"size:%zu depth:%zu length:%zu depth*length:\n",es->size,es->depth,es->length);
	fprintf(stdout,"%lg\n",(double)es->depth*es->length);
	fprintf(stderr,"creating time: %lg s\n",dtime()-st);
	if(!count)goto nosearch;
	fputs("searching\n",stderr);
	i=0;
	k=strlen(target);
	fprintf(stderr,"%s\n",target);
	st=dtime();
	while(--count)
		expr_symset_search(es,target,k);
	(esp=expr_symset_search(es,target,k))?
		fprintf(stderr,"found %s=%zd\n",esp->str,(ssize_t)esp->un.value):fputs("fail\n",stderr);
	fprintf(stderr,"searching time: %lg s\n",dtime()-st);
nosearch:
	expr_symset_free(es);
	return 0;
}
