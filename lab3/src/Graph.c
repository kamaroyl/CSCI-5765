//
// Created by Ben Straub on 2020.
//
#include <string.h>

#include "Graph.h"
#include "MinHeap.h"

#define TRUE 1
#define FALSE 0
#define MIN(a,b) a < b ? a : b

struct Graph* createGraph(int size) {
   //printf("Malloc Graph\n");
    struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph));
    int i = 0;
    graph->vertexCount = size;
    //printf("Malloc List of lists\n");
    graph->adjacencyLists = (struct AdjacencyList**) malloc(sizeof(struct AdjacencyList*) * size);
    while(i < size) {
       // printf("Malloc List %d\n", i);
        graph->adjacencyLists[i] =
                (struct AdjacencyList*) malloc(sizeof(struct AdjacencyList));
        graph->adjacencyLists[i++]->head = NULL;
    }
    return graph;
}

struct Node* newNode(int weight, int destination) {
    //printf("Malloc new node\n");
    struct Node* node = (struct Node*) malloc(sizeof(struct Node));
    node->weight = weight;
    node->destination = destination;
    node->next = NULL;
    return node;
}

void _pushNode(struct AdjacencyList* adjacencyList, struct Node* node) {
    struct Node* tmp = adjacencyList->head;
    adjacencyList->head = node;
    node->next = tmp;
}

void addEdge(struct Graph* graph,
                      int origin,
                      int destination,
                      int weight) {
    _pushNode(graph->adjacencyLists[origin], newNode(weight, destination));
    _pushNode(graph->adjacencyLists[destination], newNode(weight, origin));
}

void _cleanUpNode(struct Node* node) {
    node->next = NULL;
    //printf("Free Node\n");
    free(node);
}

void _cleanUpAdjacencyList(struct AdjacencyList* adjacencyList, int i) {
    struct Node* tmpPtr;
    while(adjacencyList->head != NULL) {
        tmpPtr = adjacencyList->head;
        adjacencyList->head = adjacencyList->head->next;
        _cleanUpNode(tmpPtr);
    }
    //printf("Free List %d\n", i);
    free(adjacencyList);
}

void cleanUpGraph(struct Graph* graph) {
    int i = 0;
    //printf("graph->vertexCount = %d\n", graph->vertexCount);
    while (i < graph->vertexCount) {
        _cleanUpAdjacencyList(graph->adjacencyLists[i], i);
        i++;
    }
    //printf("Free List of lists\n");
    free(graph->adjacencyLists);
    //printf("Free Graph\n");
    free(graph);
}

void _printForwardingTable(struct Weight* weights, struct Graph* graph, int origin) {
    int prev;
    int link;
    int i;
    char buffer[16];
    char length = 0;
    origin++; //for external
    printf("Forwarding Table:\n");
    printf("Destination    |      Link   \n");
    printf("------------------------------\n");
    for(i = 0; i < graph->vertexCount; i++) {
        memset(buffer, 0, 16);
        sprintf(buffer, "%d", (i + 1));
        length = strlen(buffer);
        while (length < 10) {
            buffer[length++] = ' ';
        }
        prev = i;
        while (weights[prev].previous != origin - 1) {
            prev = weights[prev].previous;
            //printf("weights[%d].previous = %d\n", prev + 1, weights[prev].previous + 1);
        }
        link = prev + 1;
        printf("     %s|      (%d,%d)\n", buffer, origin, link);
    }
}

void _debugPrint(struct Graph* graph, struct Weight* weights, char* pathString) {
    printf("Path String: %s\n", pathString);
    char* delim = "        ";
    printf("src%sprv%swgt\n", delim, delim);
    for(int i = 0; i < graph->vertexCount; i++) {
        printf("%d  %s%d  %s%d\n", (i + 1), delim, (weights[i].previous + 1), delim, weights[i].weight);
    }
    printf("\n");
}

void updatePathString(char* string, int newVertex) {
    int i = 0;
    int j = 0;
    char buffer[16];
    memset(buffer, 0, 16);
    //Set vertex to external
    newVertex++;
    //find end of current string
    while(string[i] != '\0') {
        i++;
    }
    if (i == 0) {
        sprintf(buffer, "%d", newVertex);
    } else {
        sprintf(buffer, ",%d", newVertex);
    }

    while(buffer[j] != '\0') {
        string[i++] = buffer[j++];
    }

}

void linkStateRouting(struct Graph* graph, int origin, int debug) {

    //printf("DIJKSTRA\n");
    //printf("Vertex Count: %d\n", graph->vertexCount);
    int i = 0;
    struct Weight* weights = (struct Weight*) malloc(sizeof(struct Weight) * graph->vertexCount);
    struct MinHeap* minHeap = createMinHeap(graph->vertexCount);
    struct MinHeapNode* minPtr;
    struct Node* graphPtr;
    int srcPtr;
    int dstPtr;

    char* string = (char*) malloc(sizeof(char) * 10 * graph->vertexCount);
    memset(string, 0, 10 * graph->vertexCount);

    // temporary variables for comparison in inner loop
    int distancePlusCost;
    int minimizedWeight;
    int previousCost;

    //Initialize Weights vector
    while (i < graph->vertexCount) {
        //create all the minHeap nodes and set their positions to their vertex
        minHeap->data[i] = createMinHeapNode(i, INFINITY);
        minHeap->position[i] = i;
        weights[i].checked = 0;
        weights[i].previous = -1;
        weights[i++].weight = INFINITY;

    }
    //printCurrentMinHeap(minHeap);
    minHeap->size = graph->vertexCount;

    //set weight at origin to 0
    weights[origin].weight = 0;
    weights[origin].previous = origin;
    weights[origin].checked = TRUE;
    decreaseKey(minHeap, origin, 0);
    heapify(minHeap, origin);
    //printCurrentMinHeap(minHeap);

    //While there are still vertices in the min heap
    while(!isHeapEmpty(minHeap)) {

        minPtr = popMin(minHeap);
        srcPtr = minPtr->vertex;
        updatePathString(string, srcPtr);
        //printf("PROCESSING VERTEX: %d\n", srcPtr);
        graphPtr = graph->adjacencyLists[srcPtr]->head;

        while (graphPtr != NULL) {
            dstPtr = graphPtr->destination;
            //printf("PROCESSING EDGE:(%d,%d)\n", srcPtr, dstPtr);
            previousCost = weights[dstPtr].weight;
            //printf("PRV COST IS: %d\n", previousCost);

            distancePlusCost = graphPtr->weight + weights[srcPtr].weight;
            //printf("NEW COST IS: %d\n", distancePlusCost);

            minimizedWeight = MIN(distancePlusCost, previousCost);
            //printf("MIN COST IS: %d\n", minimizedWeight);
            if (minHeapContains(minHeap, dstPtr)) {
                decreaseKey(minHeap, dstPtr, minimizedWeight);
            }
            //Update Weights if minimized weight is less
            if (weights[dstPtr].weight > minimizedWeight) {
                weights[dstPtr].weight = minimizedWeight;
                weights[dstPtr].previous = srcPtr;
            }
            //Update currently
            graphPtr = graphPtr->next;
        }
        weights[srcPtr].checked = TRUE;
        if (debug) {
            _debugPrint(graph, weights, string);
        }
        //printf("Freeing %d\n", minPtr->vertex);
        free(minPtr);
        //printCurrentMinHeap(minHeap);
    }
    _printForwardingTable(weights, graph, origin);

    // CLEAN UP
    cleanUpMinHeap(minHeap);
    free(string);
    free(minHeap);
    free(weights);

}


