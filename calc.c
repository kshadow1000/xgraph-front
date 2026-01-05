/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "xgraph/header/expr.h"
#include <time.h>
#include <signal.h>
#include <setjmp.h>
#include <err.h>
#include <getopt.h>
#include <errno.h>
#define BUFSIZE 4096
//dump
#include <stdarg.h>
#define PREFIX_SIZE 128
#define likely(cond) __builtin_expect(!!(cond),1)
#define unlikely(cond) __builtin_expect(!!(cond),0)
static void *xmalloc(size_t size){
	void *r;
	r=malloc(size);
	if(unlikely(!r)){
		warn("IN xmalloc(size=%zu)\n"
			"CANNOT ALLOCATE MEMORY",size);
		warnx("ABORTING");
		abort();
	}
	return r;
}
static void *xrealloc(void *old,size_t size){
	void *r;
	r=realloc(old,size);
	if(unlikely(!r)){
		warn("IN xrealloc(old=%p,size=%zu)\n"
			"CANNOT REALLOCATE MEMORY",old,size);
		warnx("ABORTING");
		abort();
	}
	return r;
}
char prefix[PREFIX_SIZE]={[0 ... (PREFIX_SIZE-1)]='-'};
unsigned long level=0;
void writeprefix(void){
	unsigned long n;
	n=level;
	while(n>=PREFIX_SIZE){
		fwrite(prefix,1,PREFIX_SIZE,stdout);
		n-=PREFIX_SIZE;
	}
	if(n)
		fwrite(prefix,1,n,stdout);
}
int xprintf(const char *fmt,...){
	va_list ap;
	int r;
	va_start(ap,fmt);
	writeprefix();
	r=vprintf(fmt,ap);
	va_end(ap);
	return r;
}
int addr2sym(const struct expr *restrict ep,const struct expr_symset *restrict esp,char buf[EXPR_SYMLEN],void *addr){
	union {
		const struct expr_symbol *es;
		const struct expr_builtin_symbol *ebs;
	} sym;
	if(!addr)return -1;
	if(addr==EXPR_VOID||addr==EXPR_VOID_NR){
		strcpy(buf,"EXPR_VOID");
		return 0;
	}
	sym.es=NULL;
	if(ep->sset)sym.es=expr_symset_rsearch(esp,addr);
	if(sym.es){
		strcpy(buf,sym.es->str);
		return 0;
	}
	sym.ebs=expr_builtin_symbol_rsearch(addr);
	if(sym.ebs){
		strcpy(buf,sym.ebs->str);
		return 0;
	}
	return -1;
}
#define idex(_ps,_indexof) \
const void **_ps=NULL;\
size_t _ps##_size=0;\
size_t _indexof(const struct expr *ep){\
	size_t i=0;\
	for(;i<_ps##_size;++i){\
		if(_ps[i]==ep)\
			return i;\
	}\
	_ps=_ps?xrealloc(_ps,(i+1)*sizeof(void *)):xmalloc((i+1)*sizeof(void *));\
	_ps[i]=ep;\
	_ps##_size=i+1;\
	return i;\
}\
__attribute__((destructor)) void _ps##atend(void){\
	if(_ps)\
		free(_ps);\
}
idex(eps,indexof);
idex(ps,indexofp);
idex(ups,indexofu);
enum odtype{NUL,MEM,SUM,BRANCH,HOT,MD,VMD,VAL,ZVAL};
char subexpr[]={[NUL]=0,[MEM]=0,[SUM]=1,[BRANCH]=1,[HOT]=1,[MD]=1,[VMD]=1,[VAL]=0,[ZVAL]=0,};
#define hassubexpr(_op) subexpr[ii[_op].st]
struct inst_info {
	const char *name;
	enum odtype st;
};
const struct inst_info ii[]={
[EXPR_COPY]={.name="copy",.st=MEM,},
[EXPR_INPUT]={.name="input",.st=NUL,},
[EXPR_CONST]={.name="const",.st=VAL,},
[EXPR_BL]={.name="bl",.st=MEM,},
[EXPR_ADD]={.name="add",.st=MEM,},
[EXPR_SUB]={.name="sub",.st=MEM,},
[EXPR_MUL]={.name="mul",.st=MEM,},
[EXPR_DIV]={.name="div",.st=MEM,},
[EXPR_MOD]={.name="mod",.st=MEM,},
[EXPR_POW]={.name="pow",.st=MEM,},
[EXPR_AND]={.name="and",.st=MEM,},
[EXPR_XOR]={.name="xor",.st=MEM,},
[EXPR_OR]={.name="or",.st=MEM,},
[EXPR_SHL]={.name="shl",.st=MEM,},
[EXPR_SHR]={.name="shr",.st=MEM,},
[EXPR_GT]={.name="gt",.st=MEM,},
[EXPR_GE]={.name="ge",.st=MEM,},
[EXPR_LT]={.name="lt",.st=MEM,},
[EXPR_LE]={.name="le",.st=MEM,},
[EXPR_SLE]={.name="sle",.st=MEM,},
[EXPR_SGE]={.name="sge",.st=MEM,},
[EXPR_SEQ]={.name="seq",.st=MEM,},
[EXPR_SNE]={.name="sne",.st=MEM,},
[EXPR_EQ]={.name="eq",.st=MEM,},
[EXPR_NE]={.name="ne",.st=MEM,},
[EXPR_ANDL]={.name="andl",.st=MEM,},
[EXPR_ORL]={.name="orl",.st=MEM,},
[EXPR_XORL]={.name="xorl",.st=MEM,},
[EXPR_NEXT]={.name="next",.st=MEM,},
[EXPR_DIFF]={.name="diff",.st=MEM,},
[EXPR_NEG]={.name="neg",.st=NUL,},
[EXPR_NOT]={.name="not",.st=NUL,},
[EXPR_NOTL]={.name="notl",.st=NUL,},
[EXPR_TSTL]={.name="tstl",.st=NUL,},
[EXPR_IF]={.name="if",.st=BRANCH,},
[EXPR_WHILE]={.name="while",.st=BRANCH,},
[EXPR_DO]={.name="do",.st=HOT,},
[EXPR_DOW]={.name="dow",.st=BRANCH,},
[EXPR_WIF]={.name="wif",.st=HOT,},
[EXPR_DON]={.name="don",.st=BRANCH,},
[EXPR_SUM]={.name="sum",.st=SUM,},
[EXPR_INT]={.name="int",.st=SUM,},
[EXPR_PROD]={.name="prod",.st=SUM,},
[EXPR_SUP]={.name="sup",.st=SUM,},
[EXPR_INF]={.name="inf",.st=SUM,},
[EXPR_ANDN]={.name="andn",.st=SUM,},
[EXPR_ORN]={.name="orn",.st=SUM,},
[EXPR_XORN]={.name="xorn",.st=SUM,},
[EXPR_GCDN]={.name="gcdn",.st=SUM,},
[EXPR_LCMN]={.name="lcmn",.st=SUM,},
[EXPR_LOOP]={.name="loop",.st=SUM,},
[EXPR_FOR]={.name="for",.st=SUM,},
[EXPR_ZA]={.name="za",.st=MEM,},
[EXPR_MD]={.name="md",.st=MD,},
[EXPR_ME]={.name="me",.st=MD,},
[EXPR_MEP]={.name="mep",.st=MD,},
[EXPR_VMD]={.name="vmd",.st=VMD},
[EXPR_DO1]={.name="do1",.st=HOT,},
[EXPR_EP]={.name="ep",.st=HOT,},
[EXPR_EVAL]={.name="eval",.st=MEM,},
[EXPR_HOT]={.name="hot",.st=HOT,},
[EXPR_PBL]={.name="pbl",.st=MEM,},
[EXPR_PZA]={.name="pza",.st=MEM,},
[EXPR_PMD]={.name="pmd",.st=MD,},
[EXPR_PME]={.name="pme",.st=MD,},
[EXPR_PMEP]={.name="pmep",.st=MD,},
[EXPR_READ]={.name="read",.st=MEM,},
[EXPR_WRITE]={.name="write",.st=MEM,},
[EXPR_OFF]={.name="off",.st=MEM,},
[EXPR_ALO]={.name="alo",.st=ZVAL,},
[EXPR_SJ]={.name="sj",.st=NUL,},
[EXPR_LJ]={.name="lj",.st=MEM,},
[EXPR_IP]={.name="ip",.st=NUL,},
[EXPR_IPP]={.name="ipp",.st=NUL,},
[EXPR_TO]={.name="to",.st=NUL,},
[EXPR_TO1]={.name="to1",.st=NUL,},
[EXPR_END]={.name="end",.st=NUL,},
};
ssize_t varindex(const struct expr *restrict ep,double *v){
	for(size_t i=0;i<ep->vsize;++i){
		if(ep->vars[i]==v)return i;
	}
	return -1;
}
const char *symbolof(const struct expr *restrict ep,void *addr){
	union {
		const struct expr_symbol *es;
		const struct expr_builtin_symbol *ebs;
	} sym;
	sym.es=NULL;
	if(ep->sset)
		sym.es=expr_symset_rsearch(ep->sset,addr);
	if(sym.es){
		return sym.es->str;
	}
	sym.ebs=expr_builtin_symbol_rsearch(addr);
	if(sym.ebs){
		return sym.ebs->str;
	}
	return NULL;
}
const char *od(const struct expr *restrict ep,void *addr){
	static char abuf[64+EXPR_SYMLEN];
	ssize_t index;
	const char *p;
	if(addr==EXPR_VOID||addr==EXPR_VOID_NR){
		strcpy(abuf,"[void]");
		return abuf;
	}
	index=varindex(ep,addr);
	if(index>=0){
		sprintf(abuf,"[%zd]",index);
		return abuf;
	}
	p=symbolof(ep,addr);
	if(p){
		strcpy(abuf,p);
		return abuf;
	}
	sprintf(abuf,"u[%zd]",indexofu(addr));
	return abuf;
}
const char *ainst(const struct expr *restrict ep,struct expr_inst *ip){
	static char abuf[128+EXPR_SYMLEN];
	const char *p;
	int r=sprintf(abuf,"%-5s %-6s",ii[ip->op].name,od(ep,ip->dst.uaddr));
	switch(ii[ip->op].st){
		case NUL:
			break;
		case MEM:
			sprintf(abuf+r," %s",od(ep,ip->un.uaddr));
			break;
		case SUM:
			sprintf(abuf+r," sum[%zu,%zu,%zu,%zu]",indexof(ip->un.es->fromep),indexof(ip->un.es->toep),indexof(ip->un.es->stepep),indexof(ip->un.es->ep));
			break;
		case BRANCH:
			sprintf(abuf+r," branch[%zu,%zu,%zu]",indexof(ip->un.eb->cond),indexof(ip->un.eb->body),indexof(ip->un.eb->value));
			break;
		case MD:
			p=symbolof(ep,ip->un.em->un.uaddr);
			if(p)
				sprintf(abuf+r," %s(%zu)",p,ip->un.em->dim);
			else
				sprintf(abuf+r," (mdinfo)heap[%zu]",indexofp(ip->un.uaddr));
			break;
		case VMD:
			sprintf(abuf+r," vmd[%zu,%zu,%zu,%zu](%zu)",indexof(ip->un.ev->fromep),indexof(ip->un.ev->toep),indexof(ip->un.ev->stepep),indexof(ip->un.ev->ep),ip->un.ev->max);
			break;
		case HOT:
			sprintf(abuf+r," expr[%zu]",indexof(ip->un.hotfunc));
			break;
		case VAL:
			sprintf(abuf+r," %lg",ip->un.value);
			break;
		case ZVAL:
			sprintf(abuf+r," %zu",ip->un.zu);
			break;
		default:
			sprintf(abuf+r," (unknown)heap[%zu]",indexofp(ip->dst.uaddr));
			break;
	}
	return abuf;
}
void list(const struct expr *restrict ep,const struct expr_symset *restrict esp){
	char ssrc[EXPR_SYMLEN],sdst[EXPR_SYMLEN],ssym[EXPR_SYMLEN];
	const char *sop=NULL;
	ssize_t index;
	xprintf("%zu instructions %zu vars in total\n",ep->size,ep->vsize);
	for(struct expr_inst *ip=ep->data;ip-ep->data<ep->size;++ip){
		*ssrc=0;
		*sdst=0;
		switch(ip->op){
			case EXPR_COPY:
				sop="copy";
				break;
			case EXPR_CONST:
				sop="const";
				sprintf(ssrc,"%g",ip->un.value);
				break;
			case EXPR_ALO:
				sop="alo";
				sprintf(ssrc,"%zd",ip->un.zd);
				break;
			case EXPR_INPUT:
					sop="input";
					strcpy(ssrc," ");
					break;
			case EXPR_IP:
					sop="ip";
					strcpy(ssrc," ");
					break;
			case EXPR_IPP:
					sop="ipp";
					strcpy(ssrc," ");
					break;
			case EXPR_TO:
					sop="to";
					strcpy(ssrc," ");
					break;
			case EXPR_TO1:
					sop="to1";
					strcpy(ssrc," ");
					break;
			case EXPR_BL:sop="bl";break;
			case EXPR_PBL:sop="pbl";break;
			case EXPR_READ:sop="read";break;
			case EXPR_WRITE:sop="write";break;
			case EXPR_OFF:sop="off";break;
			case EXPR_ZA:sop="za";break;
			case EXPR_EVAL:sop="eval";break;
			case EXPR_PZA:sop="pza";break;
			case EXPR_ADD:sop="add";break;
			case EXPR_SUB:sop="sub";break;
			case EXPR_NEXT:sop="next";break;
			case EXPR_DIFF:sop="diff";break;
			case EXPR_MUL:sop="mul";break;
			case EXPR_DIV:sop="div";break;
			case EXPR_MOD:sop="mod";break;
			case EXPR_POW:sop="pow";break;
			case EXPR_AND:sop="and";break;
			case EXPR_OR:sop="or";break;
			case EXPR_XOR:sop="xor";break;
			case EXPR_SHL:sop="shl";break;
			case EXPR_SHR:sop="shr";break;
			case EXPR_LJ:sop="lj";break;
			case EXPR_SJ:
					sop="sj";
					strcpy(ssrc," ");
					break;
			case EXPR_NEG:
					sop="neg";
					strcpy(ssrc," ");
					break;
			case EXPR_NOT:
					sop="not";
					strcpy(ssrc," ");
					break;
			case EXPR_NOTL:
					sop="notl";
					strcpy(ssrc," ");
					break;
			case EXPR_TSTL:
					sop="tstl";
					strcpy(ssrc," ");
					break;
			case EXPR_IF:sop="if";goto branch;
			case EXPR_WHILE:sop="while";goto branch;
			case EXPR_DON:sop="don";goto branch;
			case EXPR_DOW:sop="dow";goto branch;
			case EXPR_SUM:sop="sum";goto sum;
			case EXPR_INT:sop="int";goto sum;
			case EXPR_PROD:sop="prod";goto sum;
			case EXPR_SUP:sop="sup";goto sum;
			case EXPR_INF:sop="inf";goto sum;
			case EXPR_ANDN:sop="andn";goto sum;
			case EXPR_ORN:sop="orn";goto sum;
			case EXPR_XORN:sop="xorn";goto sum;
			case EXPR_GCDN:sop="gcdn";goto sum;
			case EXPR_LCMN:sop="lcmn";goto sum;
			case EXPR_LOOP:sop="loop";goto sum;
			case EXPR_FOR:sop="for";goto sum;
			case EXPR_MD:sop="md";goto md;
			case EXPR_ME:sop="me";goto md;
			case EXPR_PMD:sop="pmd";goto md;
			case EXPR_PME:sop="pme";goto md;
			case EXPR_PMEP:sop="pmep";goto md;
			case EXPR_MEP:sop="mep";goto md;
			case EXPR_VMD:sop="vmd";goto vmd;
			case EXPR_DO:sop="do";goto hot;
			case EXPR_DO1:sop="do1";goto hot;
			case EXPR_WIF:sop="wif";goto hot;
			case EXPR_EP:sop="ep";goto hot;
			case EXPR_HOT:
					sop="hot";
hot:
					level+=4;
					xprintf("hot function %p\n",ip->un.hotfunc);
					list(ip->un.hotfunc,esp);
					level-=4;
					break;
			case EXPR_GT:sop="gt";break;
			case EXPR_GE:sop="ge";break;
			case EXPR_LT:sop="lt";break;
			case EXPR_LE:sop="le";break;
			case EXPR_SEQ:sop="seq";break;
			case EXPR_SNE:sop="sne";break;
			case EXPR_SGE:sop="sge";break;
			case EXPR_SLE:sop="sle";break;
			case EXPR_EQ:sop="eq";break;
			case EXPR_NE:sop="ne";break;
			case EXPR_ANDL:sop="andl";break;
			case EXPR_ORL:sop="orl";break;
			case EXPR_XORL:sop="xorl";break;
			case EXPR_END:
					sop="end";
					strcpy(ssrc," ");
					break;
sum:
					level+=4;
					xprintf("struct expr_suminfo %p index:%p\n",ip->un.es,&ip->un.es->index);
					xprintf("%p->fromep\n",ip->un.es);
					list(ip->un.es->fromep,esp);
					xprintf("%p->toep\n",ip->un.es);
					list(ip->un.es->toep,esp);
					xprintf("%p->stepep\n",ip->un.es);
					list(ip->un.es->stepep,esp);
					xprintf("%p->ep\n",ip->un.es);
					list(ip->un.es->ep,esp);
					level-=4;
					break;
branch:
					level+=4;
					xprintf("struct expr_branchinfo %p\n",ip->un.eb);
					xprintf("%p->cond\n",ip->un.eb);
					list(ip->un.eb->cond,esp);
					xprintf("%p->body\n",ip->un.eb);
					list(ip->un.eb->body,esp);
					xprintf("%p->value\n",ip->un.eb);
					list(ip->un.eb->value,esp);
					level-=4;
					break;
md:
					level+=4;
					if(addr2sym(ep,esp,ssym,ip->un.em->un.func)<0)
						sprintf(ssym,"%p",ip->un.em->un.func);
					xprintf("struct expr_mdinfo %p dim=%zu func:%s\n",ip->un.em,ip->un.em->dim,ssym);
					for(size_t i=0;i<ip->un.em->dim;++i){
					xprintf("dimension %zu\n",i);
					list(ip->un.em->eps+i,esp);
					}
					level-=4;
					break;
vmd:
					level+=4;
					if(addr2sym(ep,esp,ssym,ip->un.ev->func)<0)
						sprintf(ssym,"%p",ip->un.ev->func);
					xprintf("struct expr_vmdinfo %p index:%p func:%s\n",ip->un.ev,&ip->un.ev->index,ssym);
					xprintf("%p->fromep\n",ip->un.ev);
					list(ip->un.ev->fromep,esp);
					xprintf("%p->toep\n",ip->un.ev);
					list(ip->un.ev->toep,esp);
					xprintf("%p->stepep\n",ip->un.ev);
					list(ip->un.ev->stepep,esp);
					xprintf("%p->ep\n",ip->un.ev);
					list(ip->un.ev->ep,esp);
					level-=4;
					break;

		}
		if(!sop)abort();
		if(!*ssrc){
			index=varindex(ep,ip->un.src);
			if(index>=0){
				if(expr_isnan(*ip->un.src))
					sprintf(ssrc,"vars[%zd]",index);
				else
					sprintf(ssrc,"vars[%zd]=%g",index,*ip->un.src);
			}else if(addr2sym(ep,esp,ssrc,ip->un.src)<0){
				sprintf(ssrc,"%p",ip->un.src);
			}
		}
		if(!*sdst){
			index=varindex(ep,ip->dst.dst);
			if(index>=0)if(expr_isnan(*ip->dst.dst))
				sprintf(sdst,"vars[%zd]",index);
			else
				sprintf(sdst,"vars[%zd]=%g",index,*ip->dst.dst);
			else if(addr2sym(ep,esp,sdst,ip->dst.dst)<0)
				sprintf(sdst,"%p",ip->dst.dst);
		}
		xprintf("%-8s%s\t%s\n",sop,sdst,ssrc);
	}
}
//
void *readall(int fd,ssize_t *len){
	char *buf,*p;
	size_t bufsiz,r1;
	ssize_t r,ret=0;
	bufsiz=BUFSIZE;
	if((buf=malloc(BUFSIZE))==NULL){
		if(len)
			*len=-1;
		return NULL;
	}
	r1=0;
	while((r=read(fd,buf+ret,BUFSIZE-r1))>0){
		r1+=r;
		ret+=r;
		if(ret==bufsiz){
			bufsiz+=BUFSIZE;
			if((p=realloc(buf,bufsiz))==NULL){
				free(buf);
				return NULL;
			}
			buf=p;
			r1=0;
		}else
			break;
	}
	if(ret==bufsiz){
		if((p=realloc(buf,bufsiz+1))==NULL){
			free(buf);
			return NULL;
		}
	buf=p;
	}
	buf[ret]=0;
	if(len)
		*len=ret;
	return buf;
}
static int x=0;
void printdouble(double val){
	char *buf,*p;
	asprintf(&buf,x?"%.1024la":"%.1024lf",val);
	p=strchr(buf,'.');
	if(p){
		p+=strlen(p);
		while(*(--p)=='0')*p=0;
		if(*p=='.')*p=0;

	}
	puts(buf);
	free(buf);
}
double d_alarm(double);
void add_common_symbols(struct expr_symset *);
struct expr_symset *es=NULL;
char *rbuf=NULL;
double atod2(const char *str){
	double r;
	/*char *c;
	r=strtod(str,&c);
	if(c==str||*c)
		errx(EXIT_FAILURE,"invaild double %s",str);*/
	int error=0;
	char err[EXPR_SYMLEN];
	r=expr_calc5(str,&error,err,NULL,EXPR_IF_PROTECT|EXPR_IF_NOKEYWORD);
	if(error)
		errx(EXIT_FAILURE,"invaild expression: %s (%s:%s)",str,expr_error(error),err);
	return r;
}
sigjmp_buf sjb;
void attimeout(int sig){
	switch(sig){
		case SIGALRM:
			siglongjmp(sjb,1);
		default:
			break;
	}
}
__attribute__((destructor)) void atend(void){
	if(es)
		expr_symset_free(es);
	if(rbuf)
		free(rbuf);
}
size_t line=0;
#define STACK_SIZE (128*1024)
size_t sstack[STACK_SIZE]={0};
size_t *ssp=sstack;
void push(size_t n){
	if(ssp-sstack>=STACK_SIZE)
		return;
	*(ssp++)=n;
}
size_t pop(void){
	if(ssp<sstack)
		return -1;
	return *(--ssp);
}
int gchr=0;
#define stop(...) if(gchr)getchar()
void callee(const struct expr *ep,struct expr_inst *ip,void *arg){
	static char abuf[128];
	sprintf(abuf,"%zu:expr[%zu]->data[%td]",line++,indexof(ep),ip-ep->data);
	if(hassubexpr(ip->op)){
		printf("%-26s %-32s begin {\n",abuf,ainst(ep,ip));
		push(line-1);
		stop();
	}else
		printf("%-26s %-28s ...",abuf,ainst(ep,ip));
}
void callee_end(const struct expr *ep,struct expr_inst *ip,void *arg){
	static char abuf[128];
	if(hassubexpr(ip->op)){
		sprintf(abuf,"%zu:expr[%zu]->data[%td]",pop(),indexof(ep),ip-ep->data);
		printf("%-57s } ok dst=%lg\n",abuf,*ip->dst.dst);
	}else {
		printf(" ok dst=%lg\n",*ip->dst.dst);
	}
	stop();
}
struct expr_callback ec={
	.before=callee,
	.after=callee_end,
	.arg=NULL,
};
double callbackmode(struct expr *ep,double input){
	line=0;
	return expr_callback(ep,input,&ec);
}
double stepmode(struct expr *ep,double input){
	struct expr_inst *ip=ep->data;
	double r;
	do{
		printf("%-26s dst=%lg\n",ainst(ep,ip),*ip->dst.dst);
	}while(!expr_step(ep,input,&r,&ip));
	return r;
}
const struct option ops[]={
	{"safe",0,NULL,'p'},
	{"no-optimize",0,NULL,'n'},
	{"no-builtin",0,NULL,'N'},
	{"dump",0,NULL,'D'},
	{"timeout",2,NULL,'t'},
	{"count",1,NULL,0xff01},
	{"injection",0,NULL,'i'},
	{"step",0,NULL,'s'},
	{"callback",0,NULL,'c'},
	{"keep",0,NULL,'k'},
	{"detach",0,NULL,'d'},
	{"getchar",0,NULL,'g'},
	{"help",0,NULL,'h'},
	{NULL},
};
void show_help(const char *a0){
	fprintf(stdout,"usage: %s [options] expression/-\n"
			"\t--safe, -p\twork on protected mode\n"
			"\t--no-optimize, -n\tdo not optimize\n"
			"\t--no-builtin, -N\tdo not use builtin symbols\n"
			"\t--dump, -D\tdump mode(do not evaluate)\n"
			"\t--timeout[=seconds,default 1], -t\tset the timeout for evaluation\n"
			"\t--count count\tevaluate how many times,default 1\n"
			"\t--injection, -i\tuse injective function only\n"
			"\t--step, -s\tstep mode\n"
			"\t--callback, -c\tcallback mode\n"
			"\t--keep, -k\tkeep symbol sets,use with -d to make symbols visible\n"
			"\t--detach, -d\tdetach each symbol sets,use with -k to make symbols visible\n"
			"\t--getchar, -g\tcall getchar() on each instructions if -s/-c is set\n"
			,a0);
	exit(EXIT_SUCCESS);
}
int main(int argc,char **argv){
	char *e;
	int flag=0;
	int dump=0;
	enum {NORMAL,STEP,CALLBACK} mode=NORMAL;
	size_t count=1;
	double alarm_sec=0.0,r;
	struct expr ep[1];
	sighandler_t sigold;
	setvbuf(stdout,NULL,_IONBF,0);
	if(argc<2)
		errx(EXIT_FAILURE,"see --help");
	opterr=1;
	for(;;){
		switch(getopt_long(argc,argv,"pnDt::Nisckdg",ops,NULL)){
			case 'p':
				flag|=EXPR_IF_PROTECT;
				break;
			case 'n':
				flag|=EXPR_IF_NOOPTIMIZE;
				break;
			case 'N':
				flag|=EXPR_IF_NOBUILTIN;
				break;
			case 'i':
				flag|=EXPR_IF_INJECTION;
				break;
			case 'k':
				flag|=EXPR_IF_KEEPSYMSET;
				break;
			case 'd':
				flag|=EXPR_IF_DETACHSYMSET;
				break;
			case 'g':
				gchr=1;
				break;
			case 'D':
				dump=1;
				break;
			case 's':
				mode=STEP;
				break;
			case 'c':
				mode=CALLBACK;
				break;
			case 't':
				alarm_sec=optarg?atod2(optarg):1.0;
				break;
			case 0xff01:
				count=(size_t)atod2(optarg);
				break;
			case 'h':
				show_help(*argv);
				break;
			case -1:
				goto break2;
			case '?':
				exit(EXIT_FAILURE);
				break;
		}
	}
break2:
	if(optind>=argc){
		err(EXIT_FAILURE,"expression not found");
	}
	e=argv[optind];
	if(!strcmp(e,"-")){
		rbuf=readall(STDIN_FILENO,NULL);
		if(!rbuf)
			err(EXIT_FAILURE,"cannot read expression frm stdin");
		e=rbuf;
	}
	srand48(getpid()^time(NULL));
	es=new_expr_symset();
	if(!es)
		err(EXIT_FAILURE,"cannot allocate memory");
	add_common_symbols(es);
	if(init_expr5(ep,e,"t",es,flag)<0){
		if(*ep->errinfo)
			errx(EXIT_FAILURE,"expression error:%s \"%s\"",expr_error(ep->error),ep->errinfo);
		else
			errx(EXIT_FAILURE,"expression error:%s",expr_error(ep->error));
	}
	if(dump)
		list(ep,es);
	else {
		if(alarm_sec!=0.0){
			sigold=signal(SIGALRM,attimeout);
			if(sigold==SIG_ERR)
				err(EXIT_FAILURE,"cannot set alarm handler");
			if(sigsetjmp(sjb,1)==1)
				errx(EXIT_FAILURE,"evaluation timed out");
			if(d_alarm(alarm_sec)<0)
				err(EXIT_FAILURE,"cannot set alarm");
		}
		do {
			switch(mode){
				case NORMAL:
					r=expr_eval(ep,0);
					break;
				case STEP:
					r=stepmode(ep,0);
					break;
				case CALLBACK:
					r=callbackmode(ep,0);
					break;
			}
		}while(--count);
		if(alarm_sec!=0.0){
			d_alarm(0.0);
			signal(SIGALRM,sigold);
		}
		printdouble(r);
	}
	expr_free(ep);
	return EXIT_SUCCESS;
}
