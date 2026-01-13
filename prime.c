#include <err.h>
#include <pthread.h>
unsigned long prime(unsigned long x){
	static unsigned long *p=NULL;
	static size_t n=0;
	static size_t size=0;
	static int im65=0;
	unsigned long *lp;
	unsigned long endn;
	size_t sizem1;
	if(!n){
		lp=malloc((size=5ul)*sizeof(unsigned long));
		if(!lp)return 0ul;
		p=lp;
		p[0]=1;
		p[1]=2;
		p[2]=3;
		p[3]=5;
		p[4]=7;
		n=4;
	}
	if(x<=n)return p[x];
	if(x+1>=size){
		sizem1=(x+1025ul)&~1023ul;
		lp=realloc(p,sizem1*sizeof(unsigned long));
		if(!lp)return 0ul;
		p=lp;
		size=sizem1;
	}
	sizem1=size-1;
	for(unsigned long i=p[n]+(im65?2:4);n<sizem1;){
		endn=(unsigned long)(sqrt((double)i)+1.0);
		for(unsigned long i1=1;p[i1]<=endn&&i1<=n;++i1){
			if(!(i%p[i1]))goto fail;
		}
		p[++n]=i;
fail:
		if(im65^=1)i+=2;
		else i+=4;
	}
	return p[x];
}
unsigned long prime_mt(unsigned long x){
	static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
	unsigned long r;
	pthread_mutex_lock(&mutex);
	r=prime(x);
	pthread_mutex_unlock(&mutex);
	return r;
}
unsigned long prime_old(unsigned long x){
	unsigned long im65,im652;
	unsigned long i,endn;
	switch(x){
		case 0ul:return 1;
		case 1ul:return 2;
		case 2ul:return 3;
		case 3ul:return 5;
		case 4ul:return 7;
		default:break;
	}
	im65=0;
	x-=4;
	for(i=11;;){
		endn=(unsigned long)(sqrt((double)i)+1.0);
		if(!(i&1))goto fail;
		if(!(i%3))goto fail;
		im652=0;
		for(unsigned long i1=5;i1<=endn;){
			if(!(i%i1))goto fail;
			if(im652^=1)i1+=2;
			else i1+=4;
		}
		if(!--x)break;
fail:
		if(im65^=1)i+=2;
		else i+=4;
	}
	return i;
}
