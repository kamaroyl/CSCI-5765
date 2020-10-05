//
// Created by benja on 10/2/2020.
//
#include <assert.h>
#include "../../src/common/Common.h"
#include "../../src/common/Utils.h"
#define BOOL_STRING(C) C == 0? "False" : "True"

void testHash() {
    int hashes[10];
    for( int i = 0; i < 10; ++i) {
        printf("%s: %d\n", commandStrings[i], hash(commandStrings[i])%1000);
        hashes[i] = hash(commandStrings[i])%1000;
        for( int j = 0; j < i; ++j) {
            assert(hashes[j] != hashes[i]);
        }
    }
}

void testIsWhiteSpace() {
    char a = ' ';
    printf("is ' ' whitespace? %s\n", BOOL_STRING(a));
    printf("is %c whitespace? %s\n", 0x09, BOOL_STRING(0x09));
}

void testCleanInput() {
    char input[] ="a!b#c$d%e&f{g}h'i\"j;k`l|m<n>o:p";
    printf("Input: %s\n", &input[0]);
    cleanInput(&input[0]);
    printf("Output: %s\n", &input[0]);
}

void tests2a_SingleToken() {
    int length = 0;
    char* input = "bye";
    char** output = s2a(input, &length);
    printf("Length = %d\n", length);
    for(int i = 0; i < length; ++i) {
        printf("Output: %s\n", output[i]);
    }
    freeTokenizedInput(length, output);

}

void tests2a_MultipleTokens() {
    int length = 0;
    char* input = "bye guy";
    char** output = s2a(input, &length);
    printf("Length = %d\n", length);
    for(int i = 0; i < length; ++i) {
        printf("Output: %s\n", output[i]);
    }
    freeTokenizedInput(length, output);

}

int main(void) {
    testHash();
    testIsWhiteSpace();
    testCleanInput();
    tests2a_SingleToken();
    tests2a_MultipleTokens();
    //testGetUserInput();
    return 0;
}
