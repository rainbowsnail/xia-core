/* ts=4 */
/*
** Copyright 2011 Carnegie Mellon University
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
** http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#ifdef __APPLE__
#include <libgen.h>
#endif
#include "Xsocket.h"
#include "dagaddr.hpp"

#define VERSION "v1.0"
#define TITLE "TCP FTP Client"

#define STREAM_NAME "www_s.stream_ftp.aaa.xia"

#define MAX_BUF_SIZE 62000



char *randomString(char *buf, int size)
{
	int i;
	static const char *filler = "!@#$%^&*()_+-=0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static int refresh = 1;
	int samples = strlen(filler);

	if (!(--refresh)) {
		// refresh rand every now and then so it doesn't degenerate too much
		// use a prime number to keep it interesting
		srand(time(NULL));
		refresh = 997;
	}
	for (i = 0; i < size; i ++) {
		buf[i] = filler[rand() % samples];
	}

	return buf;
}

void file_create(char *filename,int filesize)
{
	const int buf_size=10*1000;	

	FILE *fd=fopen(filename,"w");
	
	char buf[buf_size];

	for (int i=0;i<filesize/buf_size;i++){

		randomString(buf,buf_size);

		fwrite(buf,1,buf_size,fd);

	}

	fclose(fd);
}

char read_buf[XIA_MAXBUF];
void file_cut(char *filename,int filesize)
{
	int n;
	char newname[1000];
	sprintf(newname,"%s0",filename);	
	printf("%s\n",newname);


	FILE *fd=fopen(filename,"r");	
	FILE *newfd=fopen(newname,"w");
	
	int num=filesize/XIA_MAXBUF;
	int left=filesize-num*XIA_MAXBUF;
	for (int i=0;i<num;i++){
		n=fread(read_buf,1,XIA_MAXBUF,fd);
		fwrite(read_buf,1,n,newfd);
	}
	n=fread(read_buf,1,left,fd);
	fwrite(read_buf,1,n,newfd);
	
	
	fclose(fd);
	fclose(newfd); 
}
/* 
** where it all happens
*/
int main(int argc, char **argv)
{
	srand(time(NULL));
	
	file_create("testfile/test50k.txt",50*1000);	

	//file_cut("testfile/test100k.txt",61400);

	return 0;
}
