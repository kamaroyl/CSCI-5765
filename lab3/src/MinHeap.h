//
// Created by Ben Straub 2020.
//

#ifndef _MINHEAP_H
#define _MINHEAP_H
#define PARENT(index) (int)((index - 1)/2)
#define LEFT(index) (2 * index + 1)
#define RIGHT(index) (2 * index + 2)
struct MinHeapNode {
    int vertex;
    int distance;
};
struct MinHeap {
    int size;
    int capacity;
    int* position;
    struct MinHeapNode** data;
};

struct MinHeapNode* createMinHeapNode(int vertex, int distance);
struct MinHeap* createMinHeap(int capacity);
void decreaseKey(struct MinHeap* minHeap, int vertex, int distance);
void heapify(struct MinHeap* minHeap, int index);
char isHeapEmpty(struct MinHeap* minHeap);
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode);
char minHeapContains(struct MinHeap* minHeap, int vertex);
struct MinHeapNode* popMin(struct MinHeap* minHeap);
void cleanUpMinHeapNode(struct MinHeapNode* minHeapNode);
void cleanUpMinHeap(struct MinHeap* minHeap);

// TESTS //
void printCurrentMinHeap(struct MinHeap* minHeap);
void _swapMinHeapNode(struct MinHeapNode** one, struct MinHeapNode** two);
#endif
