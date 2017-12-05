/**
  * circularBuffer.c - circular buffer (FIFO) implementation in C.
  *
  * Written in 2017 by Nattapong W. <https://github.com/jazzpiano1004>.
  *
  * To the extent possible under law, the author(s) have dedicated all
  * copyright and related and neighboring rights to this software to
  * the public domain worldwide. This software is distributed without
  * any warranty.
  *
  * How to use this file:
    --------------------
    + First, you need to declare a data structure "CircularBufferTypeDef" in circularBuffer.h for a FIFO buffer as an argument for these functions.
    + There're 3 main functions for FIFO buffer,
    + 1) For en-queue operation, call the function CircularBuffer_Enqueue(
                                                                          CircularBufferTypeDef *targetBuffer : target FIFO buffer,
                                                                          _BUFFER_DATA_TYPE *enqueueData : en-queue data pointer,
                                                                          uint32_t enqueueSize : en-queue data size
                                                                          )

    + 2) For de-queue operation, call the function CircularBuffer_Dequeue(
                                                                          CircularBufferTypeDef *targetBuffer : target FIFO buffer,
                                                                          _BUFFER_DATA_TYPE *dequeueData : de-queue data pointer,
                                                                          uint32_t dequeueSize : de-queue data size
                                                                          )

    + 3) For initialize circular buffer, call the function CircularBuffer_Init(CircularBufferTypeDef *targetBuffer : target FIFO buffer)


  * Note : How to changing a dataType of circular buffer
    ----------------------------------------------------
    + The default dataType of circular buffer is defined by typedef : _BUFFER_DATA_TYPE in circularBuffer.h which is "int16_t".
    + This can be changed to another type (example, float32_t, uint32_t etc.) by changing typedef : _BUFFER_DATA_TYPE_DEFAULT in circularBuffer.h to another type.
**/

#include "circularBuffer.h"

/**
  * @brief  CircularBuffer_Enqueue() : This function is used to "En-queue" an input data into a circular buffer.
  * @param  targetBuffer : target circular buffer (pass by ref)
  * @param  enqueueData  : enqueued data
  * @param  enqueueSize  : size of enqueued data
  * @retval None
  */
void CircularBuffer_Enqueue(CircularBufferTypeDef *targetBuffer, _BUFFER_DATA_TYPE *enqueueData, uint32_t enqueueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> Full
      * 2 -> with rear > front
      * 3 -> with rear < front
      */
    if((targetBuffer->rear == -1)&&(targetBuffer->front == -1))     bufferState = 0;    //Empty state
    else if(targetBuffer->rear == targetBuffer->front)              bufferState = 1;    //Full state
    else if(targetBuffer->rear  > targetBuffer->front)              bufferState = 2;    //rear > front
    else if(targetBuffer->rear  < targetBuffer->front)              bufferState = 3;    //rear < front

    switch (bufferState){
        case 0:     //Empty state
            /* Change buffer to non-empty state (buffer is reset) */
            targetBuffer->front = 0;
            targetBuffer->rear  = 0;
            goto CASE_R_GREATER_THAN_F;
            break;

        case 1:     //Full state
            /* Do nothing */
            break;

        case 2:     //rear > front
            CASE_R_GREATER_THAN_F:
            if((targetBuffer->rear + enqueueSize) <= targetBuffer->size)
            {
                /**  memcpy() between buffer and enqueue data
                  *  start from rear:r to r + enqueueSize
                  *  when r + enqueueSize does not exceed end-of-buffer   (Not wrapping)
                  *  then, copy only 1 section
                  */
                memcpy(targetBuffer->buffer + (targetBuffer->rear), enqueueData, sizeof(_BUFFER_DATA_TYPE)*enqueueSize);
                targetBuffer->rear = (targetBuffer->rear + enqueueSize)%targetBuffer->size;
            }
            else
            {
                 /**  memcpy() between buffer and enqueue data
                  *  start from rear:r to r + enqueueSize
                  *  when r + enqueueSize exceed end-of-buffer            (Wrapping)
                  *  then, copy with 2 sections
                  */

                /* 1st section copy (r to end-of-buffer part) */
                memcpy(targetBuffer->buffer + (targetBuffer->rear), enqueueData, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->size - targetBuffer->rear));

                /* 2nd section copy (wrapping part) */
                // No overwritten occur
                if(enqueueSize + targetBuffer->rear - targetBuffer->size <= targetBuffer->front)
                {
                    memcpy(targetBuffer->buffer, enqueueData + (targetBuffer->size - targetBuffer->rear), sizeof(_BUFFER_DATA_TYPE)*(enqueueSize + targetBuffer->rear - targetBuffer->size));
                    targetBuffer->rear = (targetBuffer->rear + enqueueSize)%targetBuffer->size;
                }
                // Overwritten occur during wrapping!
                else
                {
                    memcpy(targetBuffer->buffer, enqueueData + (targetBuffer->size - targetBuffer->rear), sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->front));
                    targetBuffer->rear = targetBuffer->front;
                }
            }
            break;

        case 3:     //rear < front

            if(targetBuffer->rear + enqueueSize <= targetBuffer->front)
            {
                /**  memcpy() between buffer and enqueue data
                  *  start from rear:r to r + enqueueSize
                  *  when r + enqueueSize is not exceed front : f   (Overwritten do not occur)
                  */
                memcpy(targetBuffer->buffer + (targetBuffer->rear), enqueueData, sizeof(_BUFFER_DATA_TYPE)*enqueueSize);
                targetBuffer->rear = (targetBuffer->rear + enqueueSize)%targetBuffer->size;
            }
            else
            {
                /**  memcpy() between buffer and enqueue data
                  *  start from rear:r to r + enqueueSize
                  *  when r + enqueueSize is exceed front : f       (Overwritten occur!)
                  */
                memcpy(targetBuffer->buffer + (targetBuffer->rear), enqueueData, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->front - targetBuffer->rear));
                targetBuffer->rear = targetBuffer->front;
            }
            break;

        default:
            /* Do nothing */
            break;
    }

}
/**
  * @brief  CircularBuffer_Dequeue() : This function is used to "De-queue" an input data into a circular buffer.
  * @param  targetBuffer : target circular buffer (pass by ref)
  * @param  dequeueData  : dequeued data (pass by ref)
  * @param  dequeueSize  : size of dequeued data
  * @retval None
  */
void CircularBuffer_Dequeue(CircularBufferTypeDef *targetBuffer, _BUFFER_DATA_TYPE *dequeueData, uint32_t dequeueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> with rear > front
      * 2 -> with rear <= front
      */
    if((targetBuffer->rear == -1)&&(targetBuffer->front == -1))     bufferState = 0;    //Empty state
    else if(targetBuffer->rear  > targetBuffer->front)              bufferState = 1;    //rear > front
    else if(targetBuffer->rear  <= targetBuffer->front)             bufferState = 2;    //rear <= front

    switch (bufferState){
        case 0:     //Empty state
            /* Buffer is empty, There's no data in buffer. So, do nothing. */
            break;

        case 1:     //rear > front
            if(dequeueSize <= (targetBuffer->rear - targetBuffer->front))
            {
                /**  memcpy() between buffer and dequeue data
                  *  start from front:f to f + dequeueSize
                  *  when f + dequeueSize does not exceed r
                  */
                memcpy(dequeueData, targetBuffer->buffer + targetBuffer->front, sizeof(_BUFFER_DATA_TYPE)*dequeueSize);
                memset(targetBuffer->buffer + targetBuffer->front, 0, sizeof(_BUFFER_DATA_TYPE)*dequeueSize);
                targetBuffer->front = targetBuffer->front + dequeueSize;
            }
            else
            {
                /**  memcpy() between buffer and dequeue data
                  *  start from front:f to f + dequeueSize
                  *  when f + dequeueSize exceed r  (overwritten occur!)
                  */
                memcpy(dequeueData, targetBuffer->buffer + targetBuffer->front, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->rear - targetBuffer->front));
                memset(targetBuffer->buffer + targetBuffer->front, 0, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->rear - targetBuffer->front));
                targetBuffer->front = targetBuffer->rear;
            }

            if(targetBuffer->front == targetBuffer->rear)
            {
                /** check if buffer is empty after dequeue
                  * then, set r,f to -1
                  */
                targetBuffer->front = -1;
                targetBuffer->rear  = -1;
            }
            break;

        case 2:     //rear <= front
            if(targetBuffer->front + dequeueSize <= targetBuffer->size)
            {
                /**  memcpy() between buffer and dequeue data
                  *  start from front:f to f + dequeueSize
                  *  when f + dequeueSize is not exceed end-of-buffer   (Not Wrapping)
                  *  then, dequeue only 1 section
                  */
                memcpy(dequeueData, targetBuffer->buffer + targetBuffer->front, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize));
                memset(targetBuffer->buffer + targetBuffer->front, 0, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize));
                targetBuffer->front = (targetBuffer->front + dequeueSize)%targetBuffer->size;
            }
            else
            {
                /**  memcpy() between buffer and dequeue data
                  *  start from front:f to f + dequeueSize
                  *  when f + dequeueSize is exceed end-of-buffer   (Wrapping)
                  *  then, dequeue with 2 sections
                  */
                /* 1st section copy */
                memcpy(dequeueData, targetBuffer->buffer + targetBuffer->front, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->size - targetBuffer->front));
                memset(targetBuffer->buffer + targetBuffer->front, 0, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->size - targetBuffer->front));

                /* 2nd section copy (wrapping part) */
                // No overwritten occur
                if(dequeueSize + targetBuffer->front - targetBuffer->size <= targetBuffer->rear)
                {
                    memcpy(dequeueData + (targetBuffer->size - targetBuffer->front), targetBuffer->buffer, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize + targetBuffer->front - targetBuffer->size));
                    memset(targetBuffer->buffer, 0, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize + targetBuffer->front - targetBuffer->size));
                    targetBuffer->front = (targetBuffer->front + dequeueSize)%targetBuffer->size;
                }
                // Overwritten occur during wrapping!
                else
                {
                    memcpy(dequeueData + (targetBuffer->size - targetBuffer->front), targetBuffer->buffer, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->rear));
                    memset(targetBuffer->buffer, 0, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->rear));
                    targetBuffer->front = targetBuffer->rear;
                }
            }
            if(targetBuffer->front == targetBuffer->rear)
            {
                /** check if buffer is empty after dequeue
                  * then, set r,f to -1
                  */
                targetBuffer->front = -1;
                targetBuffer->rear  = -1;
            }
            break;

        default:
            break;
    }
}
/**
  * @brief  CircularBuffer_Init() : This function is used to "initialize" a circular buffer struct.
  * @param  targetBuffer : target circular buffer (pass by ref)
  * @retval None
  */
void CircularBuffer_Init(CircularBufferTypeDef *targetBuffer)
{
    targetBuffer->front = -1;
    targetBuffer->rear  = -1;
    targetBuffer->size  = BUFFER_SIZE;
    memset(targetBuffer->buffer, 0, sizeof(targetBuffer->buffer));
}
