#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include "xgraph/header/expr.h"
#include <errno.h>
#include "readall.c"
#include <time.h>
#include <math.h>
#include <err.h>
void add_common_symbols(struct expr_symset *es);
char prefix[1024]={0};
void level_inc(void){
	strcat(prefix,"----");
}
ssize_t varindex(const struct expr *restrict ep,double *v){
	for(size_t i=0;i<ep->vsize;++i){
	//	printf("checking %p at %p\n",v,ep->vars[i]);
		if(ep->vars[i]==v)return i;
	}
	return -1;
}
void level_dec(void){
	memset(prefix+strlen(prefix)-4,0,4);
}
int xprintf(const char *fmt,...){
	va_list ap;
	int r;
	va_start(ap,fmt);
	r=printf("%s",prefix);
	r+=vprintf(fmt,ap);
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
					level_inc();
					xprintf("hot function %p\n",ip->un.hotfunc);
					list(ip->un.hotfunc,esp);
					level_dec();
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
			//case EXPR_ASSIGN:sop="assign";break;
			case EXPR_END:
					sop="end";
					strcpy(ssrc," ");
					break;
sum:
					level_inc();
					xprintf("struct expr_suminfo %p index:%p\n",ip->un.es,&ip->un.es->index);
					xprintf("%p->fromep\n",ip->un.es);
					list(ip->un.es->fromep,esp);
					xprintf("%p->toep\n",ip->un.es);
					list(ip->un.es->toep,esp);
					xprintf("%p->stepep\n",ip->un.es);
					list(ip->un.es->stepep,esp);
					xprintf("%p->ep\n",ip->un.es);
					list(ip->un.es->ep,esp);
					level_dec();
					break;
branch:
					level_inc();
					xprintf("struct expr_branchinfo %p\n",ip->un.eb);
					xprintf("%p->cond\n",ip->un.eb);
					list(ip->un.eb->cond,esp);
					xprintf("%p->body\n",ip->un.eb);
					list(ip->un.eb->body,esp);
					xprintf("%p->value\n",ip->un.eb);
					list(ip->un.eb->value,esp);
					level_dec();
					break;
md:
					level_inc();
					if(addr2sym(ep,esp,ssym,ip->un.em->un.func)<0)
						sprintf(ssym,"%p",ip->un.em->un.func);
					xprintf("struct expr_mdinfo %p dim=%zu func:%s\n",ip->un.em,ip->un.em->dim,ssym);
					for(size_t i=0;i<ip->un.em->dim;++i){
					xprintf("dimension %zu\n",i);
					list(ip->un.em->eps+i,esp);
					}
					level_dec();
					break;
vmd:
					level_inc();
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
					level_dec();
					break;

		}
		if(!sop)abort();
		if(!*ssrc){
			index=varindex(ep,ip->un.src);
			if(index>=0){
				if(isnan(*ip->un.src))
					sprintf(ssrc,"vars[%zd]",index);
				else
					sprintf(ssrc,"vars[%zd]=%g",index,*ip->un.src);
			}else if(addr2sym(ep,esp,ssrc,ip->un.src)<0){
				sprintf(ssrc,"%p",ip->un.src);
			}
		}
		if(!*sdst){
			index=varindex(ep,ip->dst.dst);
			if(index>=0)if(isnan(*ip->dst.dst))
				sprintf(sdst,"vars[%zd]",index);
			else
				sprintf(sdst,"vars[%zd]=%g",index,*ip->dst.dst);
			else if(addr2sym(ep,esp,sdst,ip->dst.dst)<0)
				sprintf(sdst,"%p",ip->dst.dst);
		}
		xprintf("%-8s%s\t%s\n",sop,sdst,ssrc);
	}
}
int main(int argc,char **argv){
	char *buf,*p,*p1,*e;
	int flag=0;
	if(argc<2)
	errx(EXIT_FAILURE,"no expression input");
	struct expr_symset *es=new_expr_symset();
	//char buf[9999]={"sum(n,1,100,1,n)"};
	struct expr ep[1];
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
				//->flag=EXPR_SF_INJECTION
		}else if(!strcmp(argv[i],"--no"))
			flag|=EXPR_IF_NOOPTIMIZE;
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
		errx(EXIT_FAILURE,"expression error:%s (%s)",expr_error(ep->error),ep->errinfo);
	}
	list(ep,ep->sset);
	expr_free(ep);
	expr_symset_free(es);
	return 0;
}
