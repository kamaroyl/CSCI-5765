//
// Created by ben straub 2020.
//

#ifndef _GRAPH_H
#define _GRAPH_H
#include <stdio.h>
#include <stdlib.h>
#define INFINITY 1073741824
struct Node {
    int destination;
    int weight;
    struct Node* next;
};

/** Singly linked list**/
struct AdjacencyList {
    struct Node* head;
};

struct Graph {
    int vertexCount;
    struct AdjacencyList** adjacencyLists;
};

struct Weight {
    int previous;
    int weight;
    char checked;
};

void addEdge(struct Graph* graph,
             int origin,
             int destination,
             int weight);
struct Node* newNode(int weight, int destination);
struct Graph* createGraph(int size);

void cleanUpGraph(struct Graph* graph);

void linkStateRouting(struct Graph* graph, int origin, int debug);
#endif
