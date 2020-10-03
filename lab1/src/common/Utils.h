#ifndef UTILS_H
#define UTILS_H
extern const char NUL;
extern const int DIR_MAX;
char checkIfDirExists(const char* dir);
char checkIfFileExists(const char* fileName);
char changeDirectory(char* dir);
char* getDirContents(char* currentDir, int* success);
void zeroOutString(char* myString, int length);
#endif //UTILS_H
