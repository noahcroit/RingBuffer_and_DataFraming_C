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


  * Note : How to changing a bufferSize and dataType of circular buffer
    ----------------------------------------------------
    + BufferSize is defined by CIRCULAR_BUFFER_SIZE in circularBuffer.h. Default size is 8 elements.
    + The default dataType of circular buffer is defined by typedef : _BUFFER_DATA_TYPE in circularBuffer.h which is "int16_t".
    + This can be changed to another type (example, float32_t, uint32_t etc.) by changing typedef : _BUFFER_DATA_TYPE_DEFAULT in circularBuffer.h to another type.
**/

#include "circularBuffer.h"

/**
  * @brief  CircularBuffer_Enqueue() : This function is used to "En-queue" an input data into a circular buffer.
  * @param  targetBuffer : target circular buffer
  * @param  enqueueData  : enqueued data (pass by ref)
  * @param  enqueueSize  : size of enqueued data
  * @retval None
  */
void CircularBuffer_Enqueue(CircularBufferTypeDef *targetBuffer, void *enqueueData, uint32_t enqueueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> Full
      * 2 -> with rear > front
      * 3 -> with rear < front
      */
    if(CircularBuffer_IsEmpty(targetBuffer))         bufferState = 0;    //Empty state
    else if(CircularBuffer_IsFull(targetBuffer))     bufferState = 1;    //Full state
    else if(targetBuffer->r  > targetBuffer->f)      bufferState = 2;    //rear > front
    else if(targetBuffer->r  < targetBuffer->f)      bufferState = 3;    //rear < front

    switch (bufferState){
    case 0:     //Empty state
        /* Change buffer to non-empty state (buffer is reset) */
        targetBuffer->f = 0;
        targetBuffer->r = 0;
        goto CASE_R_GREATER_THAN_F;
        break;

    case 1:     //Full state
        /* Do nothing */
        break;

    case 2:     //rear > front
        CASE_R_GREATER_THAN_F:
        if((targetBuffer->r + enqueueSize) <= targetBuffer->bufferSize)
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize does not exceed end-of-buffer   (Not wrapping)
              *  then, copy only 1 section
              */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, sizeof(_BUFFER_DATA_TYPE)*enqueueSize);
            targetBuffer->r = (targetBuffer->r + enqueueSize)%targetBuffer->bufferSize;
        }
        else
        {
             /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize exceed end-of-buffer            (Wrapping)
              *  then, copy with 2 sections
              */

            /* 1st section copy (r to end-of-buffer part) */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->bufferSize - targetBuffer->r));

            /* 2nd section copy (wrapping part) */
            // No overwritten occur
            if(enqueueSize + targetBuffer->r - targetBuffer->bufferSize <= targetBuffer->f)
            {
                memcpy(targetBuffer->buffer, enqueueData + (targetBuffer->bufferSize - targetBuffer->r), sizeof(_BUFFER_DATA_TYPE)*(enqueueSize + targetBuffer->r - targetBuffer->bufferSize));
                targetBuffer->r = (targetBuffer->r + enqueueSize)%targetBuffer->bufferSize;
            }
            // Overwritten occur during wrapping!
            else
            {
                memcpy(targetBuffer->buffer, enqueueData + (targetBuffer->bufferSize - targetBuffer->r), sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->f));
                targetBuffer->r = targetBuffer->f;
            }
        }
        break;

    case 3:     //rear < front

        if(targetBuffer->r + enqueueSize <= targetBuffer->f)
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize is not exceed front : f   (Overwritten do not occur)
              */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, sizeof(_BUFFER_DATA_TYPE)*enqueueSize);
            targetBuffer->r = (targetBuffer->r + enqueueSize)%targetBuffer->bufferSize;
        }
        else
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize is exceed front : f       (Overwritten occur!)
              */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->f - targetBuffer->r));
            targetBuffer->r = targetBuffer->f;
        }
        break;

    default:
        /* Do nothing */
        break;
    }

}
/**
  * @brief  CircularBuffer_Dequeue() : This function is used to "De-queue" an input data into a circular buffer.
  * @param  targetBuffer : target circular buffer
  * @param  dequeueData  : dequeued data (pass by ref)
  * @param  dequeueSize  : size of dequeued data
  * @retval None
  */
void CircularBuffer_Dequeue(CircularBufferTypeDef *targetBuffer, void *dequeueData, uint32_t dequeueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> with rear > front
      * 2 -> with rear <= front
      */
    if(CircularBuffer_IsEmpty(targetBuffer))            bufferState = 0;    //Empty state
    else if(targetBuffer->r  >  targetBuffer->f)        bufferState = 1;    //rear > front
    else if(targetBuffer->r  <= targetBuffer->f)        bufferState = 2;    //rear <= front

    switch (bufferState){
    case 0:     //Empty state
        /* Buffer is empty, There's no data in buffer. So, do nothing. */
        break;

    case 1:     //rear > front
        if(dequeueSize <= (targetBuffer->r - targetBuffer->f))
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize does not exceed r
              */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, sizeof(_BUFFER_DATA_TYPE)*dequeueSize);
            memset(targetBuffer->buffer + targetBuffer->f, 0, sizeof(_BUFFER_DATA_TYPE)*dequeueSize);
            targetBuffer->f = targetBuffer->f + dequeueSize;
        }
        else
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize exceed r  (overwritten occur!)
              */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->r - targetBuffer->f));
            memset(targetBuffer->buffer + targetBuffer->f, 0, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->r - targetBuffer->f));
            targetBuffer->r = targetBuffer->r;
        }

        if(targetBuffer->f == targetBuffer->r)
        {
            /** check if buffer is empty after dequeue
              * then, set r,f to -1
              */
            targetBuffer->f = -1;
            targetBuffer->r  = -1;
        }
        break;

    case 2:     //rear <= front
        if(targetBuffer->f + dequeueSize <= targetBuffer->bufferSize)
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize is not exceed end-of-buffer   (Not Wrapping)
              *  then, dequeue only 1 section
              */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize));
            memset(targetBuffer->buffer + targetBuffer->f, 0, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize));
            targetBuffer->f = (targetBuffer->f + dequeueSize)%targetBuffer->bufferSize;
        }
        else
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize is exceed end-of-buffer   (Wrapping)
              *  then, dequeue with 2 sections
              */
            /* 1st section copy */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->bufferSize - targetBuffer->f));
            memset(targetBuffer->buffer + targetBuffer->f, 0, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->bufferSize - targetBuffer->f));

            /* 2nd section copy (wrapping part) */
            // No overwritten occur
            if(dequeueSize + targetBuffer->f - targetBuffer->bufferSize <= targetBuffer->r)
            {
                memcpy(dequeueData + (targetBuffer->bufferSize - targetBuffer->f), targetBuffer->buffer, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize + targetBuffer->f - targetBuffer->bufferSize));
                memset(targetBuffer->buffer, 0, sizeof(_BUFFER_DATA_TYPE)*(dequeueSize + targetBuffer->f - targetBuffer->bufferSize));
                targetBuffer->f = (targetBuffer->f + dequeueSize)%targetBuffer->bufferSize;
            }
            // Overwritten occur during wrapping!
            else
            {
                memcpy(dequeueData + (targetBuffer->bufferSize - targetBuffer->f), targetBuffer->buffer, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->r));
                memset(targetBuffer->buffer, 0, sizeof(_BUFFER_DATA_TYPE)*(targetBuffer->r));
                targetBuffer->f = targetBuffer->r;
            }
        }
        if(targetBuffer->f == targetBuffer->r)
        {
            /** check if buffer is empty after dequeue
              * then, set r,f to -1
              */
            targetBuffer->f = -1;
            targetBuffer->r  = -1;
        }
        break;

    default:
        break;
    }
}
/**
  * @brief  CircularBuffer_Init() : This function is used to "initialize" a circular buffer struct.
  * @param  targetBuffer : target circular buffer
  * @param  SetFrameSize    : size of frame-based processing
  * @param  SetOverlap      : frame overlap (must between 0 up to "SetFrameSize")
  * @retval None
  */
void CircularBuffer_Init(CircularBufferTypeDef *targetBuffer, int8_t SetFrameSize, int16_t SetOverlap)
{
    targetBuffer->f = -1;
    targetBuffer->r  = -1;
    targetBuffer->bufferSize  = CIRCULAR_BUFFER_SIZE;

    if(SetFrameSize > CIRCULAR_BUFFER_SIZE)     targetBuffer->frameSize = CIRCULAR_BUFFER_SIZE;
    else    targetBuffer->frameSize = SetFrameSize;

    targetBuffer->overlap   = SetOverlap;
    memset(targetBuffer->buffer, 0, sizeof(targetBuffer->buffer));
}

/**
  * @brief  CircularBuffer_IsFull() : This function is used to check if a circular buffer is full or not.
  * @param  targetBuffer : target circular buffer
  * @retval 0 -> not full
  *         1 -> full
  */
uint8_t CircularBuffer_IsFull(CircularBufferTypeDef *targetBuffer)
{
    if((targetBuffer->f == targetBuffer->r) && (targetBuffer->f != -1))      return 1;
    else        return 0;
}

/**
  * @brief  CircularBuffer_IsEmpty() : This function is used to check if a circular buffer is empty or not.
  * @param  targetBuffer : target circular buffer
  * @retval 0 -> not empty
  *         1 -> empty
  */
uint8_t CircularBuffer_IsEmpty(CircularBufferTypeDef *targetBuffer)
{
    if((targetBuffer->r == -1) && (targetBuffer->f == -1))      return 1;
    else        return 0;
}

/**
  * @brief  CircularBuffer_CheckNextFrameReady() : This function is used to check if ring buffer is ready for the next frame-based processing or not.
  * @param  buffer : circular buffer
  * @retval 1  -> ready
  *         0  -> not ready
  *         -1 -> error
  */
int8_t CircularBuffer_CheckNextFrameReady(CircularBufferTypeDef *buffer)
{
    static uint8_t firstFrameReady = 0;

    if(firstFrameReady == 0)
    {
        if(((buffer->r - buffer->f)%(buffer->frameSize) == 0) && (buffer->f != buffer->r))
        {
            firstFrameReady = 1;
            return 1;
        }
        else    return 0;
    }
    else if(firstFrameReady != 0)
    {
        if(buffer->r >= buffer->f)
        {
            if((buffer->r - buffer->f)%(buffer->overlap) == 0)     return 1;
            else    return 0;
        }
        else if(buffer->r < buffer->f)
        {
            if((buffer->bufferSize + buffer->r - buffer->f)%(buffer->overlap) == 0)    return 1;
            else    return 0;
        }
    }

    return -1;	//error
}
