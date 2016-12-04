/* ts=4 */
/*
** Copyright 2011 Carnegie Mellon University
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**    http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#ifdef __APPLE__
#include <libgen.h>
#endif
#include "Xsocket.h"
#include "Xkeys.h"
#include "dagaddr.hpp"

#define VERSION "v1.0"
#define TITLE "TCP FTP Server"

#define MAX_XID_SIZE 100
#define STREAM_NAME "www_s.stream_ftp.aaa.xia"
#define DGRAM_NAME "www_s.dgram_ftp.aaa.xia"
#define SID_DGRAM   "SID:0f00000000000000000000000000000000008888"

// if no data is received from the client for this number of seconds, close the socket
#define WAIT_FOR_DATA	10
int verbose=1;

#define MAXBUF (XIA_MAXBUF)
char cmd[MAXBUF];
char reply[MAXBUF];

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



void recvCmd(int sock){
	int n, count=0;
	char* fname;
	while(1){
		say("waiting for command\n");
		memset(cmd, '\0', strlen(cmd));
		memset(reply, '\0', strlen(reply));
		while ((n = Xrecv(sock, cmd, MAXBUF, 0))<=0){
			if (n==0) continue;
			warn("socket error while waiting for data\n");
			break;
		}
		say("receive cmd: %s %d\n",cmd,n);
		if (strncmp(cmd, "get", 3)==0){
			fname = &cmd[4];
			say("Client requested file %s\n",fname);
			//Xsend(sock,reply,strlen(reply),0)
			FILE *fd;
			if ((fd=fopen(fname, "r"))==NULL){
				say("File %s NOT EXIST\n",fname);
				break;
			}			
			while((n=fread(reply,1,MAXBUF,fd))>0){
				say("read count: %d\n",n);
				n=Xsend(sock,reply,n,0);
				say("send count: %d\n",n);
			}				
			fclose(fd);
		}
		else if (strncmp(cmd, "done",4)==0){
			say("done sending file\n");
			break;
		}
		break;
	}
	Xclose(sock);
}

void ftp_stream()
{
	int acceptor, sock;
	char sid_string[strlen("SID:") + XIA_SHA_DIGEST_STR_LEN];

	say("FTP ftp service started\n");

	if ((acceptor = Xsocket(AF_XIA, SOCK_STREAM, 0)) < 0)
		die(-2, "unable to create the stream socket\n");

	// Generate an SID to use
	if(XmakeNewSID(sid_string, sizeof(sid_string))) {
		die(-1, "Unable to create a temporary SID");
	}

	struct addrinfo hints, *ai;
	bzero(&hints, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_XIA;
	if (Xgetaddrinfo(NULL, sid_string, &hints, &ai) != 0)
		die(-1, "getaddrinfo failure!\n");

	Graph g((sockaddr_x*)ai->ai_addr);

	sockaddr_x *sa = (sockaddr_x*)ai->ai_addr;

	printf("\nStream DAG\n%s\n", g.dag_string().c_str());

    if (XregisterName(STREAM_NAME, sa) < 0 )
    	die(-1, "error registering name: %s\n", STREAM_NAME);
	say("registered name: \n%s\n", STREAM_NAME);

	if (Xbind(acceptor, (struct sockaddr *)sa, sizeof(sockaddr_x)) < 0) {
		die(-3, "unable to bind to the dag\n");
	}

	Xlisten(acceptor, 5);

	while (1) {

		say("Xsock %4d waiting for a new connection.\n", acceptor);
		sockaddr_x sa;
		socklen_t sz = sizeof(sa);

		if ((sock = Xaccept(acceptor, (sockaddr*)&sa, &sz)) < 0) {
			warn("Xsock %d accept failure! error = %d\n", acceptor, errno);
			// FIXME: should we die here instead or try and recover?
			continue;
		}

		Graph g(&sa);
		say ("Xsock %4d new session\n", sock);
		say("peer:%s\n", g.dag_string().c_str()); 


		Xclose(acceptor);

		recvCmd(sock);
		exit(0);

	}

}


int main(int argc, char *argv[])
{

	say ("\n%s (%s): started\n", TITLE, VERSION);


	ftp_stream();

	return 0;
}
