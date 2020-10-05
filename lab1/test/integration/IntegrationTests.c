#include "../../src/common/Common.h"
#include "../../src/common/Utils.h"

void testGetUserInput() {
    char input[4*ARG_MAX];
    getUserInput(&input[0]);
    printf("User Input: %s\n", input);
}

int main(void) {
    testGetUserInput();
}