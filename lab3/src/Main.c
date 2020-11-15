// Created by Ben Straub on 2020.
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "./FileReader.h"
#include "./Graph.h"

void usage() {
    printf("Usage:\n");
    printf("linkStateRouting ${FileLocation} ${node number} ${optional debug flag\n");
    printf("e.g. linkStateRouting ./Data/test10nodes 3 1\n");
}

/**
 * Inputs:
 *     0 //executable name
 *     1 Path
 *     2 Starting Node
 *     3 Debug
 */

int main(int argc, char** argv) {
    struct timeval start, end;
    struct Graph* g = NULL;
    int startingNode = 0;
    char debug = 0;
    char actualPath [PATH_MAX+1];
    char *ptr;

    gettimeofday(&start, NULL);

    if (argc < 3) {
        printf("Not enough arguments\n");
        usage();
        exit(1);
    } else if(argc == 4) {
        debug = (char) atoi(argv[3]);
        if (debug > 1 || debug < 0) {
            printf("invalid debug value: %d\n", debug);
            exit(1);
        }
    }

    ptr = realpath(argv[1], actualPath);
    g = readFile(ptr);
    startingNode = atoi(argv[2]);
    if (startingNode <= 0 || startingNode > g->vertexCount) {
        printf("starting node must be an integer greater than 0 and less than or equal to the number of vertices\n");
        printf("Errno is: %d \nInput for starting node was: %d\n", errno, startingNode);
        goto cleanup;
    }
    //Graph files assume 1 indexed nodes, while c uses 0 indexed, so convert.
    startingNode--;
    linkStateRouting(g, startingNode, debug);

    gettimeofday(&end, NULL);
    printf("Total Elapsed Time: %lu s, %lu us\n", (end.tv_sec - start.tv_sec), (end.tv_usec - start.tv_usec));
cleanup:
    cleanUpGraph(g);
    return 0;
}
