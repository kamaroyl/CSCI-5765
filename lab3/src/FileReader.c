//
// Created by benja on 11/1/2020.
//
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileReader.h"

struct Graph* readFile(char* fileLocation) {
    struct Graph* graph;
    FILE* fp = fopen(fileLocation, "rb");
    int error = 0;
    if ((error = errno)) {
        printf("Could not open file: %d \n", error);
        exit(1);
    }
    int tmp1;
    int tmp2;
    int tmp3;
    char line[512];
    char* ptr = line;
    size_t currIndex = 0;
    ssize_t lineSize = 0;
    int numberOfVertices = 0;
    char numberBuffer[16];
    memset(numberBuffer, 0, 16);
    int index = 0;
    int jndex = 0;

    // Read in the number of nodes
    lineSize = getline(&ptr, &currIndex, fp);


    while (ptr[index] != '\n') {
        //printf("%c\n", ptr[index]);
        numberBuffer[index] = ptr[index];
        index++;
    }
    numberOfVertices = atoi(numberBuffer);

    if (numberOfVertices <= 0) {
        printf("Reading File Header Failed;\n number of vertices was less than or equal to zero\n");
        exit(1);
    }
    graph = createGraph(numberOfVertices);
    printf("Number of Vertices: %d\n", numberOfVertices);
    while ((lineSize = getline(&ptr, &currIndex, fp)) != -1) {
        index = 0;

        //printf("ptr: %s", ptr);
        memset(numberBuffer, 0, 16);
        //printf("Origin: ");
        while (ptr[index] != ' ') {
            numberBuffer[index] = ptr[index];
            index++;
        }
        //printf("%s\n", numberBuffer);
        tmp1 = atoi(numberBuffer);

        index++;
        jndex = 0;

        memset(numberBuffer, 0, 16);
        //printf("Dest: ");
        while (ptr[index] != ' ') {
            numberBuffer[jndex++] = ptr[index++];
        }
        //printf("%s\n", numberBuffer);
        tmp2 = atoi(numberBuffer);

        index++;
        jndex = 0;

        memset(numberBuffer, 0, 16);
        //printf("Weight: ");
        while (ptr[index] != '\n') {
            numberBuffer[jndex++] = ptr[index++];
        }
        //printf("%s\n", numberBuffer);
        tmp3 = atoi(numberBuffer);

        //shift all edges over 1 since input file has minimum vertex of 1
        addEdge(graph, tmp1 - 1, tmp2 - 1, tmp3);
        //printf("\n");
    }
    free(ptr);
    fclose(fp);
    return graph;
}