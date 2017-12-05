/**
  * circularBuffer.h - circular buffer (FIFO) implementation in C.
  *
  * Written in 2017 by Nattapong W. <https://github.com/jazzpiano1004>.
  *
  * To the extent possible under law, the author(s) have dedicated all
  * copyright and related and neighboring rights to this software to
  * the public domain worldwide. This software is distributed without
  * any warranty.
  *
  */

#ifndef  CIRCULARBUFFER_H
#define  CIRCULARBUFFER_H
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Define for buffer data structure */
#define     BUFFER_SIZE                 8
#define     _BUFFER_DATA_TYPE_DEFAULT   int16_t

typedef _BUFFER_DATA_TYPE_DEFAULT   _BUFFER_DATA_TYPE;

typedef struct {
    _BUFFER_DATA_TYPE       buffer[BUFFER_SIZE];
    int16_t                 rear;
    int16_t                 front;
    int16_t		            size;

} CircularBufferTypeDef;

void CircularBuffer_Enqueue(CircularBufferTypeDef *targetBuffer, _BUFFER_DATA_TYPE *enqueueData, uint32_t enqueueSize);
void CircularBuffer_Dequeue(CircularBufferTypeDef *targetBuffer, _BUFFER_DATA_TYPE *dequeueData, uint32_t dequeueSize);
void CircularBuffer_Init(CircularBufferTypeDef *targetBuffer);
