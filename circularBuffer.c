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
  * @param  enqueueData  : enqueued data pointer
  * @param  enqueueSize  : size of enqueued data (#of element)
  * @retval None
  */
void CircularBuffer_Enqueue(CircularBufferTypeDef *targetBuffer, const void *enqueueData, uint32_t enqueueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> Full
      * 2 -> with rear > front
      * 3 -> with rear < front
      */
    if(CircularBuffer_IsEmpty(targetBuffer))         bufferState = BUF_STATE_EMPTY;         //Empty state
    else if(CircularBuffer_IsFull(targetBuffer))     bufferState = BUF_STATE_FULL;          //Full state
    else if(targetBuffer->r  > targetBuffer->f)      bufferState = BUF_STATE_R_MORE_THAN_F; //rear > front
    else if(targetBuffer->r  < targetBuffer->f)      bufferState = BUF_STATE_R_LESS_THAN_F; //rear < front

    switch (bufferState){
    case BUF_STATE_EMPTY:     //Empty state
        /* Change buffer to non-empty state (buffer is reset) */
        targetBuffer->f = 0;
        targetBuffer->r = 0;
        goto label1;
        break;

    case BUF_STATE_FULL:     //Full state
        /* Do nothing */
        break;

    case BUF_STATE_R_MORE_THAN_F:     //rear > front
        label1:
        if((targetBuffer->r + enqueueSize) <= targetBuffer->bufferSize)
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize does not exceed end-of-buffer   (Not wrapping)
              *  then, copy only 1 section
              */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, (targetBuffer->sizeOfElement)*enqueueSize);
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
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, targetBuffer->sizeOfElement*(targetBuffer->bufferSize - targetBuffer->r));
            /* 2nd section copy (wrapping part) */
            // No overwritten occur
            if(enqueueSize + targetBuffer->r - targetBuffer->bufferSize <= targetBuffer->f)
            {
                memcpy(targetBuffer->buffer, enqueueData + (targetBuffer->bufferSize - targetBuffer->r), targetBuffer->sizeOfElement*(enqueueSize + targetBuffer->r - targetBuffer->bufferSize));
                targetBuffer->r = (targetBuffer->r + enqueueSize)%targetBuffer->bufferSize;
            }
            // Overwritten occur during wrapping!
            else
            {
                memcpy(targetBuffer->buffer, enqueueData + (targetBuffer->bufferSize - targetBuffer->r), targetBuffer->sizeOfElement*(targetBuffer->f));
                targetBuffer->r = targetBuffer->f;
            }
        }
        break;

    case BUF_STATE_R_LESS_THAN_F:     //rear < front
        if(targetBuffer->r + enqueueSize <= targetBuffer->f)
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize is not exceed front : f   (Overwritten do not occur)
              */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, targetBuffer->sizeOfElement*enqueueSize);
            targetBuffer->r = (targetBuffer->r + enqueueSize)%targetBuffer->bufferSize;
        }
        else
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize is exceed front : f       (Overwritten occur!)
              */
            memcpy(targetBuffer->buffer + (targetBuffer->r), enqueueData, targetBuffer->sizeOfElement*(targetBuffer->f - targetBuffer->r));
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
  * @param  dequeueData  : dequeued data pointer
  * @param  dequeueSize  : size of dequeued data (#of element)
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
    if(CircularBuffer_IsEmpty(targetBuffer))            bufferState = BUF_STATE_EMPTY;          //Empty state
    else if(targetBuffer->r  >  targetBuffer->f)        bufferState = BUF_STATE_R_MORE_THAN_F;  //rear > front
    else if(targetBuffer->r  <= targetBuffer->f)        bufferState = BUF_STATE_R_LESS_THAN_F;  //rear <= front

    switch (bufferState){
    case BUF_STATE_EMPTY:     //Empty state
        /* Buffer is empty, There's no data in buffer. So, do nothing. */
        break;

    case BUF_STATE_R_MORE_THAN_F:     //rear > front
        if(dequeueSize <= (targetBuffer->r - targetBuffer->f))
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize does not exceed r
              */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, (targetBuffer->sizeOfElement)*dequeueSize);
            memset(targetBuffer->buffer + targetBuffer->f, 0, (targetBuffer->sizeOfElement)*dequeueSize);
            targetBuffer->f = targetBuffer->f + dequeueSize;
        }
        else
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize exceed r  (overwritten occur!)
              */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, targetBuffer->sizeOfElement*(targetBuffer->r - targetBuffer->f));
            memset(targetBuffer->buffer + targetBuffer->f, 0, targetBuffer->sizeOfElement*(targetBuffer->r - targetBuffer->f));
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

    case BUF_STATE_R_LESS_THAN_F:     //rear <= front
        if(targetBuffer->f + dequeueSize <= targetBuffer->bufferSize)
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize is not exceed end-of-buffer   (Not Wrapping)
              *  then, dequeue only 1 section
              */
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, targetBuffer->sizeOfElement*(dequeueSize));
            memset(targetBuffer->buffer + targetBuffer->f, 0, targetBuffer->sizeOfElement*(dequeueSize));
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
            memcpy(dequeueData, targetBuffer->buffer + targetBuffer->f, targetBuffer->sizeOfElement*(targetBuffer->bufferSize - targetBuffer->f));
            memset(targetBuffer->buffer + targetBuffer->f, 0, targetBuffer->sizeOfElement*(targetBuffer->bufferSize - targetBuffer->f));

            /* 2nd section copy (wrapping part) */
            // No overwritten occur
            if(dequeueSize + targetBuffer->f - targetBuffer->bufferSize <= targetBuffer->r)
            {
                memcpy(dequeueData + (targetBuffer->bufferSize - targetBuffer->f), targetBuffer->buffer, targetBuffer->sizeOfElement*(dequeueSize + targetBuffer->f - targetBuffer->bufferSize));
                memset(targetBuffer->buffer, 0, targetBuffer->sizeOfElement*(dequeueSize + targetBuffer->f - targetBuffer->bufferSize));
                targetBuffer->f = (targetBuffer->f + dequeueSize)%targetBuffer->bufferSize;
            }
            // Overwritten occur during wrapping!
            else
            {
                memcpy(dequeueData + (targetBuffer->bufferSize - targetBuffer->f), targetBuffer->buffer, targetBuffer->sizeOfElement*(targetBuffer->r));
                memset(targetBuffer->buffer, 0, targetBuffer->sizeOfElement*(targetBuffer->r));
                targetBuffer->f = targetBuffer->r;
            }
        }
        if(targetBuffer->f == targetBuffer->r)
        {
            /** check if buffer is empty after dequeue
              * then, set r,f to -1
              */
            targetBuffer->f = -1;
            targetBuffer->r = -1;
        }
        break;

    default:
        break;
    }
}

/**
  * @brief  CircularBuffer_Init() : This function is used to "initialize" a circular buffer struct.
  * @param  targetBuffer     : target circular buffer
  * @param  SetSizeOfElement : size of each element (bytes)
  * @param  SetFrameSize  : size of frame-based processing
  * @param  SetOverlap    : overlap size
  *                         Note! - overlap size must not exceed "SetFrameSize"
  *                               - result of modulo between frameSize and overlapSize needs to be 0 (SetFrameSize % SetOverlap = 0) to make frame-based buffer scheduling works properly.
  * @retval None
  */
void CircularBuffer_Init(CircularBufferTypeDef *targetBuffer, int8_t SetSizeOfElement, int8_t SetFrameSize, int16_t SetOverlap)
{
    targetBuffer->f = -1;
    targetBuffer->r = -1;
    targetBuffer->bufferSize  = CIRCULAR_BUFFER_SIZE;
    targetBuffer->sizeOfElement = SetSizeOfElement;

    if(SetFrameSize > targetBuffer->bufferSize)     targetBuffer->frameSize = targetBuffer->bufferSize;
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
  * @brief  CircularBuffer_IsNextFrameReady() : This function is used to check if ring buffer is ready for the next frame-based processing or not.
  *                                             If buffer is ready to be framed. Then, buffer will pop a next data frame with overlap from the previous frame.
  *                                             Warning : Function "De-enqueue" is called with De-Q size = frameSize - overlapSize, as for the framing algorithm when buffer is ready.
  * @param  buffer    : circular buffer
  * @param  dataFrame : sink data frame pointer
  * @retval FRAME_IS_READY      -> ready
  *         FRAME_IS_NOT_READY  -> not ready
  *         FRAME_ERROR         -> error
  */
int8_t CircularBuffer_IsNextFrameReady(CircularBufferTypeDef *buffer, _BUFFER_DATA_TYPE *dataFrame)
{
    static uint8_t firstFrameCompleteFlag = FIRST_FRAME_IS_NOT_COMPLETED;
    static _BUFFER_DATA_TYPE *previousOverlap;
    static uint16_t dequeueSize;

    if(firstFrameCompleteFlag == FIRST_FRAME_IS_NOT_COMPLETED)
    {
        if((buffer->r - buffer->f >= buffer->frameSize) && (buffer->f != buffer->r))
        {
            previousOverlap = (_BUFFER_DATA_TYPE *)(calloc(buffer->overlap, buffer->sizeOfElement));
            CircularBuffer_Dequeue(buffer, dataFrame, buffer->frameSize); // copy the first frame
            memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->sizeOfElement*(buffer->overlap)); // update overlap section

            dequeueSize = buffer->frameSize - buffer->overlap;
            firstFrameCompleteFlag = FIRST_FRAME_IS_COMPLETED;

            return FRAME_IS_READY;
        }
        else    return FRAME_IS_NOT_READY;
    }
    else if(firstFrameCompleteFlag == FIRST_FRAME_IS_COMPLETED)
    {
        if(buffer->r > buffer->f)
        {
            if(((buffer->r - buffer->f) >= (buffer->frameSize - buffer->overlap)))
            {
                CircularBuffer_Dequeue(buffer, dataFrame + buffer->overlap, dequeueSize);
                memcpy(dataFrame, previousOverlap, buffer->sizeOfElement*(buffer->overlap));
                memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->sizeOfElement*(buffer->overlap)); // update overlap section

                return FRAME_IS_READY;
            }
            else    return FRAME_IS_NOT_READY;
        }
        else if(buffer->r < buffer->f)
        {
            if((buffer->bufferSize + buffer->r - buffer->f) >= (buffer->frameSize - buffer->overlap))
            {
                CircularBuffer_Dequeue(buffer, dataFrame + buffer->overlap, buffer->frameSize - buffer->overlap);
                memcpy(dataFrame, previousOverlap, buffer->sizeOfElement*(buffer->overlap));
                memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->sizeOfElement*(buffer->overlap)); // update overlap section

                return FRAME_IS_READY;
            }
            else    return FRAME_IS_NOT_READY;
        }
        else if(buffer->r == buffer->f)
        {
            if(CircularBuffer_IsFull(buffer))
            {
                CircularBuffer_Dequeue(buffer, dataFrame + buffer->overlap, buffer->frameSize - buffer->overlap);
                memcpy(dataFrame, previousOverlap, buffer->sizeOfElement*(buffer->overlap));
                memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->sizeOfElement*(buffer->overlap)); // update overlap section

                return FRAME_IS_READY;
            }
            else if(CircularBuffer_IsEmpty(buffer))     return FRAME_IS_NOT_READY;
        }
    }
    return FRAME_ERROR;	//error
}
