// Much of the implementation taken from my command line implementation from csci-3453 because I hate
// c's string api
// https://github.com/kamaroyl/csci-3453/blob/master/lab/Lab_2/lab2.c
#ifndef MYSTRING_H
#define MYSTRING_H
//Const
const char colonToken = ':';
const char spaceToken = ' ';
const char tabToken = 9;
const char newLine = '\n';
const char escapeToken = '\\';
const char quoteToken = '"';
const char rightArrowToken = '>';
const char leftArrowToken = '<';
const char semicolonToken = ';';
const char NUL = '\0';

// Functions
void zeroOutString(char* myString, int length);
int myStringLength(char* myString);

//    strtok mangles the input string. Instead of copying to a new string and mangling that string
//    lets copy the values to a new array.
//    return will need to be freed
//    Used to parse the path and throw it into a global variable
char** stringToArray(char delim, char* stringToTokenize, int* length);

//Frees the entries of array (char*s) but not the array (char**) or the length (int*)
void freeStringArray(char** stringArray, int* length)
#endif //MYSTRING_H
