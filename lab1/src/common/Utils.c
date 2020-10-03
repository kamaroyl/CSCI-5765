#include "Utils.h"
#include "Common.h"

const int DIR_MAX = 100;
const char NUL = '\0';
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
            fileDescription[nameLength + 5] = '\0';
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
            fileDescription[nameLength + 1] = '\0';
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
