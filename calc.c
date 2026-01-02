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
char prefix[PREFIX_SIZE]={[0 ... (PREFIX_SIZE-1)]='-'};
unsigned long level=0;
ssize_t varindex(const struct expr *restrict ep,double *v){
	for(size_t i=0;i<ep->vsize;++i){
		if(ep->vars[i]==v)return i;
	}
	return -1;
}
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
void list(const struct expr *restrict ep,const struct expr_symset *restrict esp){
	char *sop=NULL,ssrc[EXPR_SYMLEN],sdst[EXPR_SYMLEN],ssym[EXPR_SYMLEN];
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
const struct option ops[]={
	{"safe",0,NULL,'p'},
	{"no-optimize",0,NULL,'n'},
	{"dump",0,NULL,'D'},
	{"timeout",2,NULL,'t'},
	{NULL},
};
void add_common_symbols(struct expr_symset *);
struct expr_symset *es=NULL;
char *rbuf=NULL;
__attribute__((destructor)) void atend(void){
	if(es)
		expr_symset_free(es);
	if(rbuf)
		free(rbuf);
}
void show_help(const char *a0){
	fprintf(stdout,"usage: %s [options] expression/-\n"
			"\t--safe, -p\twork on protected mode\n"
			"\t--no-optimize, -n\tdo nto optimize\n"
			"\t--dump, -D\tdump mode(do not evaluate)\n"
			,a0);
	exit(EXIT_SUCCESS);
}
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
int main(int argc,char **argv){
	char *e;
	int flag=0;
	int dump=0;
	double alarm_sec=0.0,r;
	struct expr ep[1];
	if(argc<2)
		errx(EXIT_FAILURE,"see --help");
	opterr=1;
	for(;;){
		switch(getopt_long(argc,argv,"pnDt::",ops,NULL)){
			case 'p':
				flag|=EXPR_IF_PROTECT;
				break;
			case 'n':
				flag|=EXPR_IF_NOOPTIMIZE;
				break;
			case 'D':
				dump=1;
				break;
			case 't':
				alarm_sec=optarg?atod2(optarg):1.0;
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
		errx(EXIT_FAILURE,"expression error:%s (%s)",expr_error(ep->error),ep->errinfo);
	}
	if(dump)
		list(ep,es);
	else {
		if(alarm_sec!=0.0){
			if(signal(SIGALRM,attimeout)==SIG_ERR)
				err(EXIT_FAILURE,"cannot set alarm handler");
			if(sigsetjmp(sjb,1)==1)
				errx(EXIT_FAILURE,"evaluation timed out");
			if(d_alarm(alarm_sec)<0)
				err(EXIT_FAILURE,"cannot set alarm");
		}
		r=expr_eval(ep,0);
		d_alarm(0.0);
		printdouble(r);
	}
	expr_free(ep);
	return EXIT_SUCCESS;
}
