/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../common/Common.h"

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 



int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);
	struct CommandMessage__Request_Wire msg;
	msg.__command = cd;
	msg.data = "asdf";
	msg.length = strlen(msg.data);
	char* buffer = requestToBuffer(&msg);
	freeaddrinfo(servinfo); // all done with this structure
	int delta = 0;
	int index = 0;
	while( (delta = send(sockfd, &buffer[index], msg.length + 6 - delta,0)) > 0 ) {
	    printf("Sent %d bytes\n", delta);
	    index += delta;
	}
	printf("Finished Sending Data\n");
	char receiveBuffer[4* ARG_MAX];
	index = 0;
	delta = 0;
	printf("Listening for Data\n");
	int result = getTLVMessage(sockfd, &receiveBuffer[0], 4*ARG_MAX);
	printf("Total bytes Received: %d\n", result);
	printf("Done Receiving data; recieved %d bytes of data, Result was: %d\n", index, delta);
	struct CommandMessage__Response_Wire* response = bufferToResponse(receiveBuffer);
	printf("ResponseMessage: { Status: %s, length: %d, data: %s} \n", statusStrings[response->__status], response->length, response->data);
	close(sockfd);

	return 0;
}

