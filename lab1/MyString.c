#include "MyString.h"

int myStringLength(char* myString) {
    if (myString == NULL) return -1;
    if (myString[0] == NUL) return 0;
    int i = 0;
    while (myString[i] != NUL) {
        i++;
    }
    return i;
}

void zeroOutString(char* myString, int length) {
    int i = 0;
    while(i < length) {
        myString[i] = NUL;
        ++i;
    }
}

void stripWhiteSpace(char* str) {
    int strLen = myStringLength(str)
    if(strLen <= 0) return str;
    char ptr[strLen]
}

/*

 */
char** stringToArray(char delim, char* stringToTokenize, int* length) {
    int numberOfSubstrings = 1;
    int i = 0;
    int j = 0;
    int start = 0;
    char** tokenizedString;

    while(stringToTokenize[i] != NUL) {
        if(stringToTokenize[i] == delim) numberOfSubstrings++;
        i++;
    }

    tokenizedString = (char**) malloc((numberOfSubstrings + 1) * sizeof(char*));

    for(i = 0; i < numberOfSubstrings; i++) {
        int tmp = 0;
        while(stringToTokenize[j] != delim && stringToTokenize[j] != NUL) {
            j++;
            tmp++;
        }

        tokenizedString[i] = (char*) malloc((tmp + 2) * sizeof(char));
        zeroOutString(tokenizedString[i], (tmp + 2));

        for(int k = 0; k < tmp; k++) {
            tokenizedString[i][k] = stringToTokenize[start + k];
        }
        tokenizedString[i][tmp + 1] = NUL;
        j++;
        start = j;
    }

    *length = numberOfSubstrings;
    return tokenizedString;
}

void freeStringArray(char** stringArray, int* length) {
    for(int i = 0; i < *length; i++) {
        free(stringArray[i]);
    }
}

/*
 *Input:
 *    User input string, and a pointer to a length
 *Output:
 *    a malloc'd array of char*
 * Ran into too many issues using strtok, instead parse string into array yourself
 * Responsible for tokenizing input
 */
char** s2a(char* stringVal, int* len) {
    char** stringArray;
    char delim = ' ';
    int numberOfSubstrings = 1;
    int i = 0;
    int j = 0;
    int start = 0;

    while(stringVal[i] != NUL) {
        if(stringVal[i] == delim) {
            if(stringVal[i + 1] != delim && stringVal[i + 1] != NUL) numberOfSubstrings++;
        }
        i++;
    }
    //printf("Number of Strings: %d\n", numberOfSubstrings);

    stringArray = (char**) malloc((numberOfSubstrings + 2) * sizeof(char*));// X char* + NULL

    for(i = 0; i < numberOfSubstrings; i++) {
        int tmp = 0;

        while(stringVal[j] != delim && stringVal[j] != '\0') {
            j++;
            tmp++;
        }
        stringArray[i] = (char*) malloc((tmp + 1) * sizeof(char));
        zeroOutString(stringArray[i], (tmp + 1));
        for(int k = 0; k < tmp; k++) {
            stringArray[i][k] = stringVal[start + k];
        }
        stringArray[i][tmp] = '\0';
        j++;
        start = j;
    }
    stringArray[numberOfSubstrings] = NULL;
    *len = numberOfSubstrings;
    return stringArray;
}