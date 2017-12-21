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
#define     CIRCULAR_BUFFER_SIZE            8
#define     _BUFFER_DATA_TYPE_DEFAULT       int16_t

typedef _BUFFER_DATA_TYPE_DEFAULT   _BUFFER_DATA_TYPE;

typedef struct {
    _BUFFER_DATA_TYPE   buffer[CIRCULAR_BUFFER_SIZE];   //data 1D array
    int16_t             r;          //rear
    int16_t             f;          //front
    int16_t		        bufferSize; //buffer size

    /* member for frame-based processing */
    int16_t             frameSize;      //must be less than or equal to buffer size
    int16_t             overlap;        //overlap ratio, must between 0 to 1 (floating-point type)
} CircularBufferTypeDef;

/* Function Prototyping for circularBuffer.h */
void    CircularBuffer_Enqueue(CircularBufferTypeDef *targetBuffer, void *enqueueData, uint32_t enqueueSize);
void    CircularBuffer_Dequeue(CircularBufferTypeDef *targetBuffer, void *dequeueData, uint32_t dequeueSize);
void    CircularBuffer_Init(CircularBufferTypeDef *targetBuffer, int8_t SetFrameSize, int16_t SetOverlap);
uint8_t CircularBuffer_IsEmpty(CircularBufferTypeDef *targetBuffer);
uint8_t CircularBuffer_IsFull(CircularBufferTypeDef *targetBuffer);
int8_t  CircularBuffer_CheckNextFrameReady(CircularBufferTypeDef *buffer);

