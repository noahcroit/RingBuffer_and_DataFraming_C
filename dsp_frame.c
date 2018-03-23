/**
  * dsp_frame.c : 1-D signal frame extraction in C.
  *
  * Written in 2017 by Nattapong W. <https://github.com/jazzpiano1004>.
  *
  * To the extent possible under law, the author(s) have dedicated all
  * copyright and related and neighboring rights to this software to
  * the public domain worldwide. This software is distributed without
  * any warranty.
  *
  *
  * How to use this file:
    --------------------
    + This file uses circular buffer in "circularBuffer.h" to extract data into a frame.
    + First, you need to declare a frame structure with "dspFrame_TypeDef" type in dsp_frame.h
    + There're 2 main functions for frame extraction,
      1) To load a data from FIFO circular buffer into a frame, call the function DSP_frameExtraction_IsNextFrameReady()
      2) To initialize a frame-based processing,                call the function DSP_frameExtraction_Init()

    + For a frame-based processing. After frameSize and overlapSize initialization is done by using DSP_frameExtraction_Init().
      The next data frame of FIFO circular buffer can be extracted by using function DSP_frameExtraction_IsNextFrameReady().
      If circular buffer is ready to frame the next data frame. The next frame will be loaded automatically into the input frame after calling CircularBuffer_IsNextFrameReady().
      If not. Then, the next frame will not be loaded.

**/

#include "dsp_frame.h"

/**
  * @brief  DSP_frameExtraction_Init() : This function is used to "initialize" a 1-D signal frame structure for frame extraction.
  * @param  targetFrame     : target frame structure
  * @param  pFrame          : pointer of storage frame array
  * @param  SetElementSize  : size of each element (bytes)
  * @param  SetFrameSize    : size of frame (elements)
  * @param  SetOverlap      : overlap size (element)
  *
  *         Warning!
  *         - Overlap size must not exceed "SetFrameSize"
  *         - Result of modulo between frameSize and overlapSize needs to be 0 (SetFrameSize % SetOverlap = 0)
  *           to make DSP_frameExtraction_IsNextFrameReady() function works properly.
  * @retval None
  */
void DSP_frameExtraction_Init(dspFrame_TypeDef *targetFrame, void *pFrame, int8_t SetElementSize, int32_t SetFrameSize, int32_t SetOverlap)
{
    uint32_t InputByteSize;

    targetFrame->frame = pFrame;
    targetFrame->frameSize   = SetFrameSize;
    targetFrame->elementSize = SetElementSize;
    targetFrame->overlap   = SetOverlap;
    targetFrame->firstFrameCompleteFlag = FIRST_FRAME_IS_NOT_COMPLETED;

    InputByteSize = (targetFrame->elementSize)*(targetFrame->frameSize);
    memset(targetFrame->frame, 0, InputByteSize);
}

/**
  * @brief  DSP_frameExtraction_IsNextFrameReady() : This function is used to check if ring buffer is ready to load a next data frame or not.
  *                                                  If buffer is ready. Then, the buffer will load a next data frame to array of frame.
  *
  *                                                  Warning! : Function "De-enqueue" is called with De-Q size = frameSize - overlapSize,
  *                                                            as for a framing algorithm when buffer is ready.
  * @param  targetBuf    : circular buffer structure
  * @param  targetFrame  : frame structure
  * @retval FRAME_IS_READY      -> ready
  *         FRAME_IS_NOT_READY  -> not ready
  *         FRAME_ERROR         -> error
  */

dspFrame_result DSP_frameExtraction_IsNextFrameReady(circularBuffer_TypeDef *targetBuf, dspFrame_TypeDef *targetFrame)
{
    void        *previousOverlap;
    uint32_t    dequeueSize;

    //if data type of element in buffer and frame have a difference byte size.
    if(targetFrame->elementSize != targetBuf->elementSize)
    {
        return FRAME_ERROR;	//error
    }
    //if data type of element in buffer and frame have a same byte size. -> OK!
    else
    {
        if(targetFrame->firstFrameCompleteFlag == FIRST_FRAME_IS_NOT_COMPLETED)
        {
            if((targetBuf->r - targetBuf->f >= targetFrame->frameSize) && (targetBuf->f != targetBuf->r))
            {
                dequeueSize = targetFrame->frameSize - targetFrame->overlap;
                targetFrame->firstFrameCompleteFlag = FIRST_FRAME_IS_COMPLETED;

                //Allocate memory for previous overlap section buffer
                targetFrame->p_previousOverlap = (void *)(calloc(targetFrame->overlap, targetFrame->elementSize));
                previousOverlap = targetFrame->p_previousOverlap;

                // copy the first frame
                CircularBuffer_Dequeue(targetBuf, targetFrame->frame, targetFrame->frameSize);

                // update overlap section
                memcpy(previousOverlap, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*(dequeueSize)), targetFrame->elementSize*(targetFrame->overlap));

                return FRAME_IS_READY;
            }
            else    return FRAME_IS_NOT_READY;
        }
        else if(targetFrame->firstFrameCompleteFlag == FIRST_FRAME_IS_COMPLETED)
        {
            dequeueSize = targetFrame->frameSize - targetFrame->overlap;

            if(targetBuf->r > targetBuf->f)
            {
                if((targetBuf->r - targetBuf->f) >= (targetFrame->frameSize - targetFrame->overlap))
                {
                    previousOverlap = targetFrame->p_previousOverlap;
                    memcpy(targetFrame->frame, previousOverlap, targetFrame->elementSize*(targetFrame->overlap));
                    CircularBuffer_Dequeue(targetBuf, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*targetFrame->overlap), dequeueSize);

                    // update overlap section
                    memcpy(previousOverlap, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*(dequeueSize)), targetFrame->elementSize*(targetFrame->overlap));

                    return FRAME_IS_READY;
                }
                else    return FRAME_IS_NOT_READY;
            }
            else if(targetBuf->r < targetBuf->f)
            {
                if((targetBuf->bufferSize + targetBuf->r - targetBuf->f) >= (targetFrame->frameSize - targetFrame->overlap))
                {
                    previousOverlap = targetFrame->p_previousOverlap;
                    memcpy(targetFrame->frame, previousOverlap, targetFrame->elementSize*(targetFrame->overlap));
                    CircularBuffer_Dequeue(targetBuf, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*targetFrame->overlap), dequeueSize);

                    // update overlap section
                    memcpy(previousOverlap, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*(dequeueSize)), targetFrame->elementSize*(targetFrame->overlap));

                    return FRAME_IS_READY;
                }
                else    return FRAME_IS_NOT_READY;
            }
            else if(targetBuf->r == targetBuf->f)
            {
                if(CircularBuffer_IsFull(targetBuf))
                {
                    previousOverlap = targetFrame->p_previousOverlap;
                    memcpy(targetFrame->frame, previousOverlap, targetFrame->elementSize*(targetFrame->overlap));
                    CircularBuffer_Dequeue(targetBuf, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*targetFrame->overlap), targetFrame->frameSize - targetFrame->overlap);

                    // update overlap section
                    memcpy(previousOverlap, (void *)((uint8_t *)(targetFrame->frame) + targetFrame->elementSize*(targetFrame->frameSize - targetFrame->overlap)), targetFrame->elementSize*(targetFrame->overlap));

                    return FRAME_IS_READY;
                }
                else if(CircularBuffer_IsEmpty(targetBuf))     return FRAME_IS_NOT_READY;
            }
        }
    }

    //if none of all case above is occurred. Then, there're some unknown error
    return FRAME_ERROR;
}

