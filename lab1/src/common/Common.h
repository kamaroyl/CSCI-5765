#ifndef COMMON_H
#define COMMON_H

#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#define ARG_MAX 4*1024

extern char* commandStrings[6];
extern char* statusStrings[2];

enum __attribute__((__packed__)) command {
    get,
    ls,
    pwd,
    cd,
    bye,
    sexit
};

enum __attribute__((__packed__)) status {
    OK,
    ERROR
};

// wire messages
struct __attribute__((__packed__)) CommandMessage__Request_Wire {
    enum command __command; //1
    unsigned int length; //8
    char* data; //n
};

struct __attribute__((__packed__)) CommandMessage__Response_Wire {
    enum status __status; //1
    unsigned int length;  //8
    char* data; //n
};

void *get_in_addr(struct sockaddr *sa);

void freeRequest(struct CommandMessage__Request_Wire* request);
struct CommandMessage__Request_Wire* bufferToRequest(char* buffer);
char* requestToBuffer(struct CommandMessage__Request_Wire* request);
void freeResponse(struct CommandMessage__Response_Wire* response);
struct CommandMessage__Response_Wire* bufferToResponse(char* buffer);
char* responseToBuffer(struct CommandMessage__Response_Wire* response);
int getLength(char* buffer);
int getTLVMessage(int socketFd, char* buffer, int size);
int sendAll(int socketFd, char* buffer, int size);
char* readInFile(char* fileName, int* result);
int writeOutFile(char* buffer, int length, char* filename);
#endif
