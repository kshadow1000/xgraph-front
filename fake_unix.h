
/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#ifndef _FAKE_UNIX_H_
#define _FAKE_UNIX_H_

//stdio.h and errno.h should be included.

#ifdef O_RDONLY
#undef O_RDONLY
#endif
#ifdef O_WRONLY
#undef O_WRONLY
#endif
#define O_RDONLY "rb"
#define O_WRONLY "wb"
static intptr_t fake_open(const char *path,const char *flags){
	FILE *fp;
	errno=0;
	fp=fopen(path,flags);
	if(!fp)
		return -errno;
	return (intptr_t)fp;
}
#define open fake_open
static int fake_close(intptr_t fd){
	errno=0;
	fclose((FILE *)fd);
	if(errno)
		return -errno;
	return 0;
}
#define close fake_close
static ssize_t fake_read(intptr_t fd,void *buf,size_t size){
	ssize_t r;
	errno=0;
	r=fread(buf,1,size,(FILE *)fd);
	if(errno)
		return -errno;
	return r;
}
#define read fake_read
static ssize_t fake_write(intptr_t fd,const void *buf,size_t size){
	ssize_t r;
	errno=0;
	r=fwrite(buf,1,size,(FILE *)fd);
	if(errno)
		return -errno;
	fflush((FILE *)fd);
	return r;
}
#define write fake_write
#endif
