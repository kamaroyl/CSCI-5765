/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../common/Common.h"
#include "../common/Utils.h"

#define PORT "12345" // the port client will be connecting to

extern char **environ;

int main(int argc, char *argv[])
{
    if(getpid() > 0) {
        char input[ARG_MAX]; // User input buffer
        int sockfd, numbytes;
        struct addrinfo hints, *servinfo, *p;
        int rv;
        char s[INET6_ADDRSTRLEN];
        char receiveBuffer[ARG_MAX];
        struct CommandMessage__Request_Wire *msg;
        struct CommandMessage__Response_Wire *response;
        char quit = 0;
        enum command commandToSend;
        char **tokenizedCommand; //FREE ME
        int tokenizedInputLength;

        if (argc != 2) {
            fprintf(stderr, "usage: client hostname\n");
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
        for (p = servinfo; p != NULL; p = p->ai_next) {

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

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr), s, sizeof s);
        printf("client: connecting to %s\n", s);

        freeaddrinfo(servinfo); // all done with this structure

        //User loop
        while (!quit) {
            int result;
            char *buffer;
            //print Tag
            printPromptTag();

            // wait for user input
            getUserInput(&input[0]);

            //split user input
            tokenizedCommand = s2a(&input[0], &tokenizedInputLength);

            //sanitize user input
            if (tokenizedInputLength > 2) {
                printf("Too many space delimited strings, only 2 allowed.\n");
                printf("If this is due to your use of spaces in your directory path, try not doing that.\n");
                goto FREE;
            }
            commandToSend = stringToCommand(tokenizedCommand[0]);

            if (commandToSend == UNKNOWN) {
                printf("Command was not known, please check your format\n");
                goto FREE;
            }

            if (commandToSend == ls ||
                commandToSend == lls ||
                commandToSend == pwd ||
                commandToSend == lpwd ||
                commandToSend == help ||
                commandToSend == bye) {
                if (tokenizedInputLength > 1) {
                    printf("Command does not support argument\n");
                    goto FREE;
                }
            } else if (commandToSend == get ||
                       commandToSend == cd ||
                       commandToSend == lcd) {
                if (tokenizedInputLength != 2) {
                    printf("Command requires second argument");
                } else {
                    cleanInput(tokenizedCommand[1]);
                }
            }

            //translate user input
            if (commandToSend == bye) quit = 1;
            if (commandToSend > 4) { // command is local
                switch (commandToSend) {
                    case lls:;
                        int processStatus = 0;
                        int pid = fork();
                        if(pid < 0) {
                            printf("failed to create process\n");
                        } else if(pid == 0) {
                            char* directory[] = {"."};
                            execve("ls", directory, environ);
                        } else {
                            wait(&processStatus);
                            //printf("Child Returned\n");
                            if(processStatus != 0) printf("Child process failed\nStatusCode: %d\n", processStatus);
                        }
                        break;
                    case lpwd:;
                        printf("Current Working Directory: %s", getenv("PWD"));
                    case lcd:;
                        result = changeDirectory(tokenizedCommand[1]);
                        if (result != 0) {
                            printf("Failed to change directory %s\n", tokenizedCommand[1]);
                        } else {
                            printf("changed directory %s", getenv("PWD"));
                        }
                        break;
                    case help:;
                        printf("Help: \n");
                        printf("    ls   - list files in server's current working directory\n");
                        printf("    pwd  - get the server's current working directory\n");
                        printf("    cd   - change directory on server, requires a path argument\n");
                        printf("    get  - retrieve file from server in the servers current working directory\n");
                        printf("    bye  - close connection to remote server and exit\n");
                        printf("    lls  - list files in the local current working directory\n");
                        printf("    lpwd - get the local current working directory where client is running\n");
                        printf("    lcd  - change directory where client is running\n");
                        break;
                    default:
                        printf("No Such Command\n");
                }

            } else { // command is remote
                switch(commandToSend) {
                    case ls:;
                        //malloc
                        msg = translateUserInput(ls, ".");
                        break;
                    case pwd:;
                        //malloc
                        msg = translateUserInput(pwd, "");
                        break;
                    case cd:;
                        //malloc
                        msg = translateUserInput(cd, tokenizedCommand[1]);
                        break;
                    case get:;
                        //malloc
                        msg = translateUserInput(get, tokenizedCommand[1]);
                        break;
                    case bye:;
                        //malloc
                        msg = translateUserInput(bye, "");
                        break;
                    default:
                        printf("No Such Command\n");
                        goto FREE;
                }

                //malloc
                buffer = requestToBuffer(msg);

                //send or execute command
                sendAll(sockfd, &buffer[0], msg->length + 6);

                //recieve response
                result = getTLVMessage(sockfd, &receiveBuffer[0], ARG_MAX);
                printf("DEBUG Done Receiving data; recieved %d bytes of data\n", result);
                response = bufferToResponse(receiveBuffer);
                if(msg->__command == get) {
                    result = writeOutFile(response->data, response->length,  tokenizedCommand[1]);
                    if(result < 0) {
                        printf("Failed to write file \n");
                    }
                }else {
                    printf("DEBUG ResponseMessage: { Status: %s, length: %d, data: %s} \n", statusStrings[response->__status],
                           response->length, response->data);
                }

                //error handling and results
                free(msg);
                free(buffer);
            } //Remote vs Local End

            FREE:
                //clear user input;
                memset(&input, 0, sizeof(input));
                //Free all the things!
                freeTokenizedInput(tokenizedInputLength, tokenizedCommand);
        } //user loop END

        close(sockfd);
        printf("Goodbye\n");
    }
    return 0;
}
