#include "Utils.h"
#include "Common.h"

const int DIR_MAX = 100;

//https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
char* getDirContents(char* currentDir, int* success) {
    struct dirent* de;
    DIR* dr = opendir(currentDir);
    char** holder = (char**) malloc(DIR_MAX * sizeof(char*));
    int thisStringSize[DIR_MAX];
    short index = 0;
    unsigned int totalStringLength = 0;
    if( dr == NULL ) {
        printf("Could not open current directory\n");
        *success = -1;
        return NULL;
    }

    while ((de = readdir(dr)) != NULL && index < DIR_MAX) {
        char* fileName = de->d_name;
        int nameLength = strlen(fileName);

        if( de->d_type == DT_DIR ) {
            thisStringSize[index] = nameLength + 5;
            totalStringLength += nameLength + 5;
            char* fileDescription = (char*) malloc((nameLength + 6) * sizeof(char));
            for(int i = 0; i < nameLength; ++i) {
                fileDescription[i] = fileName[i];
            }
            fileDescription[nameLength] = ' ';
            fileDescription[nameLength + 1] = '(';
            fileDescription[nameLength + 2] = 'D';
            fileDescription[nameLength + 3] = ')';
            fileDescription[nameLength + 4] = '\n';
            fileDescription[nameLength + 5] = NUL;
            holder[index] = fileDescription;
            fileDescription = NULL;
            ++index;
        } else if(de->d_type == DT_REG ) {
            thisStringSize[index] = nameLength + 1;
            totalStringLength += nameLength + 1;
            char* fileDescription = (char*) malloc((nameLength + 2) * sizeof(char));
            for(int i = 0; i < nameLength; ++i) {
                fileDescription[i] = fileName[i];
            }
            fileDescription[nameLength] = '\n';
            fileDescription[nameLength + 1] = NUL;
            holder[index] = fileDescription;
            fileDescription = NULL;
            ++index;
        }

    }
    closedir(dr);
    //printf("Total Length Of Strings: %d\n", totalStringLength);
    char* buff = (char*) malloc(totalStringLength* sizeof(char));
    int k = 0;
    for(int i = 0; i < index; ++i) {
        for(int j = 0; j < thisStringSize[i]; ++j) {
            //printf("char at i: %d, j: %d = %c\n", i, j, holder[i][j]);
            buff[k] = holder[i][j];
            ++k;
        }
        free(holder[i]);
    }
    free(holder);
    *success = 0;
    return buff;
}

char checkIfDirExists(const char* dir) {
    struct stat statData;
    return !stat(dir, &statData) && S_ISDIR(statData.st_mode);
}

char checkIfFileExists(const char* fileName) {
    struct stat statData;
    return !stat(fileName, &statData) && S_ISREG(statData.st_mode);
}

char changeDirectory(char* dir) {
    int success = -1;
    if(dir != NULL && strlen(dir) > 0) {
        if( checkIfDirExists(dir) > 0) {
            char *absolutePath = malloc(PATH_MAX);
            realpath(dir, absolutePath);
            chdir(absolutePath);
            setenv("PWD", absolutePath, 1);
            free(absolutePath);
            success = 0;
        }
    }
    return success;
}

void zeroOutString(char* myString, int length) {
    int i = 0;
    while(i < length) {
        myString[i] = NUL;
        ++i;
    }
}

void cleanInput(char* userInput) {
    int offset = 0;
    char* tmp = (char*) malloc(strlen(userInput) * sizeof(char));
    char disallowedCharacters[] = {';', ':', '&', '|', '<', '>', '\'', '\"', '`', '$', '{', '}', '!', '%', '#' };
    for(int i = 0; i < strlen(userInput) + 1; ++i) {
        for(int j = 0; j < sizeof(disallowedCharacters); ++j) {
            if(userInput[i + offset] == disallowedCharacters[j]) ++offset;

        }
        tmp[i] = userInput[i + offset];
    }
    for(int i = 0; i < strlen(tmp) + 1; ++i) {
        userInput[i] = tmp[i];
    }
}

char isWhitespace(char input) {
    return input == ' ' || input == 0x09;//tab
}

//From my undergraduate OS lab 2
//https://github.com/kamaroyl/csci-3453/blob/master/lab/Lab_2/lab2.c
void printPromptTag() {
    printf("FTP> ");
}

// Strips extra white space.
void getUserInput(char* inputPtr){
    int i = 0;
    int j = 0;
    char tmpInput[ARG_MAX];
    fgets(&tmpInput[0], ARG_MAX, stdin);
    int length = strlen(&tmpInput[0]);
    printf("Input: %s\n", &tmpInput[0]);
    while(tmpInput[i] == ' ' || tmpInput[i] == '	') i++;
    int k = 0;
    for(; j < length - i - 1; j++) {
        printf("j: %d i: %d ", j,i);
        if(isWhitespace(tmpInput[j + i]) && isWhitespace(tmpInput[j+i+1])) {
            printf("\n");
            ++k;
        } else {
            printf("Char: %c\n", tmpInput[j + i]);
            inputPtr[j-k] = tmpInput[j + i];
        }
    }
    inputPtr[j] = NUL;
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
    int numberOfSubstrings = 1;
    int i = 0;
    int j = 0;
    int start = 0;

    while(stringVal[i] != NUL) {
        if(isWhitespace(stringVal[i])) {
            if(!isWhitespace(stringVal[i + 1]) && stringVal[i + 1] != NUL) {
                numberOfSubstrings++;
            }
        }
        i++;
    }
    //printf("Number of Strings: %d\n", numberOfSubstrings);

    stringArray = (char**) malloc((numberOfSubstrings + 2) * sizeof(char*));// X char* + NULL

    for(i = 0; i < numberOfSubstrings; i++) {
        int tmp = 0;
        while(!isWhitespace(stringVal[j])  && stringVal[j] != NUL) {
            j++;
            tmp++;
        }
        stringArray[i] = (char*) malloc((tmp + 1) * sizeof(char));
        zeroOutString(stringArray[i], (tmp + 1));
        for(int k = 0; k < tmp; k++) {
            stringArray[i][k] = stringVal[start + k];
        }
        stringArray[i][tmp] = NUL;
        j++;
        start = j;
    }
    stringArray[numberOfSubstrings] = NULL;
    *len = numberOfSubstrings;
    return stringArray;
}


/*
 *Input:
 *    length - number of initialized strings
 *    tokenizedInput - the array of strings
 *
 *Output:
 *    void
 *
 *Description:
 *    utility function to free the memory allocated for the string array
 */
void freeTokenizedInput(int length, char** tokenizedInput) {
    int index = 0;
    for(; index < length; index++) {
        //printf("freeing tokenizedInput[%d]: \n", index);
        //printf("%s\n", tokenizedInput[index]);
        free(tokenizedInput[index]);
        //printf("successfully freed tokenizedInput[%d]\n", index);
    }
    free(tokenizedInput);
}
