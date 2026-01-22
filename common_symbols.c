/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "xgraph/expr/expr.h"
#include <time.h>
#include <math.h>
#include "prime.c"
#include <float.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/random.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <syscall.h>
#include <arpa/inet.h>
#include <netdb.h>
#define printval(x) warn(#x ":%lu",(unsigned long)(x))
#define printvali(x) warn(#x ":%d",(int)(x))
#define printvall(x) warn(#x ":%ld",(long)(x))
#define printvald(x) warn(#x ":%lf",(double)(x))
#define warn(fmt,...) fprintf(stderr,fmt "\n",##__VA_ARGS__)
#define casting(x,T) ((T)(x))
#define cast(x,T) expr_cast(x,T)
/*_Generic(T,\
		void *:expr_cast(x,T),\
		double:(x),\
		default:(T)(x)\
		)
		,void *:expr_cast(x,double)\*/
#define castingd(x) ((double)(x))
//_Generic((x),void *:(expr_cast(x,double)),default:((double)(x)))
#define warp1(rtype,sym,atype) static double d_##sym(double x){\
	rtype r=(rtype)sym(casting(x,atype));\
	return castingd(r);\
}
#define warpip(rtype,sym,atype) static double d_##sym(double x){\
	rtype r=(rtype)sym(cast(x,atype));\
	return castingd(r);\
}
#define warppi(rtype,sym,atype) static double d_##sym(double x){\
	rtype r=(rtype)sym(casting(x,atype));\
	return cast(r,double);\
}
#define warp2(rtype,sym,at0,at1) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(casting(v[0],at0),casting(v[1],at1));\
	return castingd(r);\
}
#define warppip(rtype,sym,at0,at1) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(casting(v[0],at0),cast(v[1],at1));\
	return expr_cast(r,double);\
}
#define warpppi(rtype,sym,at0,at1) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(cast(v[0],at0),casting(v[1],at1));\
	return expr_cast(r,double);\
}
#define warp3(rtype,sym,at0,at1,at2) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(casting(v[0],at0),casting(v[1],at1),casting(v[2],at2));\
	return castingd(r);\
}
#define warpiipi(rtype,sym,at0,at1,at2) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(casting(v[0],at0),cast(v[1],at1),casting(v[2],at2));\
	return castingd(r);\
}
#define warpiipp(rtype,sym,at0,at1,at2) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(casting(v[0],at0),cast(v[1],at1),cast(v[2],at2));\
	return castingd(r);\
}
#define warpipii(rtype,sym,at0,at1,at2) static double d_##sym(double *v,size_t n){\
	rtype r=(rtype)sym(cast(v[0],at0),n>1?casting(v[1],at1):0,n>2?casting(v[2],at2):0);\
	return castingd(r);\
}
#define warpz(rtype,sym) static double d_##sym(void){\
	rtype r=(rtype)sym();\
	return castingd(r);\
}
warpz(pid_t,fork)
warpz(pid_t,vfork)
warpz(pid_t,getpid)
warpz(pid_t,getppid)
warpz(pid_t,gettid)
warpz(pid_t,getuid)
warpz(pid_t,getgid)
warpz(pid_t,geteuid)
warpz(pid_t,getegid)
warp1(int,setuid,uid_t)
warp1(int,setgid,gid_t)
warp1(int,seteuid,uid_t)
warp1(int,setegid,gid_t)
warp1(int,close,int)
warp1(int,raise,int)
warp1(uint32_t,htonl,uint32_t)
warp1(uint16_t,htons,uint16_t)
warppi(void *,strerror,int)
warpip(size_t,strlen,void *)
warpppi(void *,strchr,void *,int)
warpip(pid_t,wait,void *)
warpip(in_addr_t,inet_addr,void *)
warp2(int,kill,pid_t,int)
warp2(int,listen,int,int)
warppip(void *,signal,int,void *)
warpipii(int,open,void *,int,int)
warp3(int,socket,int,int,int)
int tgkill(int,int,int);
warp3(int,tgkill,int,int,int)
warpiipi(int,bind,int,struct sockaddr *,socklen_t)
warpiipi(int,connect,int,struct sockaddr *,socklen_t)
warpiipp(int,accept,int,struct sockaddr *,socklen_t *)
double last_sig;
struct expr *volatile sigep[64+1];
const struct addrinfo ai_req_icmp[1]={{
	.ai_family=AF_INET,
	.ai_socktype=SOCK_DGRAM,
	//.ai_protocol=IPPROTO_ICMP,
}};
static in_addr_t inet_addr2(const char *cp){
	struct addrinfo *ai0,*ai;
	in_addr_t ret=INADDR_NONE;
	if(getaddrinfo(cp,NULL,ai_req_icmp,&ai0)<0)
		return INADDR_NONE;
	for(ai=ai0;ai;ai=ai->ai_next){
		//if(ai->ai_addrlen!=16)
		//	continue;
		ret=((struct sockaddr_in *)ai->ai_addr)->sin_addr.s_addr;
		//if(ret!=INADDR_NONE)
		//	break;
	}
	freeaddrinfo(ai0);
	return ret;
}
in_addr_t inet_addr(const char *cp) __attribute__((alias("inet_addr2")));
void d_setsig(int sig){
 	last_sig=(double)sig;
}
void d_sigep(int sig){
	expr_eval(sigep[sig],(double)sig);
}
double d_signalep(double *v,size_t n){
	int s=(int)v[0];
	sigep[s]=cast(v[1],struct expr *);
	return cast(signal(s,d_sigep),double);
}
int vfdprintf_atomic(int fd,const char *restrict format,va_list ap){
	int r;
	char buf[PIPE_BUF];
	if((r=vsnprintf(buf,PIPE_BUF,format,ap))==EOF)return EOF;
	return write(fd,buf,r>PIPE_BUF?PIPE_BUF:r);
}
int fdprintf_atomic(int fd,const char *restrict format,...){
	int r;
	va_list ap;
	va_start(ap,format);
	r=vfdprintf_atomic(fd,format,ap);
	va_end(ap);
	return r;
}
int fprintd(int fd,double v){
	char buf[PIPE_BUF],*p;
	sprintf(buf,"%.64lf",v);
	p=strchr(buf,'.');
	if(p){
		p+=strlen(p);
		while(*(--p)=='0')*p=0;
		if(*p=='.')*p=0;

	}
	return fdprintf_atomic(fd,"%s\n",buf);
}
int fprintda(int fd,double *v,size_t n){
	int r;
	if(!n)return 0;
	r=0;
	do
		r+=fprintd(fd,*(v++));
	while(--n);
	return r;
}
double dtime(void){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	return (double)ts.tv_sec+ts.tv_nsec/1000000000.0;
}
double d_sleep(double x){
	struct timespec rts,ts;
	double fx;
	x=fabs(x);
	fx=floor(x);
	ts.tv_sec=(time_t)fx;
	ts.tv_nsec=(time_t)((x-fx)*1000000000.0);
	memset(&rts,0,sizeof(struct timespec));
	nanosleep(&ts,&rts);
	return (double)rts.tv_sec+rts.tv_nsec/1000000000.0;
}

#include <sys/time.h>
double d_alarm(double x){
	struct itimerval rtv,tv;
	double fx;
	x=fabs(x);
	fx=floor(x);
	tv.it_value.tv_sec=(time_t)fx;
	tv.it_value.tv_usec=(time_t)((x-fx)*1000000.0);
	memset(&tv.it_interval,0,sizeof(struct timeval));
	memset(&rtv,0,sizeof(struct itimerval));
	if(setitimer(ITIMER_REAL,&tv,&rtv)<0)
		return -1.0;
	return (double)rtv.it_value.tv_sec+rtv.it_value.tv_usec/1000000000.0;
}
#define a2s(buf,args,size) buf=alloca(size+1);\
	for(size_t i=0;i<size;++i)\
		buf[i]=(char)*(args++)
double d_write(double *args,size_t n){
	size_t size;
	union {
		void *buf;
		double dr;
	} un;
	int fd;
	fd=(int)*args;
	un.dr=*(++args);
	size=(size_t)*(++args);
	return write(fd,un.buf,size);
}
double d_read(double *args,size_t n){
	size_t size;
	union {
		void *buf;
		double dr;
	} un;
	int fd;
	fd=(int)*args;
	un.dr=*(++args);
	size=(size_t)*(++args);
	return read(fd,un.buf,size);
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
double d_printf(double *args,size_t n){
	const char *fmt=cast(*args,const char *);
	size_t flen=strlen(fmt);
	return (double)expr_writef(fmt,flen,(ssize_t (*)(intptr_t,const void *,size_t))write,STDOUT_FILENO,(void **)args+1,n-1);
}
double d_printk(double *args,size_t n){
	const char *fmt=cast(*args,const char *);
	int r;
	//pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
	FILE *fp=NULL;
	//pthread_mutex_lock(&mutex);
	//if(!fp)
		fp=fopen("/dev/kmsg","w");
	if(!fp)goto err;
	switch(n){
		case 1:r=fprintf(fp,fmt);break;
		case 2:r=fprintf(fp,fmt,args[1]);break;
		case 3:r=fprintf(fp,fmt,args[1],args[2]);break;
		case 4:r=fprintf(fp,fmt,args[1],args[2],args[3]);break;
		case 5:r=fprintf(fp,fmt,args[1],args[2],args[3],args[4]);break;
		case 6:r=fprintf(fp,fmt,args[1],args[2],args[3],args[4],args[5]);break;
		case 7:r=fprintf(fp,fmt,args[1],args[2],args[3],args[4],args[5],args[6]);break;
		case 8:r=fprintf(fp,fmt,args[1],args[2],args[3],args[4],args[5],args[6],args[7]);break;
		default:r=fprintf(fp,"Too many args!");break;
	}
	//pthread_mutex_unlock(&mutex);
	fclose(fp);
	//fp=NULL;
	return (double)r;
err:
	//pthread_mutex_unlock(&mutex);
	return -1.0;
}
#pragma GCC diagnostic pop
/*double d_inet_addr(double *args,size_t n){
	switch(n){
		case 1:
			return (double)(uint32_t)*args;
		case 2:
			return (double)((uint32_t)args[0]+
				       ((uint32_t)args[1]<<16)
				       );
		case 3:
			return (double)((uint32_t)args[0]+
				       ((uint32_t)args[1]<<8)+
				       ((uint32_t)args[2]<<16)
				       );
		case 4:
			return (double)((uint32_t)args[0]+
				       ((uint32_t)args[1]<<8)+
				       ((uint32_t)args[2]<<16)+
				       ((uint32_t)args[3]<<24)
				       );
		default:
			return -1.0;
	}
}*/
int isprime(unsigned long n){
	if(n==2)return 1;
	if(!(n&1))return 0;
	unsigned long end=(unsigned long)(sqrt(n)+1.0);
	for(unsigned long i=3;i<end;i+=2)
		if(!(n%i))return 0;
	return 1;
}
double cal_prime(double x,unsigned long (*f)(unsigned long)){
	double fx,xmfx;
	if(x<=0.0)return 1.0;
	fx=floor(x);
	xmfx=x-fx;
	if(xmfx<=DBL_EPSILON)
		return (double)f((unsigned long)(x));
	return (1.0-xmfx)*(double)f((unsigned long)(fx))
		+xmfx*(double)f(1ul+(unsigned long)fx);
}
double d_prime(double x){
	return cal_prime(x,prime);
}
double d_prime_mt(double x){
	return cal_prime(x,prime_mt);
}
double d_prime_old(double x){
	return cal_prime(x,prime_old);
}
double d_isprime(double x){
	return (double)isprime((unsigned long)(fabs(x)));
}
double d_print(double x){
	return (double)fprintd(STDOUT_FILENO,x);
}

double d_puts(double x){
	/*union {
		double d;
		char *r;
	} un;
	un.d=x;*/
	return (double)puts(expr_cast(x,const char *));
}
double d_fprint(double *args,size_t n){
	return (double)fprintd((int)args[0],args[1]);
}
double d_printa(double *args,size_t n){
	return (double)fprintda(STDOUT_FILENO,args,n);
}
double d_sorta_old(double *args,size_t n){
	expr_sort_old(args,n);
	return (double)fprintda(STDOUT_FILENO,args,n);
}
double d_sorta(double *args,size_t n){
	if(expr_sort4(args,n,malloc,free)<0)
		return NAN;
	return (double)fprintda(STDOUT_FILENO,args,n);
}
double d_frya(double *args,size_t n){
	expr_fry(args,n);
	return (double)fprintda(STDOUT_FILENO,args,n);
}
double d_fprinta(double *args,size_t n){
	return (double)fprintda((int)args[0],args+1,n-1);
}
double geterrno(void){
	return cast(&errno,double);
}
volatile double vx[8];
void add_common_symbols(struct expr_symset *es){
	char buf[32];
	for(size_t i=0;i<(sizeof(vx)/sizeof(*vx));++i){
		sprintf(buf,"x%zu",i);
		expr_symset_add(es,buf,EXPR_VARIABLE,0,vx+i);
	}
	//puts("vx ok");
	expr_symset_add(es,"errno",EXPR_VARIABLE,0,&errno);
	expr_symset_add(es,"sig",EXPR_VARIABLE,0,&last_sig);
	expr_symset_add(es,"sigep",EXPR_VARIABLE,0,&sigep);
#define setza(c) expr_symset_add(es,#c,EXPR_ZAFUNCTION,0,d_##c)
#define setzau(c) expr_symset_add(es,#c,EXPR_ZAFUNCTION,EXPR_SF_UNSAFE,d_##c)
	expr_symset_add(es,"time",EXPR_ZAFUNCTION,0,dtime);
	expr_symset_add(es,"geterrno",EXPR_ZAFUNCTION,0,geterrno);
	setza(getpid);
	setza(getppid);
	setza(gettid);
	setza(getuid);
	setza(geteuid);
	setza(getgid);
	setza(getegid);
	setzau(fork);
	setzau(vfork);
#define setfunc(c) expr_symset_add(es,#c,EXPR_FUNCTION,0,d_##c,EXPR_SF_UNSAFE)
#define setfunci(c) expr_symset_add(es,#c,EXPR_FUNCTION,0,d_##c,EXPR_SF_INJECTION)
	setfunc(alarm);
	setfunc(close);
	setfunci(htonl);
	setfunci(htons);
	setfunci(isprime);
	setfunci(prime);
	setfunci(prime_mt);
	setfunci(prime_old);
	setfunc(print);
	setfunc(sigep);
	setfunc(puts);
	setfunc(raise);
	setfunc(setsig);
	setfunc(setuid);
	setfunc(seteuid);
	setfunc(setgid);
	setfunc(setegid);
	setfunc(sleep);
	setfunc(strerror);
	setfunc(strlen);
	setfunc(wait);
#define setmd(c,dim) expr_symset_add(es,#c,EXPR_MDFUNCTION,EXPR_SF_UNSAFE,d_##c,(size_t)dim)
	setmd(accept,3);
	setmd(bind,3);
	setmd(connect,0);
	setmd(fprint,2);
	setmd(fprinta,0);
	setmd(frya,0);
	setmd(inet_addr,0);
	setmd(kill,2);
	setmd(listen,2);
	setmd(printa,0);
	setmd(printf,0);
	setmd(printk,0);
	setmd(read,3);
	setmd(signal,2);
	setmd(signalep,2);
	setmd(sorta,0);
	setmd(sorta_old,0);
	setmd(socket,3);
	setmd(strchr,2);
	setmd(tgkill,3);
	setmd(write,3);
	setmd(open,0);
#define setconst(c) expr_symset_add(es,#c,EXPR_CONSTANT,0,(double)(c))
	setconst(AF_UNIX);
	setconst(AF_INET);
	setconst(AF_INET6);
	setconst(AF_PACKET);
	setconst(PF_UNIX);
	setconst(PF_INET);
	setconst(PF_INET6);
	setconst(PF_PACKET);
	setconst(SOCK_DGRAM);
	setconst(SOCK_RAW);
	setconst(SOCK_RDM);
	setconst(SOCK_SEQPACKET);
	setconst(SOCK_STREAM);
	setconst(IPPROTO_ICMP);
	setconst(IPPROTO_IP);
	setconst(IPPROTO_IGMP);
	setconst(IPPROTO_TCP);
	setconst(IPPROTO_UDP);
	setconst(O_APPEND);
	setconst(O_ASYNC);
	setconst(O_CLOEXEC);
	setconst(O_CREAT);
	//setconst(O_DIRECT);
	setconst(O_DIRECTORY);
	setconst(O_DSYNC);
	setconst(O_EXCL);
	//setconst(O_LARGEFILE);
	//setconst(O_NOATIME);
	setconst(O_NOCTTY);
	setconst(O_NDELAY);
	setconst(O_NOFOLLOW);
	setconst(O_NONBLOCK);
	//setconst(O_PATH);
	setconst(O_RDONLY);
	setconst(O_RDWR);
	setconst(O_SYNC);
	//setconst(O_TMPFILE);
	setconst(O_TRUNC);
	setconst(O_WRONLY);
	expr_symset_add(es,"SIG_DFL",EXPR_CONSTANT,expr_cast(SIG_DFL,double));
	expr_symset_add(es,"SIG_ERR",EXPR_CONSTANT,expr_cast(SIG_ERR,double));
	expr_symset_add(es,"SIG_IGN",EXPR_CONSTANT,expr_cast(SIG_IGN,double));
	setconst(EXIT_FAILURE);
	setconst(EXIT_SUCCESS);
	setconst(SIGHUP);
	setconst(SIGINT);
	setconst(SIGQUIT);
	setconst(SIGILL);
	setconst(SIGTRAP);
	setconst(SIGABRT);
	setconst(SIGBUS);
	setconst(SIGFPE);
	setconst(SIGKILL);
	setconst(SIGUSR1);
	setconst(SIGSEGV);
	setconst(SIGUSR2);
	setconst(SIGPIPE);
	setconst(SIGALRM);
	setconst(SIGTERM);
	setconst(SIGSTKFLT);
	setconst(SIGCHLD);
	setconst(SIGCONT);
	setconst(SIGSTOP);
	setconst(SIGTSTP);
	setconst(SIGTTIN);
	setconst(SIGTTOU);
	setconst(SIGURG);
	setconst(SIGXCPU);
	setconst(SIGXFSZ);
	setconst(SIGVTALRM);
	setconst(SIGPROF);
	setconst(SIGWINCH);
	setconst(SIGIO);
	setconst(SIGPWR);
	setconst(SIGSYS);
	setconst(SIGRTMIN);
	setconst(SIGRTMAX);
	setconst(STDIN_FILENO);
	setconst(STDOUT_FILENO);
	setconst(STDERR_FILENO);
	setconst(AT_FDCWD);
	setconst(GRND_NONBLOCK);
	setconst(GRND_RANDOM);
	setconst(GRND_INSECURE);
	setconst(INADDR_NONE);
	expr_symset_add(es,"pid",EXPR_CONSTANT,(double)getpid());
	expr_symset_add(es,"ppid",EXPR_CONSTANT,(double)getppid());
	expr_symset_add(es,"uid",EXPR_CONSTANT,(double)getuid());
	expr_symset_add(es,"gid",EXPR_CONSTANT,(double)getgid());
#define register_syscall(sysid) expr_symset_add(es,"sys_" #sysid,EXPR_CONSTANT,0,(double)(__NR_##sysid))
#include "systable.c"
}
