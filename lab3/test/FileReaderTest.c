//
// Created by Ben Straub 2020
//
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/FileReader.h"
#include "../src/Graph.h"
int main(void) {
    struct Graph* g;
    char* relativePath = "Data/test100nodes";
    char actualPath [PATH_MAX+1];
    char *ptr;

    ptr = realpath(relativePath, actualPath);


    printf("ptr: %d\n", ptr);
    printf("actualPath: %d\n", actualPath);
    g = readFile(ptr);

    linkStateRouting(g, 0, 1);

    //print Graph
    cleanUpGraph(g);
    return 0;
}
