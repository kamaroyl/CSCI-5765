//
// Created by benja on 11/7/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include "MinHeap.h"


/**
 *                                             0
 *                                           1   2
 *                                          3 4 5 6
 *                                          ...  ...
 **/

void _swapMinHeapNode(struct MinHeapNode** one, struct MinHeapNode** two) {
    struct MinHeapNode* tmp = *one;
    *one = *two;
    *two = tmp;
}

struct MinHeapNode* createMinHeapNode(int vertex, int distance) {
    struct MinHeapNode* minHeapNode = (struct MinHeapNode*) malloc(sizeof(struct MinHeapNode));
    minHeapNode->vertex = vertex;
    minHeapNode->distance = distance;
    return minHeapNode;
}

void cleanUpMinHeapNode(struct MinHeapNode* minHeapNode) {
    free(minHeapNode);
}

struct MinHeap* createMinHeap(int capacity) {
    //Check if number is odd, if so, make it even
    capacity = capacity & 1 ? capacity + 1 : capacity;
    //printf("Malloc MinHeap\n");
    struct MinHeap* minHeap = (struct MinHeap*) malloc(sizeof(struct MinHeap));
    minHeap->capacity = capacity;
    minHeap->size = 0;
    //printf("Malloc position\n");
    minHeap->position = (int*) malloc(sizeof(int) * capacity);
    //printf("Malloc data\n");
    minHeap->data = (struct MinHeapNode**) malloc(sizeof(struct MinHeapNode*) * capacity);
    return minHeap;
}

void cleanUpMinHeap(struct MinHeap* minHeap) {
    // Clean up all nodes
    struct MinHeapNode* tmpPtr;
    while((tmpPtr = popMin(minHeap)) != NULL) {
        //printf("Free MinHeapNode %d\n", tmpPtr->vertex);
        free(tmpPtr);
    }
    // Clean up all position pointers
    //printf("Free position\n");
    free(minHeap->position);
    // Clean up data
    //printf("Free Data\n");
    free(minHeap->data);
}

void _expandMinHeap(struct MinHeap* minHeap, int expansionFactor) {
    minHeap->capacity *= expansionFactor;
    struct MinHeapNode** tmpPtr = minHeap->data;
    int* tmpPositionPtr = minHeap->position;
    minHeap->data = (struct MinHeapNode**) malloc(sizeof(struct MinHeapNode*) * minHeap->capacity);
    minHeap->position = (int*) malloc(sizeof(int) * minHeap->capacity);
    for (int i = 0; i < minHeap->size; i++) {
        minHeap->data[i] = tmpPtr[i];
        minHeap->position[i] = tmpPositionPtr[i];
        tmpPtr[i] = NULL;
    }
    free(tmpPtr);
    free(tmpPositionPtr);
}

void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode) {
    int i;
    if (minHeap->size == minHeap->capacity) {
        _expandMinHeap(minHeap, 2);
    }
    i = minHeap->size;
    minHeap->data[i] = minHeapNode;
    minHeap->position[minHeapNode->vertex] = i;
    minHeap->size++;

    while ( i > 0 &&
            minHeap->data[PARENT(i)]->distance > minHeap->data[i]->distance) {
        //Swap positions in position array
        minHeap->position[minHeap->data[i]->vertex] = (int)PARENT(i);
        minHeap->position[minHeap->data[PARENT(i)]->vertex] = i;
        _swapMinHeapNode(&(minHeap->data[PARENT(i)]), &(minHeap->data[i]));

        i = PARENT(i);
    }

}

void decreaseKey(struct MinHeap* minHeap, int vertex, int distance) {
    //printf("Decrease Key: Vertex: %d, Distance: %d\n", vertex, distance);
    int tmp = minHeap->position[vertex];
    //printf("tmp at position: %d\n", tmp);
    minHeap->data[tmp]->distance = distance;

    // not at root and wight of vertex is less than it's parent
    while (tmp != 0 &&
           minHeap->data[tmp]->distance <  minHeap->data[PARENT(tmp)]->distance) {

        //update position array to reflect new positions
        minHeap->position[minHeap->data[tmp]->vertex] = (int)PARENT(tmp);
        minHeap->position[minHeap->data[PARENT(tmp)]->vertex] = tmp;

        //swap heap positioning
        _swapMinHeapNode(&(minHeap->data[tmp]), &(minHeap->data[PARENT(tmp)]));

        //next look at next parent
        tmp = PARENT(tmp);
    }
}
void heapify(struct MinHeap* minHeap, int index) {
    int min = index;
    int left = LEFT(index);
    int right = RIGHT(index);

    if (left < minHeap->size &&
        minHeap->data[left]->distance < minHeap->data[min]->distance) {
        min = left;
    }

    if (right < minHeap->size &&
        minHeap->data[right]->distance < minHeap->data[min]->distance) {
        min = right;
    }

    if (min != index) {
        struct MinHeapNode* minimumNodePtr = minHeap->data[min];
        struct MinHeapNode* currentNodePtr = minHeap->data[index];

        //Update position array so we can easily find the nodes
        minHeap->position[minimumNodePtr->vertex] = index;
        minHeap->position[currentNodePtr->vertex] = min;

        _swapMinHeapNode(&(minHeap->data[min]), &(minHeap->data[index]));
        heapify(minHeap, min);
    }
}

char isHeapEmpty(struct MinHeap* minHeap) {
    return minHeap->size == 0;
}

char minHeapContains(struct MinHeap* minHeap, int vertex) {
    return minHeap->position[vertex] < minHeap->size;
}

struct MinHeapNode* popMin(struct MinHeap* minHeap) {
        if (isHeapEmpty(minHeap)) return NULL;
        int HEAD = 0;
        struct MinHeapNode* rootPtr = minHeap->data[HEAD];
        struct MinHeapNode* largePtr = minHeap->data[minHeap->size - 1];
        minHeap->data[HEAD] = largePtr;

        minHeap->position[rootPtr->vertex] = minHeap->size - 1;
        minHeap->position[largePtr->vertex] = HEAD;
        (minHeap->size)--;
        heapify(minHeap, HEAD);
        return rootPtr;
}

void printCurrentMinHeap(struct MinHeap* minHeap) {
    printf("\nCurrent Heap: \n");
    struct MinHeapNode* holder = NULL;
    for (int i = 0; i < minHeap->size - 1; i++) {
        holder = minHeap->data[i];
        printf("Vertex: %d, Distance: %d, Position %d\n", holder->vertex, holder->distance, minHeap->position[holder->vertex]);
    }
    printf("\n");
}