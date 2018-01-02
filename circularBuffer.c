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
    + There're 3 main functions for FIFO circular buffer,
      1) To En-queue a FIFO circular buffer,    call the function CircularBuffer_Enqueue()
      2) To De-queue a FIFO circular buffer,    call the function CircularBuffer_Dequeue()
      3) To initialize a FIFO circular buffer,  call the function CircularBuffer_Init()

    + For a frame-based processing by using FIFO circular buffer. After frameSize and overlapSize initialization is done by using CircularBuffer_Init().
      The next data frame of FIFO circular buffer can be extracted by using function CircularBuffer_IsNextFrameReady().
      If circular buffer is ready to frame the next data frame. The next frame will be loaded automatically into the input frame after calling CircularBuffer_IsNextFrameReady().
      If not. Then, the next frame will not be loaded.

  * Note : How to changing a bufferSize and dataType of circular buffer
    ----------------------------------------------------
    + BufferSize is defined by CIRCULAR_BUFFER_SIZE in circularBuffer.h. Default size is 8 elements.
    + The default dataType of circular buffer is defined by typedef : _RING_BUFFER_DATA_TYPE in circularBuffer.h which is "int16_t".
    + This can be changed to another type (example, float32_t, uint32_t etc.) by changing typedef : _RING_BUFFER_DATA_TYPE in circularBuffer.h to another type.
**/

#include "circularBuffer.h"



/**
  * @brief  CircularBuffer_Enqueue() : This function is used to "En-queue" an input data into a FIFO circular buffer.
  * @param  targetBuf    : target circular buffer
  * @param  enqueueData  : enqueued data pointer
  * @param  enqueueSize  : size of enqueued data (#of element)
  * @retval None
  */
void CircularBuffer_Enqueue(CircularBufferTypeDef *targetBuf, const void *enqueueData, uint32_t enqueueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> Full
      * 2 -> with rear > front
      * 3 -> with rear < front
      */
    if(CircularBuffer_IsEmpty(targetBuf))         bufferState = BUF_STATE_EMPTY;         //Empty state
    else if(CircularBuffer_IsFull(targetBuf))     bufferState = BUF_STATE_FULL;          //Full state
    else if(targetBuf->r  > targetBuf->f)         bufferState = BUF_STATE_R_MORE_THAN_F; //rear > front
    else if(targetBuf->r  < targetBuf->f)         bufferState = BUF_STATE_R_LESS_THAN_F; //rear < front

    switch (bufferState){
    case BUF_STATE_EMPTY:     //Empty state
        /* Change buffer to non-empty state (buffer is reset) */
        targetBuf->f = 0;
        targetBuf->r = 0;
        goto label1;

    case BUF_STATE_FULL:     //Full state
        /* Do nothing */
        break;

    case BUF_STATE_R_MORE_THAN_F:     //rear > front
        label1:
        if((targetBuf->r + enqueueSize) <= targetBuf->bufferSize)
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize does not exceed end-of-buffer   (Not wrapping)
              *  then, copy only 1 section
              */
            memcpy((void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->r), enqueueData, (targetBuf->elementSize)*enqueueSize);
            targetBuf->r = (targetBuf->r + enqueueSize)%targetBuf->bufferSize;
        }
        else
        {
             /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize exceed end-of-buffer            (Wrapping)
              *  then, copy with 2 sections
              */

            /* 1st section copy (r to end-of-buffer part) */
            memcpy((void *)((uint8_t *)(targetBuf->buf) + targetBuf->r), enqueueData, targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->r));
            /* 2nd section copy (wrapping part) */
            // No overwritten occur
            if(enqueueSize + targetBuf->r - targetBuf->bufferSize <= targetBuf->f)
            {
                memcpy(targetBuf->buf, (void *)((uint8_t *)(enqueueData) + targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->r)), targetBuf->elementSize*(enqueueSize + targetBuf->r - targetBuf->bufferSize));
                targetBuf->r = (targetBuf->r + enqueueSize)%targetBuf->bufferSize;
            }
            // Overwritten occur during wrapping!
            else
            {
                memcpy(targetBuf->buf, (void *)((uint8_t *)(enqueueData) + targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->r)), targetBuf->elementSize*(targetBuf->f));
                targetBuf->r = targetBuf->f;
            }
        }
        break;

    case BUF_STATE_R_LESS_THAN_F:     //rear < front
        if(targetBuf->r + enqueueSize <= targetBuf->f)
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize is not exceed front : f   (Overwritten do not occur)
              */
            memcpy((void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->r), enqueueData, targetBuf->elementSize*enqueueSize);
            targetBuf->r = (targetBuf->r + enqueueSize)%targetBuf->bufferSize;
        }
        else
        {
            /**  memcpy() between buffer and enqueue data
              *  start from rear:r to r + enqueueSize
              *  when r + enqueueSize is exceed front : f       (Overwritten occur!)
              */
            memcpy((void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->r), enqueueData, targetBuf->elementSize*(targetBuf->f - targetBuf->r));
            targetBuf->r = targetBuf->f;
        }
        break;

    default:
        /* Do nothing */
        break;
    }
}

/**
  * @brief  CircularBuffer_Dequeue() : This function is used to "De-queue" an input data into a FIFO circular buffer.
  * @param  targetBuf    : target circular buffer
  * @param  dequeueData  : dequeued data pointer
  * @param  dequeueSize  : size of dequeued data (#of element)
  * @retval None
  */
void CircularBuffer_Dequeue(CircularBufferTypeDef *targetBuf, void *dequeueData, uint32_t dequeueSize)
{
    static uint8_t bufferState;

    /** Check buffer state
      * 0 -> Empty
      * 1 -> with rear > front
      * 2 -> with rear <= front
      */
    if(CircularBuffer_IsEmpty(targetBuf))         bufferState = BUF_STATE_EMPTY;          //Empty state
    else if(targetBuf->r  >  targetBuf->f)        bufferState = BUF_STATE_R_MORE_THAN_F;  //rear > front
    else if(targetBuf->r  <= targetBuf->f)        bufferState = BUF_STATE_R_LESS_THAN_F;  //rear <= front

    switch (bufferState){
    case BUF_STATE_EMPTY:     //Empty state
        /* Buffer is empty, There's no data in buffer. So, do nothing. */
        break;

    case BUF_STATE_R_MORE_THAN_F:     //rear > front
        if(dequeueSize <= (targetBuf->r - targetBuf->f))
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize does not exceed r
              */
            memcpy(dequeueData, (void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->f), (targetBuf->elementSize)*dequeueSize);
            //memset(targetBuf->buf + targetBuf->f, 0, (targetBuf->elementSize)*dequeueSize);
            targetBuf->f = targetBuf->f + dequeueSize;
        }
        else
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize exceed r  (overwritten occur!)
              */
            memcpy(dequeueData, (void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->f), targetBuf->elementSize*(targetBuf->r - targetBuf->f));
            //memset(targetBuf->buf + targetBuf->f, 0, targetBuf->elementSize*(targetBuf->r - targetBuf->f));
            targetBuf->r = targetBuf->r;
        }

        if(targetBuf->f == targetBuf->r)
        {
            /** check if buffer is empty after dequeue
              * then, set r,f to -1
              */
            targetBuf->f = -1;
            targetBuf->r  = -1;
        }
        break;

    case BUF_STATE_R_LESS_THAN_F:     //rear <= front
        if(targetBuf->f + dequeueSize <= targetBuf->bufferSize)
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize is not exceed end-of-buffer   (Not Wrapping)
              *  then, dequeue only 1 section
              */
            memcpy(dequeueData, (void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->f), targetBuf->elementSize*(dequeueSize));
            //memset(targetBuf->buf + targetBuf->f, 0, targetBuf->elementSize*(dequeueSize));
            targetBuf->f = (targetBuf->f + dequeueSize)%targetBuf->bufferSize;
        }
        else
        {
            /**  memcpy() between buffer and dequeue data
              *  start from front:f to f + dequeueSize
              *  when f + dequeueSize is exceed end-of-buffer   (Wrapping)
              *  then, dequeue with 2 sections
              */
            /* 1st section copy */
            memcpy(dequeueData, (void *)((uint8_t *)(targetBuf->buf) + targetBuf->elementSize*targetBuf->f), targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->f));
            memset(targetBuf->buf + targetBuf->f, 0, targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->f));

            /* 2nd section copy (wrapping part) */
            // No overwritten occur
            if(dequeueSize + targetBuf->f - targetBuf->bufferSize <= targetBuf->r)
            {
                memcpy((void *)((uint8_t *)(dequeueData) + targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->f)), targetBuf->buf, targetBuf->elementSize*(dequeueSize + targetBuf->f - targetBuf->bufferSize));
                //memset(targetBuf->buf, 0, targetBuf->elementSize*(dequeueSize + targetBuf->f - targetBuf->bufferSize));
                targetBuf->f = (targetBuf->f + dequeueSize)%targetBuf->bufferSize;
            }
            // Overwritten occur during wrapping!
            else
            {
                memcpy((void *)((uint8_t *)(dequeueData) + targetBuf->elementSize*(targetBuf->bufferSize - targetBuf->f)), targetBuf->buf, targetBuf->elementSize*(targetBuf->r));
                //memset(targetBuf->buf, 0, targetBuf->elementSize*(targetBuf->r));
                targetBuf->f = targetBuf->r;
            }
        }
        if(targetBuf->f == targetBuf->r)
        {
            /** check if buffer is empty after dequeue
              * then, set r,f to -1
              */
            targetBuf->f = -1;
            targetBuf->r = -1;
        }
        break;

    default:
        break;
    }
}

/**
  * @brief  CircularBuffer_Init() : This function is used to "initialize" a FIFO circular buffer struct.
  * @param  targetBuf     : target circular buffer
  * @param  SetElementSize : size of each element (bytes)
  * @param  SetFrameSize  : size of frame-based processing
  * @param  SetOverlap    : overlap size
  *                         Note! - overlap size must not exceed "SetFrameSize"
  *                               - result of modulo between frameSize and overlapSize needs to be 0 (SetFrameSize % SetOverlap = 0) to make frame-based buffer scheduling works properly.
  * @retval None
  */
void CircularBuffer_Init(CircularBufferTypeDef *targetBuf, int8_t SetElementSize, int16_t SetFrameSize, int16_t SetOverlap)
{
    targetBuf->f = -1;
    targetBuf->r = -1;
    targetBuf->bufferSize  = CIRCULAR_BUFFER_SIZE;
    targetBuf->elementSize = SetElementSize;

    if(SetFrameSize > targetBuf->bufferSize)     targetBuf->frameSize = targetBuf->bufferSize;
    else    targetBuf->frameSize = SetFrameSize;

    targetBuf->overlap   = SetOverlap;
    memset(targetBuf->buf, 0, sizeof(targetBuf->buf));
}

/**
  * @brief  CircularBuffer_Flush() : This function is used to check if a circular buffer is full or not.
  * @param  targetBuf : target circular buffer
  * @retval none
  */
void CircularBuffer_Flush(CircularBufferTypeDef *targetBuf)
{
    targetBuf->f = -1;
    targetBuf->r = -1;
}

/**
  * @brief  CircularBuffer_IsFull() : This function is used to check if a circular buffer is full or not.
  * @param  targetBuf : target circular buffer
  * @retval 0 -> not full
  *         1 -> full
  */
uint8_t CircularBuffer_IsFull(CircularBufferTypeDef *targetBuf)
{
    if((targetBuf->f == targetBuf->r) && (targetBuf->f != -1))      return 1;
    else        return 0;
}

/**
  * @brief  CircularBuffer_IsEmpty() : This function is used to check if a circular buffer is empty or not.
  * @param  targetBuf : target circular buffer
  * @retval 0 -> not empty
  *         1 -> empty
  */
uint8_t CircularBuffer_IsEmpty(CircularBufferTypeDef *targetBuf)
{
    if((targetBuf->r == -1) && (targetBuf->f == -1))      return 1;
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
int8_t CircularBuffer_IsNextFrameReady(CircularBufferTypeDef *buffer, _RING_BUFFER_DATA_TYPE *dataFrame)
{
    static uint8_t firstFrameCompleteFlag = FIRST_FRAME_IS_NOT_COMPLETED;
    static _RING_BUFFER_DATA_TYPE *previousOverlap;
    static uint16_t dequeueSize;

    if(firstFrameCompleteFlag == FIRST_FRAME_IS_NOT_COMPLETED)
    {
        if((buffer->r - buffer->f >= buffer->frameSize) && (buffer->f != buffer->r))
        {
            previousOverlap = (_RING_BUFFER_DATA_TYPE *)(calloc(buffer->overlap, buffer->elementSize));
            CircularBuffer_Dequeue(buffer, dataFrame, buffer->frameSize); // copy the first frame
            memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->elementSize*(buffer->overlap)); // update overlap section

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
                memcpy(dataFrame, previousOverlap, buffer->elementSize*(buffer->overlap));
                memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->elementSize*(buffer->overlap)); // update overlap section

                return FRAME_IS_READY;
            }
            else    return FRAME_IS_NOT_READY;
        }
        else if(buffer->r < buffer->f)
        {
            if((buffer->bufferSize + buffer->r - buffer->f) >= (buffer->frameSize - buffer->overlap))
            {
                CircularBuffer_Dequeue(buffer, dataFrame + buffer->overlap, buffer->frameSize - buffer->overlap);
                memcpy(dataFrame, previousOverlap, buffer->elementSize*(buffer->overlap));
                memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->elementSize*(buffer->overlap)); // update overlap section

                return FRAME_IS_READY;
            }
            else    return FRAME_IS_NOT_READY;
        }
        else if(buffer->r == buffer->f)
        {
            if(CircularBuffer_IsFull(buffer))
            {
                CircularBuffer_Dequeue(buffer, dataFrame + buffer->overlap, buffer->frameSize - buffer->overlap);
                memcpy(dataFrame, previousOverlap, buffer->elementSize*(buffer->overlap));
                memcpy(previousOverlap, dataFrame + (buffer->frameSize - buffer->overlap), buffer->elementSize*(buffer->overlap)); // update overlap section

                return FRAME_IS_READY;
            }
            else if(CircularBuffer_IsEmpty(buffer))     return FRAME_IS_NOT_READY;
        }
    }
    return FRAME_ERROR;	//error
}
