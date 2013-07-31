/*
 * AUTHOR: Jonas Van Pelt
 * source: wikipedia
 */

/* Circular buffer example, keeps one slot open */
 
#include <stdio.h>
#include <stdlib.h>
#include "circular_buffer.h"
 
void cbInit(CircularBuffer *cb, int size) {
    cb->size  = size + 1; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->elems = (ElemType *)calloc(cb->size, sizeof(ElemType));
}
 
void cbFree(CircularBuffer *cb) {
    free(cb->elems); /* OK if null */ }
 
int cbIsFull(CircularBuffer *cb) {
    return (cb->end + 1) % cb->size == cb->start; }
 
int cbIsEmpty(CircularBuffer *cb) {
    return cb->end == cb->start; }
 
/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
void cbWrite(CircularBuffer *cb, ElemType *elem) {
    cb->elems[cb->end] = *elem;
    cb->end = (cb->end + 1) % cb->size;
    if (cb->end == cb->start)
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
}
 
/* Read oldest element. App must ensure !cbIsEmpty() first. */
void cbRead(CircularBuffer *cb, ElemType *elem) {
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
}
 
/*int main(int argc, char **argv) {
    CircularBuffer cb;
    ElemType elem = {0};
 
    int testBufferSize = 64; 
    cbInit(&cb, testBufferSize);
 
    int i;
    for (i=50;i<100;i++)
    {
		elem.value[0]=i;
		cbWrite(&cb, &elem);
	}
        
 
    while (!cbIsEmpty(&cb)) {
        cbRead(&cb, &elem);
        printf("%s\n", elem.value);
    }
 
    cbFree(&cb);
    return 0;
}*/
