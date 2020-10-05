#include "Common.h"

const char NUL = '\0';

char* commandStrings[10] = {
    "get",
	"ls",
	"pwd",
	"cd",
	"bye",
	"exit",
	"help",
	"lls",
	"lpwd",
	"lcd"
};

char* statusStrings[2] = {
    "OK",
    "ERROR"
};

//http://www.cse.yorku.ca/~oz/hash.html
unsigned int hash(char *str)
{
    unsigned int hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

enum command stringToCommand(char* userInput) {
    printf("User Input = %s\n", userInput);
    unsigned int hashValue = hash(userInput) % 1000;
    printf("HASH VALUE: %d\n", hashValue);
    switch(hashValue) {
        case 613:
            return get;
        case 588:
            return ls;
        case 992:
            return pwd;
        case 276:
            return cd;
        case 813:
            return bye;
        case 503:
            return sexit;
        case 718:
            return help;
        case 288:
            return lls;
        case 796:
            return lpwd;
        case 976:
            return lcd;
        default:
            return UNKNOWN;
    }

}

void freeRequest(struct CommandMessage__Request_Wire* request) {
    char* data = request->data;
    request->data = NULL;
    free(data);
    free(request);
}

struct CommandMessage__Request_Wire* bufferToRequest(char* buffer) {
    printf("Buffer to Request\n");
    struct CommandMessage__Request_Wire* newRequest =
            (struct CommandMessage__Request_Wire*) malloc(sizeof(struct CommandMessage__Request_Wire));
    newRequest->__command = buffer[0];
    printf("Command was: %s\n", commandStrings[newRequest->__command]); 
    int length = 0;
    for(int i = 1; i <5; ++i) {
        length <<= 8;
        length |= buffer[i];
        printf("Intermediate Length: %d\n", length);
    }
    printf("Length: %d\n");
    printf("Unnetworked Length: %d\n", ntohl(length));
    newRequest->length = ntohl(length);
    printf("newRequest->length = %d\n", newRequest->length);
    char* data = (char*) malloc(newRequest->length * sizeof(char));
    for(int i = 0; i < newRequest->length + 1; ++i) {
        data[i] = buffer[i + 5]; //Offset of enum + int (1 + 4)
//        printf("data[%d] = %d\n", i + 5, data[i]);
    }
    printf("Incoming Data: %s\n", data);
    newRequest->data = data;
    return newRequest;
}

void freeResponse(struct CommandMessage__Response_Wire* response) {
    char* data = response->data;
    response->data = NULL;
    free(data);
    free(response);
}

struct CommandMessage__Response_Wire* bufferToResponse(char* buffer) {
    struct CommandMessage__Response_Wire* newResponse = 
            (struct CommandMessage__Response_Wire*) malloc(sizeof(struct CommandMessage__Response_Wire));
    newResponse->__status = buffer[0];
    printf("Status was: %s\n", statusStrings[newResponse->__status]);
    int length = 0;
    for(int i = 1; i < 5; ++i) {
        length <<= 8;
        length |= buffer[i];
        printf("Intermediate Length: %d\n", length);
    }
    printf("Length: %d\n", length);
    printf("Unnetworked Length: %d\n", ntohl(length));
    newResponse->length = ntohl(length);
    printf("newResponse->length = %d\n", newResponse->length);
    char* data = (char*) malloc(newResponse->length * sizeof(char));
    for( int i = 0; i < newResponse->length; ++i) {
        data[i] = buffer[i + 5];
        printf("data[%d] = %d\n", i + 5, data[i]);
    }
    printf("Incoming data: %s\n", data);
    newResponse->data = data;
    return newResponse; 
}

char* requestToBuffer(struct CommandMessage__Request_Wire* request) {
    char* buffer = (char*) malloc((5 + request->length + 1) * sizeof(char));
    buffer[0] = request->__command;
    printf("Buffer[0] = %d\n", buffer[0]);
    char in = 0;
    int length = htonl(request->length);
    buffer[1] = (length & -16777216) >> (3 * 8);
    printf("Buffer[1] = %d\n", buffer[1]);
    buffer[2] = (length &  16711680) >> (2 * 8);
    printf("Buffer[2] = %d\n", buffer[2]);
    buffer[3] = (length &     65280) >> (1 * 8);
    printf("Buffer[3] = %d\n", buffer[3]);
    buffer[4] = (length &       255) >> (0 * 8);
    printf("Buffer[4] = %d\n", buffer[4]);
    
    for( int i = 0; i < request->length; ++i) {
        buffer[i + 5] = request->data[i];
        printf("Buffer[%d] = %d\n", i + 5, buffer[i + 5]);
    }
    buffer[5 + request->length] = NUL;
    return buffer;
}

char* responseToBuffer(struct CommandMessage__Response_Wire* response) {
    char* buffer = (char*) malloc((5 + response->length + 1) * sizeof(char));
    buffer[0] = response->__status;
    printf("Buffer[0] = %d\n", buffer[0]);
    printf("Response Length: %d\n", response->length);
    int length = htonl(response->length);
    buffer[1] = (length & -16777216) >> (3 * 8);
    printf("Buffer[1] = %d\n", buffer[1]);
    buffer[2] = (length &  16711680) >> (2 * 8);
    printf("Buffer[2] = %d\n", buffer[2]);
    buffer[3] = (length &     65280) >> (1 * 8);
    printf("Buffer[3] = %d\n", buffer[3]);
    buffer[4] = (length &       255) >> (0 * 8);
    printf("Buffer[4] = %d\n", buffer[4]);
    for( int i = 0; i < response->length; ++i) {
        buffer[i + 5] = response->data[i];
        printf("Buffer[%d] = %d\n", i + 5, buffer[i + 5]);
    }
    printf("Copied Data\n");
    buffer[5 + response->length] = NUL;
    return buffer;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int getLength(char* buffer) {
    int length = 0;
     for(int i = 1; i < 5; ++i) {
        length <<= 8;
        length |= buffer[i];
        printf("Buffer[%d] = %d\n", i, buffer[i]);
        printf("Intermediate Length: %d\n", length);
    }

    return htonl(length);
}

int getTLVMessage(int socketFd, char* buffer, int size) {
    int delta = 0;
    int index = 0;
    int length = 0;

    delta = recv(socketFd, buffer, size, 0);
    index += delta;
    if( delta == -1 ) {
        printf("Failed to recv: error code: %s, errno: %d\n", strerror(errno), errno);
        return -1;
    }
    printf("recieved %d bytes\n", delta);
    printf("buffer[0] = %d\n", buffer[0]);
    printf("buffer[1] = %d\n", buffer[1]);
    printf("buffer[2] = %d\n", buffer[2]);
    printf("buffer[3] = %d\n", buffer[3]);
    printf("buffer[4] = %d\n", buffer[4]);
    printf("buffer[5] = %d\n", buffer[5]);
    length = getLength(buffer);
    printf("Length: %d\n", length);
    while( index < length + 5 ) {
        printf("recieving. . . \n");
        delta = recv(socketFd, &buffer[index], size - delta, 0);
        printf("Recieved!\n");
        if( delta == -1 ) {
            printf("Failed to recv in loop\n");
            return -1;
        }
        index += delta;
        printf("Index: %d\n", index);
    }

    printf("Done Recieving TLV Message\n");
    return index;
}

int sendAll(int socketFd, char* buffer, int size) {
    int index = 0;
    int delta = 0;
    while ((delta = send(socketFd, buffer, size - delta, 0)) > 0) {
        printf("sent %d bytes of data\n", delta);
        index += delta;
    }
    if (delta == -1) {
        printf("error Occurred! %s [%d]\n", strerror(errno), errno);
        return -1;
    }
    return index;
}

char* readInFile(char* fileName, int* size) {
    FILE* fp;
    int result = -1;
    const char* read = "r";
    char* buffer;
    *size = -1;
    fp = fopen(fileName, read);
    if( fp == NULL ) {
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    *size = ftell(fp);
    buffer = (char*) malloc((*size) * sizeof(char));
    rewind(fp);
    result = fread(buffer, 1, *size, fp);
    *size = result != *size? -1: *size;
    return buffer;
}

int writeOutFile(char* buffer, int length, char* filename) {
    FILE* fp;
    int success = -1;
    const char* write = "w";
    fp = fopen(filename, write);
    if(fp == NULL) return success;
    success = fwrite(buffer, 1, length, fp);
    return success == length? 0: -1;

}

struct CommandMessage__Request_Wire* translateUserInput(enum command __command, char* argument) {
    struct CommandMessage__Request_Wire* msg =
            (struct CommandMessage__Request_Wire*) malloc(sizeof(struct CommandMessage__Request_Wire));
    printf("In Translate User Input:\n");
    printf("Command is: %s\n",commandStrings[__command] );
    msg->__command = __command;
    printf("Argument is: %s\n", argument);
    msg->data = argument;
    printf("ArgumentLength is: %lld\n", strlen(argument));
    msg->length = strlen(argument);
    return msg;
}

char* wrapRawStingInMalloc(char* rawString) {
    int length = strlen(rawString);
    char* mallocdString = (char*) malloc(length * sizeof(char) + 1);
    for(int i = 0; i < length; ++i) {
        mallocdString[i] = rawString[i];
    }
    mallocdString[length] = NUL;
    return mallocdString;
}
