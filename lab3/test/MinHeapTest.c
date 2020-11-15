//
// Created by Ben Straub on 2020.
//
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/MinHeap.h"

void testSwap() {
    struct MinHeapNode** data =(struct MinHeapNode**) malloc(sizeof(struct MinHeapNode*) * 2);
    struct MinHeapNode* node0 = createMinHeapNode(0, 1);
    struct MinHeapNode* node1 = createMinHeapNode(1, 2);
    data[0] = node0;
    data[1] = node1;
    _swapMinHeapNode(&data[0], &data[1]);
    assert(data[0]->vertex == node1->vertex && data[1]->vertex == node0->vertex);
    free(node0);
    free(node1);
    data[0] = NULL;
    data[1] = NULL;
    free(data);
}

void testUpdate() {
    struct MinHeap* minHeap = createMinHeap(5);
    for(int i = 0; i < 5; i++) {
        minHeap->data[i] = createMinHeapNode(i, 99999);
    }
    minHeap->size = 5;
    printCurrentMinHeap(minHeap);
    decreaseKey(minHeap, 3, 5);
    printCurrentMinHeap(minHeap);
    cleanUpMinHeap(minHeap);
}

int main(void) {
    printf("LEFT of 1 should be 3: %d\n", LEFT(1));
    printf("LEFT of 2 should be 5: %d\n", LEFT(2));
    printf("RIGHT of 1 should be 4: %d\n", RIGHT(1));
    printf("RIGHT of 2 should be 6: %d\n", RIGHT(2));
    printf("PARENT of 1 should be 0: %d\n", PARENT(1));
    printf("PARENT of 2 should be 0: %d\n", PARENT(2));
    testSwap();
    testUpdate();
    return 0;
}