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
#include <sys/time.h>

#define VERSION "v1.0"
#define TITLE "TCP FTP Client"

#define STREAM_NAME "www_s.stream_ftp.aaa.xia"

#define MAX_BUF_SIZE 62000

#define MAXBUF (XIA_MAXBUF)
char fin[256];
char fout[256];
char cmd[MAXBUF];

int verbose=1;
int delay=1;

struct addrinfo *ai;
sockaddr_x *sa;

/*
** write the message to stdout unless in quiet mode
*/
void say(const char *fmt, ...)
{
	if (verbose) {
		va_list args;

		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
}

/*
** always write the message to stdout
*/
void warn(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);

}

/*
** write the message to stdout, and exit the app
*/
void die(int ecode, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	va_end(args);
	fprintf(stdout, "%s: exiting\n", TITLE);
	exit(ecode);
}


/*
** do a short pause between operations
*/
void pausex()
{
	int t;
	if (delay == -1)
		// default - don't pause at all
		return;
	else if (delay == 0)
		// pause for some random period less than a second
		t = rand() % 1000000;
	else
		// pause for the specfied number of hundredths of a second
		t = delay;
	usleep(t);
}

/*
** create a socket and connect to the remote server
*/
int connectToServer()
{
	int ssock;
	if ((ssock = Xsocket(AF_XIA, XSOCK_STREAM, 0)) < 0) {
		die(-2, "unable to create the server socket\n");
	}
	say("Xsock %4d created\n", ssock);

	if (Xconnect(ssock, (struct sockaddr *)sa, sizeof(sockaddr_x)) < 0)
		die(-3, "unable to connect to the destination dag\n");

	say("Xsock %4d connected\n", ssock);

	return ssock;
}


void getFile()
{
	int total_time=0;
	int total_size=0;
	int total_write_time=0;
	struct timeval tv1,tv2,tv3,tv4;
	struct timezone tz1,tz2;

	FILE *fd=fopen(fout,"w");
	int ssock;
	int count = 0;
	
	say("getting file %s\n",fin);

	ssock = connectToServer();
	//start
	sprintf(cmd, "get %s\0",fin);
	say("send cmd %s %d\n",cmd,strlen(cmd));


	int n = Xsend(ssock, cmd, strlen(cmd), 0);
	if (n < 0)
		die(-4, "Send error on socket %d\n", ssock);
	gettimeofday(&tv3,NULL);
	
	while (1){
		gettimeofday(&tv1,&tz1);
		n = Xrecv(ssock, cmd, MAXBUF,0);
		if (n<=0) break;		
		gettimeofday(&tv2,&tz2);
		say("Xrecv block time: %d\n",(tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec));		
		
		
		total_size+=n;
		total_time+=(tv2.tv_sec-tv1.tv_sec)*1000*1000+(tv2.tv_usec-tv1.tv_usec);	

		
		gettimeofday(&tv1,&tz1);		
		fwrite(cmd,1,n,fd);
		gettimeofday(&tv2,&tz2);
		say("Fwrite block time: %d\n",(tv2.tv_sec-tv1.tv_sec)*1000000+(tv2.tv_usec-tv1.tv_usec));
		//say("write %d bytes into %s\n",n,fout);
		total_write_time+=(tv2.tv_sec-tv1.tv_sec)*1000*1000+(tv2.tv_usec-tv1.tv_usec);
	}
	
	gettimeofday(&tv4,NULL);
	say("CONCLUSION: transmit file(%d bytes)\n",total_size);
	say("CONCLUSION: total Xrecv block time is %d us\n",total_time);
	say("CONCLUSION: total write time is %d us\n",total_write_time);
	say("CONCLUSION: transmit time with writing file is %d %d ms\n",(total_time+total_write_time)/1000,(tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000);
	fclose(fd);

	//end
	sprintf(cmd, "done\0");
	say("send cmd %s %d\n",cmd,strlen(cmd));
	Xsend(ssock,cmd,strlen(cmd),0);
	Xclose(ssock);
	say("Xsock %4d closed\n", ssock);

}


/*
** where it all happens
*/
int main(int argc, char **argv)
{
	srand(time(NULL));
	
	if (argc==2){
		say ("\n%s (%s): started\n", TITLE, VERSION);
		strcpy(fin,argv[1]);
		sprintf(fout, "my%s", fin);
	}
	else{
		warn("please input the filename\n");
		exit(0);
	}

	if (Xgetaddrinfo(STREAM_NAME, NULL, NULL, &ai) != 0)
		die(-1, "unable to lookup name %s\n", STREAM_NAME);
	sa = (sockaddr_x*)ai->ai_addr;

	Graph g(sa);
	printf("\n%s\n", g.dag_string().c_str());

	getFile();

	return 0;
}
