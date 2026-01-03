/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "xgraph/header/expr.h"
#include <time.h>
#include <math.h>
#include <err.h>
#define psize(s) printf("sizeof(" #s ")=%zu\n",sizeof(s))
const char *t2s[]={
	[EXPR_CONSTANT]="Constant",
	[EXPR_VARIABLE]="Variable",
	[EXPR_FUNCTION]="Function",
	[EXPR_MDFUNCTION]="Multi-dimension function",
	[EXPR_MDEPFUNCTION]="Multi-dimension function*",
	[EXPR_HOTFUNCTION]="Hot function",
	[EXPR_ZAFUNCTION]="Zero-argument function"
};
void add_common_symbols(struct expr_symset *);
#define likely(cond) __builtin_expect(!!(cond),1)
#define unlikely(cond) __builtin_expect(!!(cond),0)
static void *xmalloc(size_t size){
	void *r;
	if(unlikely(size>=SSIZE_MAX)){
		warnx("IN xmalloc(size=%zu)\n"
			"CANNOT ALLOCATE MEMORY",size);
		goto ab;
	}
	r=malloc(size);
	if(unlikely(!r)){
		warn("IN xmalloc(size=%zu)\n"
			"CANNOT ALLOCATE MEMORY",size);
ab:
		warnx("ABORTING");
		abort();
	}
	return r;
}
static void *xrealloc(void *old,size_t size){
	void *r;
	if(unlikely(size>=SSIZE_MAX)){
		warnx("IN xrealloc(old=%p,size=%zu)\n"
			"CANNOT REALLOCATE MEMORY",old,size);
		goto ab;
	}
	r=realloc(old,size);
	if(unlikely(!r)){
		warn("IN xrealloc(old=%p,size=%zu)\n"
			"CANNOT REALLOCATE MEMORY",old,size);
ab:
		warnx("ABORTING");
		abort();
	}
	return r;
}
void list(void){
	srand48(time(NULL)+getpid());
	psize(struct expr);
	psize(struct expr_inst);
	psize(struct expr_suminfo);
	psize(struct expr_mdinfo);
	psize(struct expr_vmdinfo);
	psize(struct expr_branchinfo);
	psize(struct expr_symset);
	psize(struct expr_symbol);
	psize(struct expr_builtin_symbol);
	psize(struct expr_builtin_keyword);
	psize(struct expr_resource);
	for(const struct expr_builtin_keyword *p=expr_keywords;;++p){
		if(!p->str){
			printf("%zu keywords\n",p-expr_keywords);
			break;
		}
		printf("%-12s\tKeyword %s%s%s\t%s\n",p->str,p->flag&EXPR_KF_SUBEXPR?"S":" ",p->flag&EXPR_KF_SEPCOMMA?"C":" ",p->flag&EXPR_KF_NOPROTECT?"N":" ",p->desc);
	}
	printf("\n");
	for(const struct expr_builtin_symbol *p=expr_symbols;;++p){
		if(!p->str){
			printf("%zu symbols\n",p-expr_symbols);
			break;
		}
		printf("%-16s\t%-30s",p->str,t2s[p->type]);
		switch(p->type){
			case EXPR_CONSTANT:
				printf("%g",p->un.value);
				break;
			case EXPR_VARIABLE:
				printf("%g",*(double *)p->un.addr);
				break;
			case EXPR_FUNCTION:
				if(p->flag&EXPR_SF_INJECTION)
					printf("I ");
				if(p->flag&EXPR_SF_UNSAFE)
					printf("U ");
				break;
			case EXPR_MDFUNCTION:
			case EXPR_MDEPFUNCTION:
				if(p->flag&EXPR_SF_INJECTION)
					printf("I ");
				if(p->flag&EXPR_SF_UNSAFE)
					printf("U ");
				if(p->dim)
					printf("%hd",p->dim);
				else
					printf("no limit");
				break;
			case EXPR_ZAFUNCTION:
				if(p->flag&EXPR_SF_INJECTION)
					printf("I ");
				if(p->flag&EXPR_SF_UNSAFE)
					printf("U ");
				break;
			default:
				abort();

		}
		printf("\n");
	}
	return;
}
void list_symbol(struct expr_symbol *p){
	size_t dim;
	if(!p)
		return;
	printf("%-24s\t%-30s",p->str,t2s[p->type]);
	switch(p->type){
		case EXPR_CONSTANT:
			printf("value: %g",p->un.value);
			break;
		case EXPR_VARIABLE:
			printf("value: %g",*(double *)p->un.addr);
			break;
		case EXPR_FUNCTION:
			if(p->flag&EXPR_SF_INJECTION)
				printf("I ");
			if(p->flag&EXPR_SF_UNSAFE)
				printf("U ");
			break;
		case EXPR_MDFUNCTION:
		case EXPR_MDEPFUNCTION:
			if(p->flag&EXPR_SF_INJECTION)
				printf("I ");
			if(p->flag&EXPR_SF_UNSAFE)
				printf("U ");
			dim=*(size_t *)(p->str+strlen(p->str)+1);
			if(dim)
				printf("%zu",dim);
			else
				printf("no limit");
			break;
		case EXPR_ZAFUNCTION:
			if(p->flag&EXPR_SF_INJECTION)
				printf("I ");
			if(p->flag&EXPR_SF_UNSAFE)
				printf("U ");
			break;
		default:
			abort();

	}
	printf("\n");
	for(int i=0;i<EXPR_SYMNEXT;++i){
		list_symbol(p->next[i]);
	}
}
void list_common(void){
	struct expr_symset es[1]={EXPR_SYMSET_INITIALIZER};
	add_common_symbols(es);
	list_symbol(es->syms);
	printf("%zu common symbols,depth=%zu\n",es->size,es->depth);
	expr_symset_free(es);
}
int main(int argc,char **argv){
	if(argc<2){
		list();
	}else {
		expr_allocator=xmalloc;
		expr_reallocator=xrealloc;
		list_common();
	}
	return EXIT_SUCCESS;
}
