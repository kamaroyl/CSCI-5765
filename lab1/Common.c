#include "Common.h"
char* commandStrings[6] = {
    "get",
	"ls",
	"pwd",
	"cd",
	"bye",
	"exit"
};

char* statusStrings[2] = {
    "OK",
    "ERROR"
};

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
    buffer[5 + request->length] = '\0';
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
    buffer[5 + response->length] = '\0';
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
        delta = recv(socketFd, &buffer[index], size - delta, 0);
        if( delta == -1 ) {
            printf("Failed to recv in loop\n");
            return -1;
        }
        index += delta;
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
