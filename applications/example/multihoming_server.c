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

#define MAX_XID_SIZE 100
#define MUL_SERVER1 "www_s.multihong_server1.aaa.xia"
#define MUL_SERVER2 "www_s.multihong_server2.aaa.xia"

/*
char sid_string[strlen("SID:") + XIA_SHA_DIGEST_STR_LEN];
	// Generate an SID to use
	if (XmakeNewSID(sid_string, sizeof(sid_string))) {
		die(-1, "Unable to create a temporary SID");
	}
*/
int verbose = 1;	// display all messages
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
	fprintf(stdout, "exiting\n");
	exit(ecode);
}

void *server(void * id){
	int sock;
	char buf[XIA_MAXBUF];
	sockaddr_x cdag;
	socklen_t dlen;
	int n;
	int ser_id = *((int*)id);
	char sid_string[strlen("SID:") + XIA_SHA_DIGEST_STR_LEN];
	// Generate an SID to use
	if (XmakeNewSID(sid_string, sizeof(sid_string))) {
		die(-1, "Unable to create a temporary SID");
	}
	
	say("%d Datagram service started\n", ser_id);

	if ((sock = Xsocket(AF_XIA, SOCK_DGRAM, 0)) < 0)
		die(-2, "unable to create the datagram socket\n");

	struct addrinfo *ai;
	if (Xgetaddrinfo(NULL, sid_string, NULL, &ai) != 0)
		die(-1, "getaddrinfo failure!\n");

	sockaddr_x *sa = (sockaddr_x*)ai->ai_addr;

	Graph g((sockaddr_x*)ai->ai_addr);
	printf("\n%d Datagram DAG\n%s\n", ser_id, g.dag_string().c_str());
	
    
	if(ser_id == 1){
		if (XregisterName(MUL_SERVER1, sa) < 0 )
			die(-1, "error registering name: %s\n", MUL_SERVER1);
		say("registered name: \n%s\n", MUL_SERVER1);
	}
	else{
		if (XregisterName(MUL_SERVER2, sa) < 0 )
			die(-1, "error registering name: %s\n", MUL_SERVER2);
		say("registered name: \n%s\n", MUL_SERVER2);
	}
	if (Xbind(sock, (sockaddr *)sa, sizeof(sa)) < 0) {
		die(-3, "unable to bind to the dag\n");
	}

	pid_t pid = 0;

	while (1) {
		say("Dgram Server %d waiting\n", ser_id);

		dlen = sizeof(cdag);
		memset(buf, 0, sizeof(buf));
		if ((n = Xrecvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&cdag, &dlen)) < 0) {
			warn("Recv error on socket %d, closing connection\n", pid);
			break;
		}

		say("server %d received %d bytes\n", ser_id, n);
		
		if ((n = Xsendto(sock, buf, n, 0, (struct sockaddr *)&cdag, dlen)) < 0) {
			warn("%5d send error\n", pid);
			break;
		}

		say("server %d sent %d bytes\n", ser_id, n);
	}

	Xclose(sock);
}
int main()
{
	pthread_t *clients = (pthread_t*)malloc(2 * sizeof(pthread_t));

	if (!clients)
		die(-5, "Unable to allocate threads\n");
	int id1 = 1;//(int*)malloc(sizeof(int));
	int id2 = 2;//(int*)malloc(sizeof(int));
	//*id1 = 1;
	//*id2 = 2;
	pthread_create(&clients[0], NULL, server, (void *)&id1);
	pthread_create(&clients[1], NULL, server, (void *)&id2);
	
	for (int i = 0; i < 2; i++) {
		pthread_join(clients[i], NULL);
	}

	free(clients);
	
	return 0;
}
