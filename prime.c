uint64_t prime(uint64_t x){
	static uint64_t *p=NULL;
	static size_t n=0;
	static size_t size=0;
	static int im65=0;
	uint64_t *lp;
	uint64_t endn;
	size_t sizem1;
	if(!n){
		lp=malloc((size=5)*sizeof(uint64_t));
		if(!lp)return 0;
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
		sizem1=(x+1025)&~1023;
		lp=realloc(p,sizem1*sizeof(uint64_t));
		if(!lp)return 0;
		p=lp;
		size=sizem1;
	}
	sizem1=size-1;
	for(uint64_t i=p[n]+(im65?2:4);n<sizem1;){
		endn=(uint64_t)(sqrt((double)i)+1.0);
		for(uint64_t i1=1;p[i1]<=endn&&i1<=n;++i1){
			if(!(i%p[i1]))goto fail;
		}
		p[++n]=i;
fail:
		if(im65^=1)i+=2;
		else i+=4;
	}
	return p[x];
}
uint64_t prime_mt(uint64_t x){
	static uint32_t mutex=0;
	uint64_t r;
	expr_mutex_lock(&mutex);
	r=prime(x);
	expr_mutex_unlock(&mutex);
	return r;
}
uint64_t prime_old(uint64_t x){
	uint64_t im65,im652;
	uint64_t i,endn;
	switch(x){
		case 0:return 1;
		case 1:return 2;
		case 2:return 3;
		case 3:return 5;
		case 4:return 7;
		default:break;
	}
	im65=0;
	x-=4;
	for(i=11;;){
		endn=(uint64_t)(sqrt((double)i)+1.0);
		if(!(i&1))goto fail;
		if(!(i%3))goto fail;
		im652=0;
		for(uint64_t i1=5;i1<=endn;){
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
