// Based off Beej's socket server

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "../common/Common.h"
#include "../common/Utils.h"

#define PORT "12345"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int shutdown = 0;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int children[5];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
            //printf("p->ai_family: %d, p->ai_socktype: %d, p->ai_protocol: %d",p->ai_family, p->ai_socktype, p->ai_protocol );
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(!shutdown) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			char receiveBuffer[4*ARG_MAX];
            char* responseBuffer;
			int result = 0;
            int success = -1;
			char shouldListen = 1;
            struct CommandMessage__Request_Wire* request   = NULL;
            struct CommandMessage__Response_Wire* response = NULL;

			while(shouldListen && !shutdown) {
			    memset(&receiveBuffer[0], 0, 4*ARG_MAX);
			    success = -1;

                //LISTEN ON SOCKET
                //Transform receive buffer into a pointer by referencing the first byte and grabbing the address
                result = getTLVMessage(new_fd, &receiveBuffer[0], sizeof(receiveBuffer));
                printf("Recieved %d Bytes of Data\n", result);

                //malloc'd
                request = bufferToRequest(receiveBuffer);

                printf("CommandMessage: { command: %s, length: %d, data: %s} \n",
                       commandStrings[request->__command],
                       request->length,
                       request->data);

                response =
                        (struct CommandMessage__Response_Wire *) malloc(sizeof(struct CommandMessage__Response_Wire));
                switch (request->__command) {
                    case get:;
                        if (checkIfFileExists(request->data)) {
                            //malloc
                            char* fileContents = readInFile(request->data, &success);
                            printf("Result From File was: %d", success);
                            if( success == -1) {
                                printf("Failed to read file");
                                response->__status = ERROR;
                                response->data = wrapRawStingInMalloc("An error occurred while reading specified file\n");
                                response->length = strlen(response->data);
                            } else {
                                printf("Read File\n");
                                response->__status = OK;
                                response->data = fileContents;
                                response->length = success; //read in file sets success to the length of the file on success
                            }
                            fileContents = NULL;
                        }
                        break;
                    case ls:;

                        //malloc
                        char *dirContents = getDirContents(request->data, &success);
                        printf("Result was %d\n", success);
                        if (success != 0) {
                            printf("Failed to get dir Contents\n");
                            response->__status = ERROR;
                            response->data = wrapRawStingInMalloc("An error occurred getting directory contents\n");
                            response->length = strlen(response->data);
                        } else {
                            printf("Got Dir Contents\n");
                            response->__status = OK;
                            printf("Result was: \n %s", dirContents);
                            response->data = dirContents;
                            printf("strlen(dirContents) = %d\n", strlen(dirContents));
                            response->length = strlen(dirContents);
                        }
                        dirContents = NULL;
                        break;
                    case pwd:;
                        printf("Getting CWD");
                        response->__status = OK;
                        response->data =wrapRawStingInMalloc(getenv("PWD"));
                        response->length = strlen(response->data);
                        break;
                    case cd:;
                        success = changeDirectory(request->data);

                        if (success != 0) {
                            printf("Failed to change Directory\n");
                            response->__status = ERROR;
                            response->data = wrapRawStingInMalloc("An Error occurred while changing directory\n");
                            response->length = strlen(response->data);
                        } else {
                            printf("changed directory");
                            response->__status = OK;
                            response->data = wrapRawStingInMalloc(getenv("PWD"));
                            response->length = strlen(response->data);
                        }
                        break;
                    case bye:;
                        shouldListen = 0;
                        response->__status = OK;
                        response->data = wrapRawStingInMalloc(" \0");
                        response->length = 1;
                        break;
                    case sexit:;
                        printf("Exit called");
                        shutdown = 1;
                        break;
                    default:
                        printf("Unknown\n");
                        response->__status = ERROR;
                        response->data = wrapRawStingInMalloc("Unknown Command\n");
                        response->length = strlen(response->data);
                        break;
                }
                //malloc'd
                responseBuffer = responseToBuffer(response);
                printf("Now Sending Data\n");
                sendAll(new_fd, responseBuffer, response->length + 5);

                free(responseBuffer);
                printf("Freed Response Buffer\n");
                freeRequest(request);
                printf("Freed Request\n");
                freeResponse(response);
                printf("freed Response\n");

            }
            printf("closing socket\n");
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}
    printf("Shutdown\n");
	return 0;
}

