#ifndef UTILS_H
#define UTILS_H
extern const char NUL;
extern const int DIR_MAX;
char changeDirectory(char* dir);
char* getDirContents(char* currentDir, int* success);
void zeroOutString(char* myString, int length);
#endif //UTILS_H
