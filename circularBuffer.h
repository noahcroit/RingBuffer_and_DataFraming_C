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

#ifndef  __CIRCULARBUFFER_H
#define  __CIRCULARBUFFER_H


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Define for default setup of buffer data structure */
#define     DEFAULT_CIRCULAR_BUFFER_SIZE    2048
#define     _DEFAULT_BUFFER_DATA_TYPE       int32_t

/* Define of all buffer states */
#define     BUF_STATE_EMPTY                 0
#define     BUF_STATE_FULL                  1
#define     BUF_STATE_R_MORE_THAN_F         2
#define     BUF_STATE_R_LESS_THAN_F         3

typedef _DEFAULT_BUFFER_DATA_TYPE   _RING_BUFFER_DATA_TYPE;

typedef struct {

    void                *buf;           //pointer of 1-D data array
    int32_t             r;              //rear
    int32_t             f;              //front
    int32_t		        	bufferSize;     //buffer size (elements)
    int8_t              elementSize;    //size per element (bytes)

} circularBuffer_TypeDef;

/* Function Prototyping for circularBuffer.h */
void CircularBuffer_Enqueue (circularBuffer_TypeDef *targetBuf,
                             const void *enqueueData,
                             uint32_t enqueueSize);

void CircularBuffer_Dequeue (circularBuffer_TypeDef *targetBuf,
                             void *dequeueData,
                             uint32_t dequeueSize);

void CircularBuffer_Init    (circularBuffer_TypeDef *targetBuf,
                             void *pBuf,
                             int8_t SetElementSize,
                             int32_t SetBufferSize);

void    CircularBuffer_Flush    (circularBuffer_TypeDef *targetBuf);
uint8_t CircularBuffer_IsEmpty  (circularBuffer_TypeDef *targetBuf);
uint8_t CircularBuffer_IsFull   (circularBuffer_TypeDef *targetBuf);

#endif
