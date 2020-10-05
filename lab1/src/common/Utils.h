#ifndef UTILS_H
#define UTILS_H
extern const int DIR_MAX;
char checkIfDirExists(const char* dir);
char checkIfFileExists(const char* fileName);
char changeDirectory(char* dir);
char* getDirContents(char* currentDir, int* success);
void zeroOutString(char* myString, int length);
void cleanInput(char* userInput);
char isWhitespace(char input);
void printPromptTag();
void getUserInput(char* inputPtr);
enum command stringToCommand(char* userInput);
char** s2a(char* stringVal, int* len);
void freeTokenizedInput(int length, char** tokenizedInput);
#endif //UTILS_H
